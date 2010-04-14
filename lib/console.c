/** @file printf, etc.
 */
/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
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

#include <libos/console.h>
#include <libos/percpu.h> 

static chardev_t *console;
#ifdef CONFIG_LIBOS_QUEUE
DECLARE_QUEUE(consolebuf, 4096);
static queue_t *qconsole;
#endif
uint32_t console_lock;

#ifdef CONFIG_LIBOS_QUEUE
static void drain_consolebuf_cd(chardev_t *dest, int blocking)
{
	if (cpu->console_ok)
		queue_to_chardev(dest, &consolebuf, consolebuf.size,
		                 0, CHARDEV_BLOCKING);
}

static void drain_consolebuf_queue(queue_t *dest, int blocking)
{
	if (cpu->console_ok) {
		queue_to_queue(dest, &consolebuf, consolebuf.size, 0, 0);
		queue_notify_consumer(dest, blocking);
	}
}

static void drain_consolebuf_cd_callback(queue_t *q, int blocking)
{
	assert(q == &consolebuf);
	drain_consolebuf_cd(q->consumer, blocking);
}

static void drain_consolebuf_queue_callback(queue_t *q, int blocking)
{
	assert(q == &consolebuf);
	drain_consolebuf_queue(q->consumer, blocking);
}

static void drain_consolebuf(void)
{
	/* If we're crashing, or if consolebuf is getting full and we
	 * can't wait for readline, drain everything directly.
	 */
	if (console && cpu->console_ok)
		drain_consolebuf_cd(console, 1);
	else if (qconsole)
		drain_consolebuf_queue(qconsole, 1);
}
#else
static void drain_consolebuf(void)
{
}
#endif

void console_init(chardev_t *cd)
{
#ifdef CONFIG_LIBOS_QUEUE
	assert(!consolebuf.consumer);
	consolebuf.consumer = cd;
	smp_lwsync();
	consolebuf.data_avail = drain_consolebuf_cd_callback;
#endif

	console = cd;
}

#ifdef CONFIG_LIBOS_QUEUE
void qconsole_init(queue_t *q)
{
	assert(!consolebuf.consumer);
	consolebuf.consumer = q;
	smp_lwsync();
	consolebuf.data_avail = drain_consolebuf_queue_callback;

	qconsole = q;
}
#endif

static int putchar_nolock(queue_t *q, int c)
{
	uint8_t ch = c;

	if (c == '\n')
		putchar_nolock(q, '\r');

#ifdef CONFIG_LIBOS_QUEUE
	/* If we're crashing and we have a direct console device,
	 * go ahead and use it, and don't worry about screwing up
	 * the readline output.
	 */
	if (!(unlikely(cpu->crashing) && console && cpu->console_ok))
		queue_writechar(q, ch);
	else
#endif
	if (console && cpu->console_ok)
		console->ops->tx(console, &ch, 1, CHARDEV_BLOCKING);

	return c;
}

void console_write_nolock(const char *s, size_t len)
{
#ifdef CONFIG_LIBOS_QUEUE
	queue_t *q = &consolebuf;

	if (unlikely(cpu->crashing) && qconsole) {
		q = qconsole;
		queue_notify_consumer(q, 1);
	}

	while (*s && len--)
		putchar_nolock(q, *s++);

	queue_notify_consumer(q, cpu->crashing);

	/* We try to wait for readline to gracefully handle output,
	 * but don't wait forever if output is coming too quickly --
	 * it's probably debugging output that we don't want to miss.
	 */
	if (q == &consolebuf && queue_get_space(q) < q->size / 2)
		drain_consolebuf();
#else
	while (*s && len--)
		putchar_nolock(NULL, *s++);
#endif
}

#define CRASH_IN_PRINT 10000

void console_write(const char *s, size_t len)
{
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > CRASH_IN_PRINT)
			return;

		cpu->crashing += CRASH_IN_PRINT;
		lock = 0;
	}

	register_t saved = disable_int_save();

	if (lock)
		spin_lock(&console_lock);
		
	console_write_nolock(s, len);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing -= CRASH_IN_PRINT;

	restore_int(saved);
}

/* Prevent concurrent crashes from resulting in an unreadable
 * intermingled mess.
 */
static uint32_t crash_lock;

void set_crashing(int crashing)
{
	if (crashing) {
		if (cpu->crashing++ == 0) {
			raw_spin_lock(&crash_lock);
			cpu->crashing = 1;
			drain_consolebuf();
		}
	} else {
		if (--cpu->crashing == 0)
			spin_unlock(&crash_lock);
	}
}
