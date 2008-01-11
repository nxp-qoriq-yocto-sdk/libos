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
static unsigned long heap_start, heap_end;
static uint32_t alloc_lock;

void *alloc(unsigned long size, unsigned long align)
{
	spin_lock(&alloc_lock);

	unsigned long new_heap = (heap_start + align - 1) & ~(align - 1);
	void *ret = (void *)new_heap;
	new_heap += size;

	if (new_heap > heap_end || new_heap < heap_start)
		ret = NULL;
	else
		heap_start = new_heap;

	spin_unlock(&alloc_lock);

	if (ret)
		memset(ret, 0, size);

	return ret;
}

void alloc_init(unsigned long _heap_start, unsigned long _heap_end)
{
	heap_start = _heap_start;
	heap_end = _heap_end;
}