/*
 * Memory allocation -- currently a simple alloc-and-never-free
 * pointer.
 *
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Author: Scott Wood <scottwood@freescale.com>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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

#include <string.h>
#include <libos/bitops.h>

extern int _end;

typedef struct {	
	unsigned long start, end;
} allocator;

static allocator heap, virtual;
static uint32_t alloc_lock;

static void *__alloc(allocator *a, unsigned long size, unsigned long align)
{
	register_t saved = spin_lock_critsave(&alloc_lock);

	unsigned long new_start = (a->start + align - 1) & ~(align - 1);
	void *ret = (void *)new_start;
	new_start += size;

	if (new_start >= a->end || new_start < a->start)
		ret = NULL;
	else
		a->start = new_start;

	spin_unlock_critsave(&alloc_lock, saved);
	return ret;
}

void *alloc(unsigned long size, unsigned long align)
{
	void *ret = __alloc(&heap, size, align);

	if (ret)
		memset(ret, 0, size);

	return ret;
}

void *valloc(unsigned long size, unsigned long align)
{
	return __alloc(&virtual, size, align);
}

void alloc_init(unsigned long start, unsigned long end)
{
	heap.start = start;
	heap.end = end;
}

void valloc_init(unsigned long start, unsigned long end)
{
	virtual.start = start;
	virtual.end = end;
}
