/** @file
 * Error definitions.
 *
 * These start at -256 to avoid conflicts with libfdt errors.
 */

/*
 * Copyright (C) 2008,2009 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_ERRORS_H
#define LIBOS_ERRORS_H

#include <libos/hcall-errors.h>

#define ERR_BADTREE          (-256) /**< Semantic error in device tree */
#define ERR_NOMEM            (-257) /**< Out of memory */
#define ERR_NOTRANS          (-258) /**< No translation possible */
#define ERR_BUSY             (-259) /**< Resource busy */
#define ERR_INVALID          (-260) /**< Invalid request or argument */
#define ERR_BADIMAGE         (-261) /**< Data image is invalid or broken */
#define ERR_BADADDR          (-262) /**< Bad pointer or address */
#define ERR_RANGE            (-263) /**< Value out of range */
#define ERR_UNHANDLED        (-264) /**< Operation not handled */
#define ERR_NOTFOUND         (-265) /**< Item not found */
#define ERR_WOULDBLOCK       (-266) /**< Operation would block; try again */
#define ERR_UNKNOWN          (-267) /**< Unknown failure */

#endif
