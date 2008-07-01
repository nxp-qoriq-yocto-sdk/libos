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

#ifndef LIBOS_H
#define LIBOS_H

#include <stddef.h>
#include <libos/types.h>
#include <libos/console.h>
#include <libos/malloc.h>

#define to_container(memberinstance, containertype, membername) ({ \
	typeof(memberinstance) _ptr = memberinstance; \
	(containertype *)(uintptr_t)_ptr - offsetof(containertype, membername); \
})

#define stopsim() do { \
	asm volatile("mr 22, 22" : : : "memory"); \
	for(;;); \
} while (0)

extern int crashing;

#define BUG() do { \
	crashing = 1; \
	printf("Assertion failure at %s:%d\n", __FILE__, __LINE__); \
	__builtin_trap(); \
} while (0)

#define assert(x) do { if (__builtin_expect(!(x), 0)) BUG(); } while (0)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define LONG_BITS (sizeof(long) * 8)

#define max(x, y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; \
})

#define min(x, y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; \
})

#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */

typedef uint64_t phys_addr_t;
typedef unsigned long register_t;

void *simple_alloc(size_t size, size_t align);
void simple_alloc_init(void *start, size_t size);

extern mspace libos_mspace;

static inline void *alloc(unsigned long size, size_t align)
{
	void *ret;

#ifdef CONFIG_LIBOS_MALLOC
	if (__builtin_constant_p(align) && align <= 8)
		ret = mspace_malloc(libos_mspace, size);
	else
		ret = mspace_memalign(libos_mspace, align, size);
#else
	ret = simple_alloc(size, align);
#endif

	memset(ret, 0, size);
	return ret;
}

#define alloc_type(T) alloc(sizeof(T), __alignof__(T))

void *valloc(unsigned long size, unsigned long align);
void valloc_init(unsigned long start, unsigned long end);

#ifdef CONFIG_LIBOS_MALLOC
static inline void *malloc(size_t size)
{
	return mspace_malloc(libos_mspace, size);
}

static inline void free(void *ptr)
{
	mspace_free(libos_mspace, ptr);
}
#endif

#endif
