/*
 * Memory allocation using dlmalloc
 *
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
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

#include <libos/io.h>
#include <libos/malloc.h>
#include <libos/printlog.h>
#include <libos/libos.h>

mspace libos_mspace;

typedef struct {
	void *start, *end;
} segment_t;

#define NUM_SEGMENTS 64

static segment_t segments[NUM_SEGMENTS];
static int nextseg = 0;

void malloc_add_segment(void *start, void *end)
{
	if (nextseg < NUM_SEGMENTS) {
		segments[nextseg].start = start;
		segments[nextseg].end = end;
		nextseg++;
	} else {
		printlog(LOGTYPE_MALLOC, LOGLEVEL_ERROR,
		         "malloc_add_segment: discarded %d bytes at 0x%p\n",
		         end - start + 1, start);
	}
}

void malloc_exclude_segment(void *start, void *end)
{
	for (int i = 0; i < nextseg; i++) {
		segment_t *s = &segments[i];

		if (start > s->end)
			continue;

		if (end < s->start)
			continue;

		if (start > s->start) {
			if (s->end > end)
				malloc_add_segment(end + 1, s->end);
		
			s->end = start - 1;
		} else if (end < s->end) {
			s->start = end + 1;
		} else {
			*s = segments[--nextseg];
		}
	}
}

static int next_usable_segment(int next, ssize_t *size)
{
	while (next < nextseg) {
		*size = segments[next].end - segments[next].start + 1;
	
		if (*size >= 1024) {
			printlog(LOGTYPE_MALLOC, LOGLEVEL_NORMAL,
			         "malloc_init: using %ld %ciB at 0x%p - 0x%p\n",
			         *size >= 1024 * 1024 ? *size / (1024 * 1024) : *size / 1024,
			         *size >= 1024 * 1024 ? 'M' : 'K',
			         segments[next].start, segments[next].end);

			return next;
		}

		printlog(LOGTYPE_MALLOC, LOGLEVEL_NORMAL,
		         "malloc_init: discarded %ld bytes at 0x%p (too small)\n",
		         *size, segments[next].start);

		next++;
	}
	
	return -1;
}

mspace malloc_init(void)
{
	ssize_t size;
	int next;
	
	next = next_usable_segment(0, &size);
	if (next < 0) {
		printlog(LOGTYPE_MALLOC, LOGLEVEL_ALWAYS,
		         "malloc_init: No suitable memory\n");
		return NULL;
	}

	libos_mspace = create_mspace_with_base(segments[next].start, size, 1);
	if (!libos_mspace) {
		printlog(LOGTYPE_MALLOC, LOGLEVEL_ERROR,
		         "malloc_init: Failed to create mspace.\n");
		return NULL;
	}

	while ((next = next_usable_segment(next + 1, &size)) >= 0)
		mspace_add_segment(libos_mspace, segments[next].start, size);

	return libos_mspace;
}
