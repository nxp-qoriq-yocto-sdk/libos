/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_PRINTLOG
#define LIBOS_PRINTLOG

#include <stdint.h>
#include <stdio.h>

#include <libos/io.h>

#define NUM_LOGTYPES 256
#define LIBOS_BASE_LOGTYPE  0
#define CLIENT_BASE_LOGTYPE 64

#define LOGTYPE_MISC      0
#define LOGTYPE_MMU       1
#define LOGTYPE_IRQ       2
#define LOGTYPE_MP        3
#define LOGTYPE_MALLOC    4
#define LOGTYPE_DEV       5

#define MAX_LOGLEVEL 15

/** Mandatory output.
 * Use for unrecoverable hardware failures, crashes, etc.
 */
#define LOGLEVEL_ALWAYS   0

/** Critical error.  Use for more urgent errors and recoverable
 * hardware failures.
 */
#define LOGLEVEL_CRIT     1

/** Error.  Use when the configuration or guest has done something which
 * is prohibited.
 */
#define LOGLEVEL_ERROR    2

/** Warning.  
 * Use for things that are valid, but questionable (overlapping PMAs,
 * missing properties that have a default but we want to discourage
 * relying on the default, etc.
 */
#define LOGLEVEL_WARN     3

/** Normal log message.
 * Use for informational output, progress indicators, etc.
 */
#define LOGLEVEL_NORMAL   4

/** Normal log messages w/ some extra info.
 * Has a bit more info than normal, but less than debug.
 */
#define LOGLEVEL_EXTRA    6

/** Debug log message.
 * Use for output that is useful in debugging, but would be distracting,
 * confusing, or excessive for normal users.  Additional levels up to
 * LOGLEVEL_DEBUG+3 may be used for more obscure messages, which are not
 * quite verbose enough to be considered LOGLEVEL_VERBOSE.
 */
#define LOGLEVEL_DEBUG    8

/** Verbose debug log message.
 * Use for output that is useful in some debugging, but is sufficiently
 * voluminous or performance-impacting that it should have to be specially
 * enabled.  Additional levels up to LOGLEVEL_VERBOSE+3 may be used for
 * even more severe output.
 *
 * Note that this output may be too verbose to leave enabled for more than
 * short periods of time, without making things unacceptably slow.
 */
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
