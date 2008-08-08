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

#ifndef LIBOS_CONSOLE_H
#define LIBOS_CONSOLE_H

#include <stdarg.h>
#include <stdint.h>
#include <libos/chardev.h>

void console_init(chardev_t *cd);
void qconsole_init(queue_t *q);
int putchar(int c);
int puts(const char *s);
void puts_len(const char *s, size_t len);
size_t printf(const char *str, ...);
size_t vprintf(const char *str, va_list args);
extern int crashing;

#define NUM_LOGTYPES 256
#define LIBOS_BASE_LOGTYPE  0
#define CLIENT_BASE_LOGTYPE 64

#define LOGTYPE_MISC      0
#define LOGTYPE_MMU       1
#define LOGTYPE_IRQ       2
#define LOGTYPE_MP        3
#define LOGTYPE_MALLOC    4

#define MAX_LOGLEVEL 15
#define LOGLEVEL_ALWAYS   0
#define LOGLEVEL_ERROR    2
#define LOGLEVEL_NORMAL   4
#define LOGLEVEL_DEBUG    8
#define LOGLEVEL_VERBOSE 12

extern uint8_t loglevels[NUM_LOGTYPES];
extern void invalid_logtype(void);

/* Unfortunately, GCC will not inline a varargs function.
 *
 * The separate > and == comparisons are to shut up the
 * "comparison is always true" warning with LOGTYPE_ALWAYS.
 */
#define printlog(logtype, loglevel, fmt, args...) do { \
	/* Force a linker error if used improperly. */ \
	if (logtype >= NUM_LOGTYPES || loglevel > MAX_LOGLEVEL) \
		invalid_logtype(); \
	\
	if ((!__builtin_constant_p(loglevel) || \
	     loglevel <= CONFIG_LIBOS_MAX_BUILD_LOGLEVEL) && \
	    __builtin_expect(loglevels[logtype] == loglevel || \
	                     loglevels[logtype] > loglevel, 0)) \
		printf("[%ld] " fmt, mfspr(SPR_PIR), ##args); \
} while (0)
		
#endif
