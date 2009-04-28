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

#ifndef __ALLOC_H
#define __ALLOC_H

#include <string.h>
#include <malloc.h>

#include <libos/libos.h>

static inline __attribute__((malloc)) void *alloc(size_t size, size_t align)
{
	void *ret;

#ifdef CONFIG_LIBOS_MALLOC
	if (__builtin_constant_p(align) && align <= 8)
		ret = mspace_malloc(libos_mspace, size);
	else
		ret = mspace_memalign(libos_mspace, align, size);
#else
	ret = memalign(align, size);
#endif

	if (likely(ret))
		memset(ret, 0, size);

	return ret;
}

#define alloc_type(T) alloc(sizeof(T), __alignof__(T))
#define alloc_type_num(T, n) alloc(sizeof(T) * (n), __alignof__(T))

#ifdef CONFIG_LIBOS_VIRT_ALLOC
void valloc_init(unsigned long start, unsigned long end);
__attribute__((malloc)) void *valloc(unsigned long size, unsigned long align);
#endif	/* CONFIG_LIBOS_VIRT_ALLOC */

#endif	/* __ALLOC_H */
