/** @file printf, etc.
 */
/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
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

#include <stdio.h>
#include <limits.h>

#include <libos/libos.h>
#include <libos/percpu.h>
#include <libos/console.h>

int putchar(int c)
{
	char ch = c;

	console_write(&ch, 1);

	return (unsigned char)ch;
}

#define CRASH_IN_PRINT 10000

int puts(const char *s)
{
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > CRASH_IN_PRINT)
			return EOF;

		cpu->crashing += CRASH_IN_PRINT;
		lock = 0;
	}

	register_t saved = disable_int_save();

	if (lock)
		spin_lock(&console_lock);

	console_write_nolock(s, INT_MAX);
	console_write_nolock("\n", 1);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing -= CRASH_IN_PRINT;

	restore_int(saved);
	return 0;
}

int vprintf(const char *str, va_list args)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	int lock = 1;

	if (unlikely(cpu->crashing)) {
		if (cpu->crashing > CRASH_IN_PRINT)
			return -1;

		cpu->crashing += CRASH_IN_PRINT;
		lock = 0;
	}

	register_t saved = disable_int_save();

	if (lock)
		spin_lock(&console_lock);

	int ret = vsnprintf(buffer, buffer_size, str, args);
	if (ret > buffer_size)
		ret = buffer_size;

	console_write_nolock(buffer, ret);

	if (lock)
		spin_unlock(&console_lock);
	else
		cpu->crashing -= CRASH_IN_PRINT;

	restore_int(saved);
	return ret;
}

int printf(const char *str, ...)
{
	int ret;
	va_list args;

	va_start(args, str);
	ret = vprintf(str, args);
	va_end(args);

	return ret;
}
