
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
#include <libfdt.h>
#include <libos/io.h>
#include <libos/chardev.h>
#include <libos/console.h>
#include <malloc.h>

extern uint8_t init_stack_top;

cpu_t cpu0 = {
	.kstack = &init_stack_top - FRAMELEN,
	.client = 0,
};

void *fdt;

#define PAGE_SIZE 4096

#define MAX_DT_PATH 256

#define MAX_ADDR_CELLS 4
#define MAX_SIZE_CELLS 2
#define MAX_INT_CELLS 4

#define CELL_SIZE 4

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

void init(unsigned long devtree_ptr)
{
	int dtmap_tsize = TLB_TSIZE_4M;
	unsigned long dtmap_size = tsize_to_pages(dtmap_tsize) * 4096;
	unsigned long dtmap_base = 0x80000000;
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
			printlog(LOGTYPE_DEV, LOGLEVEL_ERROR,
			         "Unrecognized stdout compatible.\n");
			stdout = NULL;
		}

		if (stdout)
			console_init(stdout);
		else
			/* The message will at least go to the log buffer... */
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR,
			         "Failed to initialize stdout.\n");
	} else {
		printlog(LOGTYPE_MISC, LOGLEVEL_ERROR,
		         "No stdout found.\n");
	}
}

void libos_client_entry(unsigned long devtree_ptr)
{
	init(devtree_ptr);

	printf("Hello World\n");
}

static void core_init(void)
{
	cpu->coreid = mfspr(SPR_PIR);

	/* set up a TLB entry for CCSR space */
	tlb1_init();
}

void (*secondary_startp)(void) = NULL;

void secondary_init(void)
{
	core_init();
	if (secondary_startp) {
		secondary_startp();
	}
}
