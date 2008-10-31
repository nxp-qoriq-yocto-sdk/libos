/** @file
 * Linked lists
 */
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

#ifndef LIBOS_LIST_H
#define LIBOS_LIST_H

#include <stddef.h>

typedef struct list {
	struct list *next, *prev;
} list_t;

static inline void list_init(list_t *list)
{
	list->next = list->prev = list;
}

static inline void list_add(list_t *list, list_t *node)
{
	list_t *prev = list->prev;

	prev->next = node;
	node->prev = prev;
	node->next = list;
	list->prev = node;
}

static inline void list_del(list_t *node)
{
	list_t *prev = node->prev;
	list_t *next = node->next;
	
	prev->next = next;
	next->prev = prev;
	
	node->next = node->prev = NULL;
}

#define list_for_each(list, iter) \
	for (list_t *(iter) = (list)->next; (iter) != (list); (iter) = (iter)->next)

#define list_for_each_delsafe(list, iter, tmp) \
	for (list_t *(iter) = (list)->next, \
	     *(tmp) = (iter)->next; \
	     (iter) != (list); \
	     (iter) = (tmp), \
	     (tmp) = (iter)->next)

static inline int list_empty(list_t *list)
{
	return list->next == list;
}

#endif
