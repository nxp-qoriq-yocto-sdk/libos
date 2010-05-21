/** @file
 * Freescale hypervisor error numbers.
 *
 * Copyright (C) 2009,2010 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_HCALL_ERRORS_H
#define LIBOS_HCALL_ERRORS_H

/* Epapr error codes */

#define EV_EPERM                1       /** Operation not permitted */
#define EV_ENOENT               2       /**  Entry Not Found */
#define EV_EIO                  3       /** I/O error occured */
#define EV_EAGAIN               4       /** The operation had insufficient resources to complete and should be retried */
#define EV_ENOMEM               5       /** There was insufficient memory to complete the operation */
#define EV_EFAULT               6       /** Bad guest address */
#define EV_ENODEV               7       /** No such device */
#define EV_EINVAL               8       /** An argument supplied to the hcall was out of range or invalid */
#define EV_INTERNAL             9       /** An internal error occured */
#define EV_CONFIG              10       /** A configuration error was detected */
#define EV_INVALID_STATE       11       /** The object is in an invalid state */
#define EV_UNIMPLEMENTED       12       /** Unimplemented hypercall */
#define EV_BUFFER_OVERFLOW     13       /** Caller-supplied buffer too small */

#endif
