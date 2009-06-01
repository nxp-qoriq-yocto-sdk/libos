/*
 * Copyright (C) 2008 - 2009 Freescale Semiconductor, Inc.
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
#include <libos/printlog.h>

#define to_container(memberinstance, containertype, membername) ({ \
	typeof(&((containertype *)0)->membername) _ptr = (memberinstance); \
	(containertype *)(((uintptr_t)_ptr) - offsetof(containertype, membername)); \
})

#define stopsim() do { \
	asm volatile("mr 22, 22" : : : "memory"); \
	for(;;); \
} while (0)

#ifdef CONFIG_LIBOS_CONSOLE
extern void set_crashing(void);
#else
#define set_crashing()
#endif

#define BUG() do { \
	set_crashing(); \
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

#define align(x, y) (((x) + (y) - 1) & ~((y) - 1))

#ifndef HAVE_VIRT_TO_PHYS
static inline phys_addr_t virt_to_phys(void *ptr)
{
	return (uintptr_t)ptr - PHYSBASE;
}
#endif

typedef struct mem_resource {
	phys_addr_t start, size;
	void *virt;
} mem_resource_t;

struct driver;

typedef struct device {
	mem_resource_t *regs;
	struct interrupt **irqs;
	int num_regs;
	int num_irqs;
	struct driver *driver;
	void *data;

	/* Exposable interfaces */
	struct int_ops *irqctrl;
	struct chardev *chardev;
} device_t;

typedef struct driver {
	const char *compatible;
	int (*probe)(struct driver *driver, device_t *dev);
} driver_t;

#define __driver __attribute__((section(".libos.drivers"))) \
	__attribute__((used))

int libos_bind_driver(device_t *dev, const char *compat_strlist, size_t compat_len);

const char *strlist_iterate(const char *strlist, size_t len,
                            size_t *pos);

extern const int cache_block_size;

struct trapframe;
void return_hook(struct trapframe *regs);
void secondary_init(void);
void libos_client_entry(unsigned long devtree_ptr);

#ifdef __CHECKER__
#define __force __attribute__((force))
#define __bitwise __attribute__((bitwise))
#else
#define __force
#define __bitwise
#endif

#endif
