/** @file
 * Hardware threads API
 *
 * Author: Laurentiu Tudor <Laurentiu.Tudor@freescale.com>
 *
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_MP_H
#define LIBOS_MP_H

static inline void mttmr(int reg, register_t val)
{
	asm volatile(".long 0x7c0003dc | "
			"(((%0 & 0x1f) << 16) | ((%0 & 0x3e0) << 6)) | "
			"(%1 << 21)" : : "i" (reg), "r" (val) : "memory");
}

static inline register_t mftmr(int reg)
{
	register_t ret;

	asm volatile(".long 0x7c0002dc | "
			"(((%1 & 0x1f) << 16) | ((%1 & 0x3e0) << 6)) | "
			"(%0 << 21)" : "=r" (ret) : "i" (reg) : "memory");

	return ret;
}

#endif
