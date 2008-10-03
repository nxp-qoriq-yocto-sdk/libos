

/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
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

#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <libos/bitops.h>
#include <libos/console.h>
#include <libos/chardev.h>
#include <libos/readline.h>
#include <libos/percpu.h> 

static chardev_t *console;
#ifdef CONFIG_LIBOS_QUEUE
DECLARE_QUEUE(early_console, 4096);
static queue_t *qconsole = &early_console;
#endif
static uint32_t console_lock;

#ifdef CONFIG_LIBOS_READLINE
readline_t *rl_console;
#endif

void console_init(chardev_t *cd)
{
#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole == &early_console) {
		assert(early_console.head == 0);
		
		cd->ops->tx(cd, early_console.buf,
		            early_console.tail, CHARDEV_BLOCKING);

		qconsole = NULL;
	}
#endif

	console = cd;
}

#ifdef CONFIG_LIBOS_QUEUE
void qconsole_init(queue_t *q)
{
	if (qconsole == &early_console) {
		assert(early_console.head == 0);
		queue_write(q, early_console.buf, early_console.tail);
		queue_notify_consumer(q);
	}

	qconsole = q;
}
#endif

static int __putchar(int c)
{
	uint8_t ch = c;
	if (console) {
		if (c == '\n')
			console->ops->tx(console, (uint8_t *)"\r", 1, CHARDEV_BLOCKING);

		console->ops->tx(console, &ch, 1, CHARDEV_BLOCKING);
	}

#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole) {
		if (c == '\n')
			queue_writechar(qconsole, '\r');

		queue_writechar(qconsole, ch);
	}
#endif

	return c;
}

static void __puts_len(const char *s, size_t len)
{
	int last = '\n';

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !cpu->crashing)
		readline_suspend(rl_console);
#endif

	while (*s && len--) {
		last = *s;
		__putchar(*s++);
	}

#ifdef CONFIG_LIBOS_QUEUE
	if (qconsole)
		queue_notify_consumer(qconsole);
#endif

#ifdef CONFIG_LIBOS_READLINE
	if (rl_console && !cpu->crashing && last == '\n')
		readline_resume(rl_console);
#endif
}

int putchar(int c)
{
	int lock = 1;
	char ch = c;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > 1)
			return -1;

		cpu->crashing++;
		lock = 0;
	}

	register_t saved = disable_critint_save();

	if (lock)
		spin_lock(&console_lock);
		
	__puts_len(&ch, 1);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing--;

	restore_critint(saved);
	return 0;
}

int puts(const char *s)
{
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > 1)
			return -1;

		cpu->crashing++;
		lock = 0;
	}

	register_t saved = disable_critint_save();

	if (lock)
		spin_lock(&console_lock);
		
	__puts_len(s, INT_MAX);
	__puts_len("\r\n", 2);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing--;

	restore_critint(saved);
	return 0;
}

size_t vprintf(const char *str, va_list args)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > 1)
			return -1;

		cpu->crashing++;
		lock = 0;
	}

	register_t saved = disable_critint_save();

	if (lock)
		spin_lock(&console_lock);
		
	size_t ret = vsnprintf(buffer, buffer_size, str, args);
	if (ret > buffer_size)
		ret = buffer_size;
	
	__puts_len(buffer, ret);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing--;

	restore_critint(saved);
	return ret;
}

size_t printf(const char *str, ...)
{
	size_t ret;
	va_list args;

	va_start(args, str);
	ret = vprintf(str, args);
	va_end(args);

	return ret;
}

void set_crashing(void)
{
	cpu->crashing = 1;
}
