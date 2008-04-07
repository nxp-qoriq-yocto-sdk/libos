/** @file
 * Read a line from an interactive console.
 *
 * Note: This does not gracefully handle commands that are longer than
 * the screen height.
 */
/* Copyright (C) 2008 Freescale Semiconductor, Inc.
 * Author: Scott Wood <scottwood@freescale.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <libos/readline.h>
#include <libos/errors.h>
#include <libos/libos.h>
#include <libos/bitops.h>

#define HISTORY   256
#define LINE_SIZE 256

typedef struct line {
	struct line *prev, *next;
	int pos, end;
	char buf[LINE_SIZE];
} line_t;

struct readline {
	rl_action_t action;
	const char *prompt;
	queue_t *in, *out;
	line_t *oldest_line, *newest_line, *line;
	uint32_t lock;

	enum {
		st_normal, st_escape, st_bracket, st_num, st_action
	} state;

	int num[2], num_idx, width, suspended;
	char action_buf[LINE_SIZE];
};

static uint32_t rl_lock;

static int line_at(readline_t *rl, int pos)
{
	return (pos + strlen(rl->prompt)) / rl->width;
}

static int column_at(readline_t *rl, int pos)
{
	return (pos + strlen(rl->prompt)) % rl->width;
}

static void __set_cursor(readline_t *rl, int pos)
{
	int virt, horiz, back = 0;
	char buf[16];
	
	virt = line_at(rl, pos) - line_at(rl, rl->line->pos);
	horiz = column_at(rl, pos);

	if (virt < 0) {
		virt = -virt;
		back = 1;
		
		if (pos >= rl->line->pos) {
			crashing = 1;
			printf(">= pos %d rl->line->pos %d virt %d horiz %d\n", pos, rl->line->pos, virt, horiz);
		}
		
		assert(pos < rl->line->pos);
	}

	if (virt) {
		snprintf(buf, 16, "\033[%d%c", virt, back ? 'A' : 'B');
		buf[15] = 0;
		queue_writestr(rl->out, buf);
	}

	if (horiz) {
		snprintf(buf, 16, "\r\033[%dC", horiz);
		buf[15] = 0;
		queue_writestr(rl->out, buf);
	}
}

static void set_cursor(readline_t *rl, int pos)
{
	__set_cursor(rl, pos);
	rl->line->pos = pos;
}

static void display_to_end(readline_t *rl)
{
	if (rl->line->end != rl->line->pos) {
		int pos = rl->line->pos;

		queue_write(rl->out, (uint8_t *)&rl->line->buf[rl->line->pos],
		            rl->line->end - rl->line->pos);

		/* At least some terminals won't scroll until first
		 * character of next line is output, which confuses
		 * our idea of where the cursor is.
		 */
		if (column_at(rl, rl->line->end) == 0)
			queue_writestr(rl->out, " \b");
		
		rl->line->pos = rl->line->end;
		__set_cursor(rl, pos);
		rl->line->pos = pos;
	}
}

static void hide_line(readline_t *rl)
{
	int i;
	__set_cursor(rl, rl->line->end);

	queue_writestr(rl->out, "\r\033[2K");

	for (i = 0; i < line_at(rl, rl->line->end); i++)
		queue_writestr(rl->out, "\033[A\033[2K");
}

static void unhide_line(readline_t *rl)
{
	queue_writestr(rl->out, rl->prompt);
	
	if (rl->width) {
		int pos = rl->line->pos;

		rl->line->pos = 0;
		display_to_end(rl);
		set_cursor(rl, pos);
	} else {
		queue_write(rl->out, (uint8_t *)&rl->line->buf, rl->line->end);
	}
}

static void detach_line(readline_t *rl, line_t *line)
{
	if (line->prev)
		line->prev->next = line->next;
	else {
		assert(line == rl->oldest_line);
		rl->oldest_line = line->next;
	}

	if (line->next)
		line->next->prev = line->prev;
	else {
		assert(line == rl->newest_line);
		rl->newest_line = line->prev;
	}
	
	line->next = line->prev = NULL; 
}

static line_t *get_newline(readline_t *rl)
{
	line_t *line;

	if (!rl->width || rl->line->end == 0) {
		assert(rl->line == rl->newest_line);
		return rl->line;
	}

	if (rl->newest_line->end == 0) {
		line = rl->newest_line;
		detach_line(rl, line);
		return line;
	}

	line = alloc_type(line_t);
	if (line)
		return line;

	line = rl->oldest_line;
	detach_line(rl, line);
	return line;
}

static void dup_line(readline_t *rl)
{
	line_t *line;
	assert(rl->width);
	assert(rl->line->next);
	assert(rl->line != rl->newest_line);
	
	line = get_newline(rl);
	assert(!line->next);
	
	if (line == rl->line)
		return;

	assert(!line->prev);

	memcpy(line->buf, rl->line->buf, LINE_SIZE);
	line->pos = rl->line->pos;
	line->end = rl->line->end;

	line->prev = rl->newest_line;
	rl->newest_line->next = line;
	rl->line = rl->newest_line = line;
}

static void delete_char(readline_t *rl)
{
	assert(rl->line->pos <= rl->line->end);
	if (rl->line->pos == rl->line->end)
		return;

	assert(rl->width);
	if (rl->line->next)
		dup_line(rl);

	memmove(&rl->line->buf[rl->line->pos], &rl->line->buf[rl->line->pos + 1],
	        rl->line->end - rl->line->pos - 1);

	rl->line->buf[rl->line->end - 1] = ' ';
	display_to_end(rl);
	rl->line->end--;
}

static void backspace(readline_t *rl)
{
	if (rl->line->pos == 0)
		return;
	
	assert(rl->line->pos > 0);
	
	if (rl->width) {
		if (rl->line->next)
			dup_line(rl);
	
		if (rl->line->pos != rl->line->end)
			memmove(&rl->line->buf[rl->line->pos], &rl->line->buf[rl->line->pos + 1],
			        rl->line->end - rl->line->pos - 1);

		rl->line->buf[rl->line->end - 1] = ' ';

		set_cursor(rl, rl->line->pos - 1);
		display_to_end(rl);
	} else {
		rl->line->pos--;
		queue_writestr(rl->out, "\b \b");
	}

	rl->line->end--;
}

static void readline_rx_callback(queue_t *q)
{
	register_t saved = spin_lock_critsave(&rl_lock);
	readline_t *rl = q->consumer;
	int ch;

	if (!rl) {
		spin_unlock_critsave(&rl_lock, saved);
		return;
	}

	spin_lock(&rl->lock);
	spin_unlock(&rl_lock);

	if (rl->state != st_action && rl->suspended) {
		printf("\n");
		readline_resume(rl);
	}

	while ((ch = queue_readchar(q)) >= 0) {
		switch (rl->state) {
		case st_normal:
			switch (ch) {
			case '\033':
				rl->state = st_escape;
				rl->num[0] = 0;
				rl->num[1] = 0;
				rl->num_idx = 0;
				break;
			
			case '\r': {
				line_t *newline = get_newline(rl);
				line_t *oldline = rl->line;
	
				if (rl->line->pos != rl->line->end)
					__set_cursor(rl, rl->line->end);

				queue_writestr(rl->out, "\r\n");
				queue_notify_consumer(rl->out);
				
				if (rl->line->next) {
					/* If an older command was re-issued as-is, bring it
					 * to the front of the list.
					 */
					assert(rl->line != rl->newest_line);
					detach_line(rl, rl->line);

					rl->line->prev = rl->newest_line;
					rl->newest_line->next = rl->line;
					rl->newest_line = rl->line;
				}
				
				if (newline != rl->line) {
					assert(rl->line == rl->newest_line);
					assert(!newline->next);
					assert(!newline->prev);

					newline->prev = rl->newest_line;
					rl->newest_line->next = newline;
					rl->newest_line = rl->line = newline;
				}
				
				rl->state = st_action;
				spin_unlock_critsave(&rl->lock, saved);
				
				memcpy(rl->action_buf, oldline->buf, oldline->end);
				rl->action_buf[oldline->end] = 0;
				
				if (rl->action(rl->action_buf))
					return;

				saved = spin_lock_critsave(&rl->lock);

				rl->state = st_normal;
				rl->line->pos = 0;
				rl->line->end = 0;
				
				queue_writestr(rl->out, rl->prompt);
				break;
			}
			
			case '\b':
				backspace(rl);
				break;

			case 1: /* Ctrl-A */
				if (rl->width)
					set_cursor(rl, 0);
				
				break;

			case 5: /* Ctrl-E */
				if (rl->width)
					set_cursor(rl, rl->line->end);
				
				break;
			
			case 32 ... 126: /* printable */
				assert(rl->line->end < LINE_SIZE);
				if (rl->line->end + 1 == LINE_SIZE)
					break;

				if (rl->line->next)
					dup_line(rl);

				if (rl->line->end != rl->line->pos)
					memmove(&rl->line->buf[rl->line->pos + 1],
					        &rl->line->buf[rl->line->pos],
					        rl->line->end - rl->line->pos);
				
				rl->line->end++;
				rl->line->buf[rl->line->pos++] = ch;

				queue_writechar(rl->out, ch);
				display_to_end(rl);
				break;

			case 127:
				/* ANSI DEL is treated as delete on some terminals, and backspace
				 * on others.  This should probably be configurable in the absence
				 * of any way to detect the terminal type.
				 */
				backspace(rl);
				break;
			}
			
			break;

		case st_escape:
			if (ch == '[') {
				rl->state = st_bracket;
				break;
			}

			rl->state = st_normal;
			break;

		case st_bracket:
			switch (ch) {
			case '0' ... '9':
				rl->state = st_num;
				goto num;

			case 'A': /* up */
				if (rl->line->prev) {
					hide_line(rl);
					rl->line = rl->line->prev;
					unhide_line(rl);
				}
				
				break;

			case 'B': /* down */
				if (rl->line->next) {
					hide_line(rl);
					rl->line = rl->line->next;
					unhide_line(rl);
				}
				
				break;
			
			case 'D': /* left */
				if (rl->line->pos == 0 || !rl->width)
					break;
				
				assert(rl->line->pos > 0);
				
				set_cursor(rl, rl->line->pos - 1);
				break;

			case 'C': /* right */
				if (rl->line->pos == rl->line->end)
					break;

				assert(rl->width);
				assert(rl->line->pos < rl->line->end);

				set_cursor(rl, rl->line->pos + 1);
				break;

			case 'H': /* home */
				if (rl->width)
					set_cursor(rl, 0);
				
				break;

			case 'F': /* end */
				if (rl->width)
					set_cursor(rl, rl->line->end);
				
				break;
			};
			
			rl->state = st_normal;
			break;
		
		case st_num:
			switch (ch) {
			case '0' ... '9':
num:
				rl->num[rl->num_idx] *= 10;
				rl->num[rl->num_idx] += ch - '0';
				goto no_normal;

			case ';':
				if (rl->num_idx >= sizeof(rl->num) / sizeof(int))
					break;

				rl->num_idx++;
				goto no_normal;

			case 'R': {
				if (rl->num_idx != 1)
					break;
				
				rl->width = rl->num[1];
				queue_writestr(rl->out, "\0338");
				break;
			}

			case 'n':
				if (rl->num_idx != 0)
					break;
			
				if (rl->num[0] == 0) {
					/* ANSI-capable terminal detected.
					 *
					 * Enable line-wrap, and determine line length by saving
					 * current position, moving to the end of the line, and
					 * requesting that position.  Upon receiving the response,
					 * we'll move back to the current position.
					 */
					queue_writestr(rl->out, "\033[?7h\0337\033[999C\033[6n");
				}
				
				break;

			case '~':
				if (rl->num_idx != 0)
					break;

				switch (rl->num[0]) {
				case 1: /* home */
					if (rl->width)
						set_cursor(rl, 0);
				
					break;
				
				case 3: /* delete */
					delete_char(rl);
					break;

				case 4: /* end */
					if (rl->width)
						set_cursor(rl, rl->line->end);
				
					break;
				}
			}

			rl->state = st_normal;
no_normal:
			break;

		case st_action:
			break;
		}
	}

	queue_notify_consumer(rl->out);
	spin_unlock_critsave(&rl->lock, saved);
}

readline_t *readline_init(queue_t *in, queue_t *out,
                          const char *prompt, rl_action_t action)
{
	readline_t *rl = alloc_type(readline_t);
	if (!rl)
		return NULL;

	rl->in = in;
	in->consumer = rl;
	in->data_avail = readline_rx_callback;

	rl->out = out;
	out->producer = rl;

	rl->prompt = prompt;
	rl->action = action;
	
	rl->line = rl->newest_line = rl->oldest_line = alloc_type(line_t);
	if (!rl->line)
		return NULL;

	queue_writestr(out, rl->prompt);

	/* Request terminal status, to see if we have an ANSI-capable terminal */
	queue_writestr(out, "\033[5n");
	queue_notify_consumer(rl->out);

	return rl;
}

/** Hide command line to allow for other output.
 */
void readline_suspend(readline_t *rl)
{
	register_t saved = spin_lock_critsave(&rl->lock);
	
	if (!rl->suspended && rl->state != st_action) {
		if (rl->width)
			hide_line(rl);
		
		queue_writechar(rl->out, '\r');
		queue_notify_consumer(rl->out);
		rl->suspended = 1;
	}

	spin_unlock_critsave(&rl->lock, saved);
}

/** Redisplay command line after readline_suspend().
 */
void readline_resume(readline_t *rl)
{
	register_t saved = spin_lock_critsave(&rl->lock);
	
	if (rl->state != st_action) {
		unhide_line(rl);
		queue_notify_consumer(rl->out);
		rl->suspended = 0;
	}

	spin_unlock_critsave(&rl->lock, saved);
}
