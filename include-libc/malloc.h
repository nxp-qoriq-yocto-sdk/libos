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

#ifndef __MALLOC_H
#define __MALLOC_H

#ifdef CONFIG_LIBOS_MALLOC
#include <libos/malloc.h>

extern mspace libos_mspace;

static inline __attribute__((malloc)) void *malloc(size_t size)
{
	return mspace_malloc(libos_mspace, size);
}

static inline __attribute__((malloc)) void *memalign(size_t align, size_t size)
{
	return mspace_memalign(libos_mspace, size, align);
}

static inline void free(void *ptr)
{
	mspace_free(libos_mspace, ptr);
}
#elif defined(CONFIG_LIBOS_SIMPLE_ALLOC)
void *simple_alloc(size_t size, size_t align);
void simple_alloc_init(void *start, size_t size);

static inline __attribute__((malloc)) void *malloc(size_t size)
{
	return simple_alloc(size, 8);
}

static inline __attribute__((malloc)) void *memalign(size_t align, size_t size)
{
	return simple_alloc(size, align);
}

static inline void free(void *ptr)
{
}
#endif

#endif	/* __MALLOC_H */
