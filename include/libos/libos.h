/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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
#include <libos/errors.h>

#define to_container(memberinstance, containertype, membername) ({ \
	typeof(&((containertype *)0)->membername) _ptr = (memberinstance); \
	(containertype *)(((uintptr_t)_ptr) - offsetof(containertype, membername)); \
})

#define stopsim() do { \
	asm volatile("mr 22, 22" : : : "memory"); \
	for(;;); \
} while (0)

#ifdef CONFIG_LIBOS_CONSOLE
extern void set_crashing(int crashing);
#else
#define set_crashing(crashing)
#endif

#define BUG() do { \
	set_crashing(1); \
	printf("Assertion failure at %s:%d\n", __FILE__, __LINE__); \
	set_crashing(0); \
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

#define P4080REV1 0x82080010 /* SVR value for P4080 Rev1 */

/* Bit Field macros
 * 	v = bit field variable; m = mask, m##_SHIFT = shift, x = value to load
 */
#define set_bf(v, m, x)		(v = ((v) & ~(m)) | (((x) << (m##_SHIFT)) & (m)))
#define get_bf(v, m)		(((v) & (m)) >> (m##_SHIFT))

#ifndef HAVE_VIRT_TO_PHYS
 /* FIXME loading 32-bit images above 4 GiB */
extern unsigned long physbase_phys;

static inline phys_addr_t virt_to_phys(void *ptr)
{
	return (uintptr_t)ptr - PHYSBASE + physbase_phys;
}
#endif

/* Returns the current timebase taking care of erratum A-006958.
 * The erratum states that a 64-bit read of TBL may appear to
 * count backwards if the timebase increment causes a carry out
 * from TBL into TBU.
 *
 * The workaround consists in reading the TB by using the same procedure
 * as for 32-bit cpus.
 **/
static inline uint64_t get_tb(void)
{
	register_t tbl, tbu;
again:
	tbu = mfspr(SPR_TBU);
	tbl = mfspr(SPR_TBL);
	if (tbu != mfspr(SPR_TBU))
		goto again;

	return ((uint64_t) tbu << 32 | tbl);
}

#define SVR_T104x_CPU_FAMILY 	0x85200000
#define PVR_E5500		0x80241020

/* Reads the PVR with workaround for erratum A-008007 impacting
 * the t104x processor family.
 * The issue in the erratum is related to reset and causes PVR
 * contents to be unreliable. The workaround is to return the
 * hardcoded PVR if one of the impacted processors is detected
 * based on SVR.
 */
static inline register_t get_pvr(void)
{
	if ((mfspr(SPR_SVR) & 0xfff00000) == SVR_T104x_CPU_FAMILY)
		return PVR_E5500;
	else
		return mfspr(SPR_PVR);
}

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

typedef struct dev_compat {
	const char *compatible;
	void *data;
} dev_compat_t;

typedef struct driver {
	const dev_compat_t *compatibles;
	int (*probe)(device_t *dev, const dev_compat_t *compat_id);
} driver_t;

#define __driver __attribute__((section(".libos.drivers"))) \
	__attribute__((used))

int libos_bind_driver(device_t *dev, const char *compat_strlist, size_t compat_len);

const char *strlist_iterate(const char *strlist, size_t len,
                            size_t *pos);

const dev_compat_t *match_compat(const char *strlist, size_t len,
                                 const dev_compat_t *compat_list);

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

#ifdef CONFIG_LIBOS_HCALL_INSTRUCTIONS
int setup_hcall_instructions(uint8_t *opcodes, size_t len);
#else
static inline int setup_hcall_instructions(uint8_t *opcodes, size_t len)
{
	return ERR_INVALID;
}
#endif

#endif
