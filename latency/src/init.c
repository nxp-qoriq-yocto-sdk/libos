
/*
 * Copyright (C) 2009-2011 Freescale Semiconductor, Inc.
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



#include <libos/libos.h>
#include <libos/percpu.h>
#include <libos/fsl-booke-tlb.h>
#include <libos/trapframe.h>
#include <libos/uart.h>
#include <libos/ns16550.h>
#include <libos/errors.h>
#include <libos/alloc.h>
#include <libos/core-regs.h>
#include <libos/bitops.h>
#include <libos/io.h>
#include <libos/chardev.h>
#include <libos/console.h>
#include <libos/epapr_hcalls.h>

#include <malloc.h>
#include <libfdt.h>

extern uint8_t init_stack_top;

cpu_t cpu0 = {
	.kstack = &init_stack_top - FRAMELEN,
	.client = 0,
};

void *mpic_regs;
void *ipi_regs, *timer_regs; /* may be from a different MPIC than mpic_regs */
void *fdt;

// Timestamps
static volatile unsigned long t1, t3;

#define PAGE_SIZE 4096UL

#define MAX_DT_PATH 256

#define MAX_ADDR_CELLS 4
#define MAX_SIZE_CELLS 2
#define MAX_INT_CELLS 4

#define CELL_SIZE 4

#define MPIC_CTPR	0x0080
#define MPIC_EOI	0x00B0
#define MPIC_IACK	0x00A0
#define MPIC_IPIVPR0	0x10A0
#define MPIC_GTVPRA0	0x1120

/* Offset from timer base */
#define MPIC_TIMER_BCR0 0x10

/* Must be a power of 2 */
#define NUM_TIMESTAMPS	16

static volatile unsigned long t[NUM_TIMESTAMPS];
static volatile unsigned int index;

static inline void mpic_write(uint32_t reg, uint32_t val)
{
	out32(mpic_regs + reg, val);
}

static inline register_t mpic_read(uint32_t reg)
{
	return in32(mpic_regs + reg);
}

static int coreint;

void ext_int_handler(trapframe_t *frameptr)
{
	int vec;

	if (coreint)
		vec = mfspr(SPR_EPR);
	else
		vec = mpic_read(MPIC_IACK);

	if (vec == 0) {
		t[index] = mfspr(SPR_TBL);
		index = (index + 1) & (NUM_TIMESTAMPS - 1);
	}

	/* Don't EOI spurious interrupts.
	 * See arch/powerpc/include/asm/kvm_para.h for why we get
	 * spurious interrupts (it should be rare in real usage).
	 */
	if (vec != 0xffff)
		mpic_write(MPIC_EOI, 0);
}

static int get_stdout(void)
{
	const char *path;
	int node, len;

	node = fdt_subnode_offset(fdt, 0, "chosen");
	if (node < 0)
		return node;

	path = fdt_getprop(fdt, node, "linux,stdout-path", &len);
	if (!path)
		return ERR_NOTFOUND;

	node = fdt_path_offset(fdt, path);
	if (node < 0)
		return ERR_BADTREE;

	return node;
}

uint32_t dt_get_timebase_freq(void)
{
	int node;
	int len;

	node = fdt_node_offset_by_prop_value(fdt, -1, "device_type", "cpu", strlen("cpu")+1);

	if (node == -FDT_ERR_NOTFOUND) {
		printf("cpu node not found\n");
		return -1;
	}

        const uint32_t *tb = fdt_getprop(fdt, node, "timebase-frequency", &len);
	if (!tb) {
		printf("timebase not found\n");
		return -1;
	}

	return (uint32_t)*tb;
}

unsigned long tb_to_nsec(uint64_t freq, unsigned long ticks)
{

	return ticks * 1000000000ULL / freq;
}


int get_addr_format(const void *tree, int node,
		    uint32_t *naddr, uint32_t *nsize)
{
	*naddr = 2;
	*nsize = 1;

	int len;
	const uint32_t *naddrp = fdt_getprop(tree, node, "#address-cells", &len);
	if (!naddrp) {
		if (len != -FDT_ERR_NOTFOUND)
			return len;
	} else if (len == 4 && *naddrp <= MAX_ADDR_CELLS) {
		*naddr = *naddrp;
	} else {
		printlog(LOGTYPE_MISC, LOGLEVEL_NORMAL,
			 "Bad addr cells %d\n", *naddrp);
		return ERR_BADTREE;
	}

	const uint32_t *nsizep = fdt_getprop(tree, node, "#size-cells", &len);
	if (!nsizep) {
		if (len != -FDT_ERR_NOTFOUND)
			return len;
	} else if (len == 4 && *nsizep <= MAX_SIZE_CELLS) {
		*nsize = *nsizep;
	} else {
		printlog(LOGTYPE_MISC, LOGLEVEL_NORMAL,
			 "Bad size cells %d\n", *nsizep);
		return ERR_BADTREE;
	}

	return 0;
}

int get_addr_format_nozero(const void *tree, int node,
			   uint32_t *naddr, uint32_t *nsize)
{
	int ret = get_addr_format(tree, node, naddr, nsize);
	if (!ret && (*naddr == 0 || *nsize == 0)) {
		printlog(LOGTYPE_MISC, LOGLEVEL_NORMAL,
			 "Bad addr/size cells %d/%d\n", *naddr, *nsize);

		ret = ERR_BADTREE;
	}

	return ret;
}

void copy_val(uint32_t *dest, const uint32_t *src, int naddr)
{
	int pad = MAX_ADDR_CELLS - naddr;

	memset(dest, 0, pad * 4);
	memcpy(dest + pad, src, naddr * 4);
}

static int sub_reg(uint32_t *reg, const uint32_t *sub)
{
	int i, borrow = 0;

	for (i = MAX_ADDR_CELLS - 1; i >= 0; i--) {
		int prev_borrow = borrow;
		borrow = reg[i] < sub[i] + prev_borrow;
		reg[i] -= sub[i] + prev_borrow;
	}

	return !borrow;
}

static int add_reg(uint32_t *reg, const uint32_t *add, int naddr)
{
	int i, carry = 0;

	for (i = MAX_ADDR_CELLS - 1; i >= MAX_ADDR_CELLS - naddr; i--) {
		uint64_t tmp = (uint64_t)reg[i] + add[i] + carry;
		carry = tmp >> 32;
		reg[i] = (uint32_t)tmp;
	}

	return !carry;
}

/* FIXME: It is assumed that if the first byte of reg fits in a
 * range, then the whole reg block fits.
 */
static int compare_reg(const uint32_t *reg, const uint32_t *range,
		       const uint32_t *rangesize)
{
	uint32_t end[MAX_ADDR_CELLS];
	int i;

	for (i = 0; i < MAX_ADDR_CELLS; i++) {
		if (reg[i] < range[i])
			return 0;
		if (reg[i] > range[i])
			break;
	}

	memcpy(end, range, sizeof(end));

	/* If the size forces a carry off the final cell, then
	 * reg can't possibly be beyond the end.
	 */
	if (!add_reg(end, rangesize, MAX_ADDR_CELLS))
		return 1;

	for (i = 0; i < MAX_ADDR_CELLS; i++) {
		if (reg[i] < end[i])
			return 1;
		if (reg[i] > end[i])
			return 0;
	}

	return 0;
}

/* reg must be MAX_ADDR_CELLS */
static int find_range(const uint32_t *reg, const uint32_t *ranges,
		      int nregaddr, int naddr, int nsize, int buflen)
{
	int nrange = nregaddr + naddr + nsize;
	int i;

	if (nrange <= 0)
		return ERR_BADTREE;

	for (i = 0; i < buflen; i += nrange) {
		uint32_t range_addr[MAX_ADDR_CELLS];
		uint32_t range_size[MAX_ADDR_CELLS];

		if (i + nrange > buflen) {
			return ERR_BADTREE;
		}

		copy_val(range_addr, ranges + i, nregaddr);
		copy_val(range_size, ranges + i + nregaddr + naddr, nsize);

		if (compare_reg(reg, range_addr, range_size))
			return i;
	}

	return -FDT_ERR_NOTFOUND;
}

/* Currently only generic buses without special encodings are supported.
 * In particular, PCI is not supported.  Also, only the beginning of the
 * reg block is tracked; size is ignored except in ranges.
 */
int xlate_one(uint32_t *addr, const uint32_t *ranges,
	      int rangelen, uint32_t naddr, uint32_t nsize,
	      uint32_t prev_naddr, uint32_t prev_nsize,
	      phys_addr_t *rangesize)
{
	uint32_t tmpaddr[MAX_ADDR_CELLS], tmpaddr2[MAX_ADDR_CELLS];
	int offset = find_range(addr, ranges, prev_naddr,
				naddr, prev_nsize, rangelen / 4);

	if (offset < 0)
		return offset;

	ranges += offset;

	copy_val(tmpaddr, ranges, prev_naddr);

	if (!sub_reg(addr, tmpaddr))
		return ERR_BADTREE;

	if (rangesize) {
		copy_val(tmpaddr, ranges + prev_naddr + naddr, prev_nsize);

		if (!sub_reg(tmpaddr, addr))
			return ERR_BADTREE;

		*rangesize = ((uint64_t)tmpaddr[2]) << 32;
		*rangesize |= tmpaddr[3];
	}

	copy_val(tmpaddr, ranges + prev_naddr, naddr);

	if (!add_reg(addr, tmpaddr, naddr))
		return ERR_BADTREE;

	/* Reject ranges that wrap around the address space.  Primarily
	 * intended to enable blacklist entries in fsl,hvranges.
	 */
	copy_val(tmpaddr, ranges + prev_naddr, naddr);
	copy_val(tmpaddr2, ranges + prev_naddr + naddr, nsize);

	if (!add_reg(tmpaddr, tmpaddr2, naddr))
		return ERR_NOTRANS;

	return 0;
}

int xlate_reg_raw(const void *tree, int node, const uint32_t *reg,
		  uint32_t *addrbuf, phys_addr_t *size,
		  uint32_t naddr, uint32_t nsize)
{
	uint32_t prev_naddr, prev_nsize;
	const uint32_t *ranges;
	int len, ret;

	int parent = fdt_parent_offset(tree, node);
	if (parent < 0)
		return parent;

	copy_val(addrbuf, reg, naddr);

	if (size) {
		*size = reg[naddr];
		if (nsize == 2) {
			*size <<= 32;
			*size |= reg[naddr + 1];
		}
	}

	for (;;) {
		prev_naddr = naddr;
		prev_nsize = nsize;
		node = parent;

		parent = fdt_parent_offset(tree, node);
		if (parent == -FDT_ERR_NOTFOUND)
			break;
		if (parent < 0)
			return parent;

		ret = get_addr_format(tree, parent, &naddr, &nsize);
		if (ret < 0)
			return ret;

		ranges = fdt_getprop(tree, node, "ranges", &len);
		if (!ranges) {
			if (len == -FDT_ERR_NOTFOUND)
				return ERR_NOTRANS;

			return len;
		}

		if (len == 0)
			continue;
		if (len % 4)
			return ERR_BADTREE;

		ret = xlate_one(addrbuf, ranges, len, naddr, nsize,
				prev_naddr, prev_nsize, NULL);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int xlate_reg(const void *tree, int node, const uint32_t *reg,
	      phys_addr_t *addr, phys_addr_t *size)
{
	uint32_t addrbuf[MAX_ADDR_CELLS];
	uint32_t naddr, nsize;

	int parent = fdt_parent_offset(tree, node);
	if (parent < 0)
		return parent;

	int ret = get_addr_format(tree, parent, &naddr, &nsize);
	if (ret < 0)
		return ret;

	ret = xlate_reg_raw(tree, node, reg, addrbuf, size, naddr, nsize);
	if (ret < 0)
		return ret;

	if (addrbuf[0] || addrbuf[1])
		return ERR_BADTREE;

	*addr = ((uint64_t)addrbuf[2] << 32) | addrbuf[3];
	return 0;
}

int dt_get_reg(const void *tree, int node, int res,
	       phys_addr_t *addr, phys_addr_t *size)
{
	int ret, len;
	uint32_t naddr, nsize;
	const uint32_t *reg = fdt_getprop(tree, node, "reg", &len);
	if (!reg)
		return len;

	int parent = fdt_parent_offset(tree, node);
	if (parent < 0)
		return parent;

	ret = get_addr_format(tree, parent, &naddr, &nsize);
	if (ret < 0)
		return ret;

	if (naddr == 0 || nsize == 0)
		return ERR_NOTRANS;

	if ((unsigned int)len < (naddr + nsize) * 4 * (res + 1))
		return ERR_BADTREE;

	return xlate_reg(tree, node, &reg[(naddr + nsize) * res], addr, size);
}

phys_addr_t uart_addr;
void *uart_virt;

static void tlb1_init(void)
{
	if (uart_virt)
		tlb1_set_entry(UART_TLB_ENTRY, (uintptr_t)uart_virt,
		               uart_addr, TLB_TSIZE_4K, TLB_MAS2_IO,
		               TLB_MAS3_KDATA, 0, 0, 0, 0);

	cpu->console_ok = 1;
}

chardev_t *test_init_uart(int node)
{
	uint32_t baud = 115200;

	if (dt_get_reg(fdt, node, 0, &uart_addr, NULL) < 0)
		return NULL;

	uart_virt = valloc(2 * PAGE_SIZE, PAGE_SIZE);
	uart_virt += uart_addr & (PAGE_SIZE - 1);

	tlb1_init();

	uint64_t freq = 0;
	int len;
	const uint32_t *prop = fdt_getprop(fdt, node, "clock-frequency", &len);
	if (prop && len == 4) {
		freq = *prop;
	} else if (prop && len == 8) {
		freq = *(const uint64_t *)prop;
	} else {
		printlog(LOGTYPE_DEV, LOGLEVEL_ERROR,
			 "%s: bad/missing clock-frequency\n", __func__);
	}

	prop = fdt_getprop(fdt, node, "current-speed", &len);
	if (prop) {
		if (len == 4)
			baud = *(const uint32_t *)prop;
		else
			printlog(LOGTYPE_DEV, LOGLEVEL_NORMAL,
				 "%s: bad current-speed property\n", __func__);
	}

	return ns16550_init(uart_virt, NULL, freq, 16, baud);
}

#define KVM_HC_FEATURES            3
#define KVM_HC_PPC_MAP_MAGIC_PAGE  4

static inline unsigned int kvm_get_features(uint32_t *features)
{
	register uintptr_t r11 __asm__("r11");
	register uintptr_t r3 __asm__("r3");
	register uintptr_t r4 __asm__("r4");

	r11 = _EV_HCALL_TOKEN(EV_KVM_VENDOR_ID, KVM_HC_FEATURES);

	__asm__ __volatile__ (EV_HCALL_RESOLVER
		: "+r" (r11), "=r" (r3), "=r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	*features = r4;

	return r3;
}

static inline unsigned int kvmppc_map_magic_page(unsigned long addr,
                                                 uint32_t *features)
{
	register uintptr_t r11 __asm__("r11");
	register uintptr_t r3 __asm__("r3");
	register uintptr_t r4 __asm__("r4");

	r11 = _EV_HCALL_TOKEN(EV_KVM_VENDOR_ID, KVM_HC_PPC_MAP_MAGIC_PAGE);
	r3 = addr;
	r4 = addr;

	__asm__ __volatile__ (EV_HCALL_RESOLVER
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	*features = r4;

	return r3;
}

#define CELL_SIZE 4

static int have_kvm;
static uint32_t kvm_features, kvm_magic_features;

#define KVM_FEATURE_MAGIC_PAGE       0x2
#define KVM_MAGIC_FEAT_SR            0x1
#define KVM_MAGIC_FEAT_MAS0_TO_SPRG7 0x2
#define KVM_MAGIC_FEAT_EPR           0x4
#define KVM_MAGIC_FEAT_MPIC_CTPR     0x8

#define MAGIC_ADDR 0xfffff000
#define MAGIC_OFFSET(x) (((x) + MAGIC_ADDR) & 0xffff)

typedef struct spr_patch {
	uint32_t insn;
	int offset;
	int bits32;
	uint32_t features;
} spr_patch_t;

#define KVM_MAGIC_OFF_SPRG0           32
#define KVM_MAGIC_OFF_SPRG1           40
#define KVM_MAGIC_OFF_SPRG2           48
#define KVM_MAGIC_OFF_SPRG3           56
#define KVM_MAGIC_OFF_SPRG4           208
#define KVM_MAGIC_OFF_SPRG5           216
#define KVM_MAGIC_OFF_SPRG6           224
#define KVM_MAGIC_OFF_SPRG7           232
#define KVM_MAGIC_OFF_DEAR            80
#define KVM_MAGIC_OFF_ESR             200
#define KVM_MAGIC_OFF_SRR0            64
#define KVM_MAGIC_OFF_SRR1            72
#define KVM_MAGIC_OFF_PIR             204
#define KVM_MAGIC_OFF_EPR             240
#define KVM_MAGIC_OFF_MAS0            168
#define KVM_MAGIC_OFF_MAS1            172
#define KVM_MAGIC_OFF_MAS2            184
#define KVM_MAGIC_OFF_MAS3            180
#define KVM_MAGIC_OFF_MAS4            192
#define KVM_MAGIC_OFF_MAS6            196
#define KVM_MAGIC_OFF_MAS7            176
#define KVM_MAGIC_OFF_MPIC_CTPR       244
#define KVM_MAGIC_OFF_MPIC_PRIO_PEND  248

extern uint32_t text_start, text_end;

#define SPR_TO 0x100

#define SPR_INST(spr) (0x7c0002a6 | \
                       (((spr) & 0x1f) << 16) | \
                       (((spr) & 0x3e0) << 6))

static struct spr_patch spr_patches[] = {
	{ .insn = SPR_INST(SPR_SPRG0), .offset = KVM_MAGIC_OFF_SPRG0, },
	{ .insn = SPR_INST(SPR_SPRG1), .offset = KVM_MAGIC_OFF_SPRG1, },
	{ .insn = SPR_INST(SPR_SPRG2), .offset = KVM_MAGIC_OFF_SPRG2, },
	{ .insn = SPR_INST(SPR_SPRG3), .offset = KVM_MAGIC_OFF_SPRG3, },
	{ .insn = SPR_INST(SPR_SPRG4), .offset = KVM_MAGIC_OFF_SPRG4,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_SPRG5), .offset = KVM_MAGIC_OFF_SPRG5,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_SPRG6), .offset = KVM_MAGIC_OFF_SPRG6,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_SPRG7), .offset = KVM_MAGIC_OFF_SPRG7,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_USPRG4), .offset = KVM_MAGIC_OFF_SPRG4,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_USPRG5), .offset = KVM_MAGIC_OFF_SPRG5,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_USPRG6), .offset = KVM_MAGIC_OFF_SPRG6,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_USPRG7), .offset = KVM_MAGIC_OFF_SPRG7,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_DEAR), .offset = KVM_MAGIC_OFF_DEAR,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_ESR), .offset = KVM_MAGIC_OFF_ESR,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_SRR0), .offset = KVM_MAGIC_OFF_SRR0,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_SRR1), .offset = KVM_MAGIC_OFF_SRR1,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_PIR), .offset = KVM_MAGIC_OFF_PIR,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_EPR), .offset = KVM_MAGIC_OFF_EPR,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_EPR },
	{ .insn = SPR_INST(SPR_MAS0), .offset = KVM_MAGIC_OFF_MAS0,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS1), .offset = KVM_MAGIC_OFF_MAS1,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS2), .offset = KVM_MAGIC_OFF_MAS2,
	  .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS3), .offset = KVM_MAGIC_OFF_MAS3,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS4), .offset = KVM_MAGIC_OFF_MAS4,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS6), .offset = KVM_MAGIC_OFF_MAS6,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
	{ .insn = SPR_INST(SPR_MAS7), .offset = KVM_MAGIC_OFF_MAS7,
	  .bits32 = 1, .features = KVM_MAGIC_FEAT_MAS0_TO_SPRG7 },
};

#define SPR_REG_MASK  0x03e00000

#define LWZ    0x80000000
#define STW    0x90000000

/* FIXME 64-bit chip */
static void patch_spr(struct spr_patch *p, uint32_t *insn)
{
	/* reg is in same location for load/store as mfspr/mtspr */
	uint32_t new_insn = *insn & SPR_REG_MASK;

	if (p->bits32) {
		if (*insn & SPR_TO)
			new_insn |= STW | MAGIC_OFFSET(p->offset);
		else
			new_insn |= LWZ | MAGIC_OFFSET(p->offset);
	} else {
		if (*insn & SPR_TO)
			new_insn |= STW | MAGIC_OFFSET(p->offset + 4);
		else
			new_insn |= LWZ | MAGIC_OFFSET(p->offset + 4);
	}

	*insn = new_insn;
	icache_block_sync(insn);
}

static uint32_t mpic_read_ctpr(void)
{
	return mpic_read(MPIC_CTPR);
}

static uint32_t kvm_read_ctpr(void)
{
	return *(uint32_t *)(MAGIC_ADDR + KVM_MAGIC_OFF_MPIC_CTPR);
}

static void mpic_write_ctpr(uint32_t ctpr)
{
	mpic_write(MPIC_CTPR, ctpr);
}

static void kvm_write_ctpr(uint32_t ctpr)
{
	/* This sync is to make sure that, when writing to CTPR to end a
	 * critical section, all of the caller's memory accesses have
	 * completed.  This is done here because the paravirt CTPR is
	 * cacheable, unlike the real CTPR, and the caller may have used
	 * "mbar 1" to order non-cacheable accesses in the critical section
	 * with the end of the critical section.  This allows the paravirt
	 * to be a drop-in replacement fo a store to the real CTPR.
	 *
	 * This sync can be removed if the above does not apply to your
	 * application.
	 */
	smp_sync();

	*(uint32_t *)(MAGIC_ADDR + KVM_MAGIC_OFF_MPIC_CTPR) = ctpr;

	/* This sync is needed to ensure that either KVM sees the new CTPR,
	 * or we see the new PRIO_PEND.
	 */
	smp_sync();

	/* Can change "> ctpr" to "!= 0" to avoid
	 * spurious interrupt generation (see ext_int_handler).
	 */
	if (*(uint32_t *)(MAGIC_ADDR + KVM_MAGIC_OFF_MPIC_PRIO_PEND) > ctpr)
		mpic_write_ctpr(ctpr);
}

static uint32_t (*read_ctpr)(void) = mpic_read_ctpr;
static void (*write_ctpr)(uint32_t ctpr) = mpic_write_ctpr;

static void init_kvm_magic(void)
{
	uint32_t *insn;

	if (kvmppc_map_magic_page(MAGIC_ADDR, &kvm_magic_features))
		return;

	for (insn = &text_start; insn < &text_end; insn++) {
		int i;

		for (i = 0; i < sizeof(spr_patches) / sizeof(spr_patch_t);
		     i++) {
			spr_patch_t *p = &spr_patches[i];

			if ((*insn & ~(SPR_REG_MASK | SPR_TO)) == p->insn)
				patch_spr(p, insn);
		}
	}

	if (kvm_magic_features & KVM_MAGIC_FEAT_EPR)
		coreint = 1;

	if (kvm_magic_features & KVM_MAGIC_FEAT_MPIC_CTPR) {
		read_ctpr = kvm_read_ctpr;
		write_ctpr = kvm_write_ctpr;
	}

	printf("KVM: paravirt on\n");
}

static void init_kvm(void)
{
	have_kvm = 1;

	if (kvm_get_features(&kvm_features))
		return;

	if (kvm_features & KVM_FEATURE_MAGIC_PAGE)
		init_kvm_magic();
}

static void init_hv(int node)
{
	const struct fdt_property *pdata;
	uint32_t hcall_opcode[4] = {};
	int size, ret, i;

	pdata = fdt_get_property(fdt, node, "hcall-instructions", &size);
	if (!pdata || size > 4 * CELL_SIZE || size % CELL_SIZE) {
		/* no hcalls */
		return;
	}

	/* Read all opcodes, at most 4 */
	for (i = 0; i < size / CELL_SIZE; i++)
		hcall_opcode[i] = *((uint32_t *)pdata->data + i);

	ret = setup_hcall_instructions((uint8_t *)hcall_opcode, size);
	if (ret < 0)
		return;

	/* hcall mechanism OK, now check for hv type */
	if (!fdt_node_check_compatible(fdt, node, "linux,kvm"))
		init_kvm();
}

void init(unsigned long devtree_ptr)
{
	int dtmap_tsize = TLB_TSIZE_4M;
	unsigned long dtmap_size = tsize_to_pages(dtmap_tsize) * 4096;
	unsigned long dtmap_base = 0x80000000;
	phys_addr_t mpic_paddr, ipi_paddr, timer_paddr;
	chardev_t *stdout;
	int node;

	/* We may be running at a physical address other than zero,
	 * so create a separate device tree mapping.  Assume it doesn't
	 * cross a 4M boundary for now.
	 */
	tlb1_set_entry(DEVTREE_TLB_ENTRY, dtmap_base,
	               devtree_ptr & ~(dtmap_size - 1), dtmap_tsize,
	               TLB_MAS2_MEM, TLB_MAS3_KDATA, 0, 0, 0, 0);

	fdt = (void *)(dtmap_base + (devtree_ptr & (dtmap_size - 1)));

	/* alloc the heap */
	uintptr_t heap = (unsigned long)fdt + fdt_totalsize(fdt);
	heap = (heap + 15) & ~15;

	simple_alloc_init((void *)heap, 0x100000); // FIXME: hardcoded 1MB heap
	valloc_init(1024 * 1024, PHYSBASE);

	node = get_stdout();
	if (node >= 0) {
		if (!fdt_node_check_compatible(fdt, node, "ns16550"))
			stdout = test_init_uart(node);
		else {
			printf("Unrecognized stdout compatible.\n");
			stdout = NULL;
		}

		if (stdout)
			console_init(stdout);
		else
			/* The message will at least go to the log buffer... */
			printf("Failed to initialize stdout.\n");
	} else {
		printf("No stdout found.\n");
	}

	node = fdt_path_offset(fdt, "/hypervisor");
	if (node >= 0)
		init_hv(node );

	node = fdt_node_offset_by_compatible(fdt, 0, "chrp,open-pic");
	dt_get_reg(fdt, node, 0, &mpic_paddr, NULL);

	node = fdt_node_offset_by_compatible(fdt, 0, "fsl,mpic-ipi");
	if (node >= 0)
		dt_get_reg(fdt, node, 0, &ipi_paddr, NULL);
	else
		ipi_paddr = mpic_paddr + 0x40;

	node = fdt_path_offset(fdt, "timer0");
	if (node >= 0)
		dt_get_reg(fdt, node, 0, &timer_paddr, NULL);
	else
		timer_paddr = mpic_paddr + 0x1100;

	mpic_regs = valloc(4 * PAGE_SIZE, 4 * PAGE_SIZE);
	ipi_regs = valloc(PAGE_SIZE, PAGE_SIZE);
	timer_regs = valloc(PAGE_SIZE, PAGE_SIZE);

	tlb1_set_entry(MPIC_TLB_ENTRY, (uintptr_t)mpic_regs, mpic_paddr,
	               TLB_TSIZE_16K, TLB_MAS2_IO, TLB_MAS3_KDATA, 0, 0, 0, 0);
	tlb1_set_entry(IPI_TLB_ENTRY, (uintptr_t)ipi_regs,
	               ipi_paddr & ~(PAGE_SIZE - 1), TLB_TSIZE_4K,
	               TLB_MAS2_IO, TLB_MAS3_KDATA, 0, 0, 0, 0);
	tlb1_set_entry(TIMER_TLB_ENTRY, (uintptr_t)timer_regs,
	               timer_paddr & ~(PAGE_SIZE - 1), TLB_TSIZE_4K,
	               TLB_MAS2_IO, TLB_MAS3_KDATA, 0, 0, 0, 0);

	ipi_regs += ipi_paddr & (PAGE_SIZE - 1);
	timer_regs += timer_paddr & (PAGE_SIZE - 1);
}

void libos_client_entry(unsigned long devtree_ptr)
{
	volatile unsigned long t1, t3;
	volatile unsigned long t2[NUM_TIMESTAMPS];
	unsigned int i;
	unsigned int count = 20;

	init(devtree_ptr);

	uint32_t tb_freq = dt_get_timebase_freq();

	printf("Interrupt latency measurement tool %s %s\n", __DATE__, __TIME__);
	printf("L1: time (ns) from trigger to ISR.\n");
	printf("L2: time (ns) from trigger to return from ISR.\n");

	/* Disable all core timer interrupts -- we don't have a handler,
	 * and we don't know what state the loader left them in.
	 */
	mtspr(SPR_TCR, 0);
	isync();
	enable_int();

	write_ctpr(0);
	mpic_write(MPIC_IPIVPR0, 0xF0000);

	index = 0;
	while (--count) {
		i = index;
		sync();
		t3 = 0;
		t[i] = 0;
		t1 = mfspr(SPR_TBL);
		isync(); // We want the mpic_write() to occur after the mfspr()
		out32(ipi_regs, 1); // Trigger an interrupt
		while (t[i] == 0);
		t3 = mfspr(SPR_TBL);
		printf("L1=%lu L2=%lu\n",
		       tb_to_nsec(tb_freq,t[i] - t1),
		       tb_to_nsec(tb_freq, t3 - t1));
	}
	mpic_write(MPIC_IPIVPR0, 0);

	printf("\nInterrupt jitter measurement tool %s %s\n", __DATE__, __TIME__);
	printf("T1 is based on timestamp in ISR.\n");
	printf("T2 is based on timestamp after return from ISR.\n");
	printf("Results are difference in timestamps (ns) between interrupts.\n");

	index = 0;
	mpic_write(MPIC_GTVPRA0, 0xF0000);
	out32(timer_regs + MPIC_TIMER_BCR0, 100000);
	while (index < (NUM_TIMESTAMPS - 1)) {
		i = index;
		sync();
		while (i == index); /* Wait until ISR runs and then exits */
		t2[i] = mfspr(SPR_TBL);
	}
	out32(timer_regs + MPIC_TIMER_BCR0, 0x80000000);

	for (i = 1; i < NUM_TIMESTAMPS - 1; i++)
		printf("T1=%lu T2=%lu\n",
		       tb_to_nsec(tb_freq, t[i] - t[i - 1]),
		       tb_to_nsec(tb_freq, t2[i] - t2[i - 1]));

	printf("CTPR test (raise CTPR on pending IRQ):\n");
	printf("Time is latency from raising CTPR to ISR.\n");
	index = 0;

	write_ctpr(15);
	mpic_write(MPIC_IPIVPR0, 0xF0000);
	count = 5;
	while (--count) {
		unsigned long delay;
		i = index;
		sync();
		t[i] = 0;
		out32(ipi_regs, 1); // Trigger an interrupt
		in32(ipi_regs);

		/* Wait a little while to ensure the interrupt gets pending */
		delay = mfspr(SPR_TBL);
		while (mfspr(SPR_TBL) - delay < 100);

		if (index > i)
			printf("FAIL: index %u i %u\n", index, i);

		t1 = mfspr(SPR_TBL);
		isync();
		write_ctpr(0);

		while (t[i] == 0);
		printf("Time=%lu\n",
		       tb_to_nsec(tb_freq,t[i] - t1));

		write_ctpr(15);
	}
	write_ctpr(0);
	mpic_write(MPIC_IPIVPR0, 0);

	printf("CTPR test (lower CTPR on pending IRQ):\n");
	printf("Time is time to raise CTPR.\n");
	index = 0;

	disable_int();
	mpic_write(MPIC_IPIVPR0, 0xF0000);
	count = 5;
	while (--count) {
		unsigned long delay;
		int fail = 0;

		i = index;
		sync();
		t3 = 0;
		t[i] = 0;
		out32(ipi_regs, 1); // Trigger an interrupt
		in32(ipi_regs);

		/* Wait a little while to ensure the interrupt gets pending */
		delay = mfspr(SPR_TBL);
		while (mfspr(SPR_TBL) - delay < 100);

		if (index > i) {
			printf("FAIL MSR[EE]: index %u i %u\n", index, i);
			fail = 1;
		}

		t1 = mfspr(SPR_TBL);
		isync();
		write_ctpr(15);
		t3 = mfspr(SPR_TBL);

		/* Make sure the CTPR write has fully taken effect --
		 * not needed on KVM, but maybe on hardware.
		 */
		read_ctpr();

		delay = mfspr(SPR_TBL);
		while (mfspr(SPR_TBL) - delay < 100);

		enable_int();

		if (!fail && index > i)
			printf("FAIL CTPR: index %u i %u\n", index, i);

		write_ctpr(0);

		while (t[i] == 0);
		printf("Time=%lu\n",
		       tb_to_nsec(tb_freq, t3 - t1));

		disable_int();
	}
	mpic_write(MPIC_IPIVPR0, 0);
}

static void core_init(void)
{
	cpu->coreid = mfspr(SPR_PIR);

	/* set up a TLB entry for CCSR space */
	tlb1_init();
}
