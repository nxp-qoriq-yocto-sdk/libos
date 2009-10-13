/** @file printf, etc.
 */
/*
 * Copyright (C) 2007-2009 Freescale Semiconductor, Inc.
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
static void drain_consolebuf_cd(queue_t *q)
{
	if (cpu->console_ok)
		queue_to_chardev(q->consumer, q, q->size, 0, CHARDEV_BLOCKING);
}

static void drain_consolebuf_queue(queue_t *q)
{
	if (cpu->console_ok) {
		queue_to_queue(q->consumer, q, q->size, 0, 0);
		queue_notify_consumer(q->consumer);
	}
}
#endif

void console_init(chardev_t *cd)
{
#ifdef CONFIG_LIBOS_QUEUE
	assert(!consolebuf.consumer);
	consolebuf.consumer = cd;
	smp_lwsync();
	consolebuf.data_avail = drain_consolebuf_cd;
#endif

	console = cd;
}

#ifdef CONFIG_LIBOS_QUEUE
void qconsole_init(queue_t *q)
{
	assert(!consolebuf.consumer);
	consolebuf.consumer = q;
	smp_lwsync();
	consolebuf.data_avail = drain_consolebuf_queue;

	qconsole = q;
}
#endif

static int putchar_nolock(int c)
{
	uint8_t ch = c;

	if (c == '\n')
		putchar_nolock('\r');

#ifdef CONFIG_LIBOS_QUEUE
	/* If we're crashing and we have a direct console device,
	 * go ahead and use it, and don't worry about screwing up
	 * the readline output.
	 */
	if (!(unlikely(cpu->crashing) && console && cpu->console_ok))
		queue_writechar(&consolebuf, ch);
	else
#endif
	if (console && cpu->console_ok)
		console->ops->tx(console, &ch, 1, CHARDEV_BLOCKING);

	return c;
}

void console_write_nolock(const char *s, size_t len)
{
	/* This is a bit of a hack, but we can't spin on a queue
	 * consumer, and when the queue is full it's better to drop
	 * full lines than end up with small, unintelligible fragments.
	 * This can still result in shorter strings crowding out the
	 * longer ones.
	 *
	 * Eventually, we may want to consider a special crash/debug output
	 * path that can take over (and spin on) a chardev that is normally
	 * attached to a byte channel.
	 *
	 * We insist on a few extra characters beyond len to account for
	 * any \n to \r\n conversions that putchar_nolock() may make.
	 */
	if (queue_get_space(&consolebuf) < min(strnlen(s, len), len + 5))
		return;

	while (*s && len--)
		putchar_nolock(*s++);

#ifdef CONFIG_LIBOS_QUEUE
	queue_notify_consumer(&consolebuf);
#endif
}

void console_write(const char *s, size_t len)
{
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > 1)
			return;

		cpu->crashing++;
		lock = 0;
	}

	register_t saved = disable_int_save();

	if (lock)
		spin_lock(&console_lock);
		
	console_write_nolock(s, len);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing--;

	restore_int(saved);
}

void set_crashing(void)
{
	cpu->crashing = 1;
}
