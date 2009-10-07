/** @file
 * Freescale hypervisor error numbers.
 *
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

#ifndef LIBOS_HCALL_ERRORS_H
#define LIBOS_HCALL_ERRORS_H

/* Posix return codes */

#define EPERM            1      /**< Operation not permitted */
#define ENOENT           2      /**< Entry Not Found */
#define EIO              5      /**< I/O error occured */
#define EAGAIN          11      /**< The operation had insufficient resources to complete and should be retried */
#define ENOMEM          12      /**< There was insufficient memory to complete the operation */
#define EFAULT          14      /**< Bad guest address */
#define ENODEV          19      /**< No such device */
#define EINVAL          22      /**< An argument supplied to the hcall was out of range or invalid */

/* Extended return codes */

#define FH_ERR_INTERNAL         1024    /**< An internal error occured */
#define FH_ERR_CONFIG           1025    /**< A configuration error was detected */
#define FH_ERR_INVALID_STATE    1026    /**< The object is in an invalid state */
#define FH_ERR_UNIMPLEMENTED    1027    /**< Unimplemented hypercall */
#define FH_ERR_BUFFER_OVERFLOW  1028    /**< Caller-supplied buffer too small */
#define FH_ERR_TOO_LARGE        1029    /**< Argument is too large */

#endif
