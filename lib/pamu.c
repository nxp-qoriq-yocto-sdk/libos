/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc.
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

#include <libos/pamu.h>
#include <libos/alloc.h>
#include <libos/printlog.h>
#include <libos/io.h>
#include <libos/bitops.h>
#include <libos/errors.h>

#include <limits.h>

static pamu_mmap_regs_t *pamu_regs;

static ppaace_t *ppaact;
static spaace_t *spaact;
static ome_t *omt;
static unsigned long fspi;
static unsigned int max_subwindow_count;

/*The MSB of pamu_enable_ints is used to store the single bit ecc threshold*/
int pamu_hw_init(unsigned long pamu_reg_base, unsigned long pamu_reg_size,
			void  *mem, unsigned long memsize, uint8_t pamu_enable_ints,
			uint32_t threshold)
{
	uintptr_t pamu_offset;
	uint32_t *pc;
	phys_addr_t phys;
	void *ptr = mem;
	uint32_t pics_val = 0;
	uint32_t ecc_val = 0;
	uint32_t reg_val;

	printlog(LOGTYPE_MISC, LOGLEVEL_DEBUG, "Starting PAMU init\n");

	if (!ppaact) {
		ppaact = ptr;
		ptr += align(PAACT_SIZE + 1, PAMU_TABLE_ALIGNMENT);
		spaact = ptr;
		ptr += align(SPAACT_SIZE + 1, PAMU_TABLE_ALIGNMENT);
		omt = ptr;

		if (ptr + OMT_SIZE > mem + memsize) {
			printlog(LOGTYPE_PAMU, LOGLEVEL_ERROR,
					"PAMU tables don't fit in the allocted memory\n");
			return ERR_RANGE;
		}
	}

	pamu_offset = CCSRBAR_VA + pamu_reg_base;
	pc = (uint32_t *) (pamu_offset + PAMU_PC);
	pamu_regs = (pamu_mmap_regs_t *) (pamu_offset + PAMU_MMAP_REGS_BASE);

	/* Check the version of the PAMU */
	reg_val = in32((uint32_t *)(pamu_offset + PAMU_PC3));
	max_subwindow_count = 1 << (1 + PAMU_PC3_MWCE(reg_val));
	/* FIXME: simics returns MWCE=3 even for Rev 2 so read PVR to clarify */
	reg_val = mfspr(SPR_PVR) & 0xfffffff0;
	/* FIXME: P4080RM r.H says PVR=0xnnnnn1nn, but is 0 for sim&HW rev1&2 */
	if ((reg_val >> 12) == 0x00080230 && (reg_val & 0xf0) != 0x10 &&
	    max_subwindow_count == 16)
		max_subwindow_count = 256;

	/* set up pointers to corenet control blocks */

	phys = virt_to_phys(ppaact);
	out32(&pamu_regs->ppbah, phys >> 32);
	out32(&pamu_regs->ppbal, (uint32_t)phys);
	phys = virt_to_phys(ppaact + PAACE_NUMBER_ENTRIES);
	out32(&pamu_regs->pplah, phys >> 32);
	out32(&pamu_regs->pplal, (uint32_t)phys);

	phys = virt_to_phys(spaact);
	out32(&pamu_regs->spbah, phys >> 32);
	out32(&pamu_regs->spbal, (uint32_t)phys);
	phys = virt_to_phys(spaact + SPAACE_NUMBER_ENTRIES);
	out32(&pamu_regs->splah, phys >> 32);
	out32(&pamu_regs->splal, (uint32_t)phys);

	phys = virt_to_phys(omt);
	out32(&pamu_regs->obah, phys >> 32);
	out32(&pamu_regs->obal, (uint32_t)phys);
	phys = virt_to_phys(omt + OME_NUMBER_ENTRIES);
	out32(&pamu_regs->olah, phys >> 32);
	out32(&pamu_regs->olal, (uint32_t)phys);

	/*
	 * set PAMU enable bit,
	 * plus allow ppaact and spaact to be cached
	 * & enable PAMU access violation interrupts.
	 */
	if (pamu_enable_ints & (1 << pamu_int_av))
		pics_val |= PAMU_ACCESS_VIOLATION_ENABLE;
	if (pamu_enable_ints & (1 << pamu_int_operation))
		pics_val |= PAMU_OPERATION_ERROR_INT_ENABLE;
	if (pamu_enable_ints & (1 << pamu_int_singlebit)) {
		ecc_val |= PAMU_SB_ECC_ERR;
		out32((uint32_t *)(pamu_offset + PAMU_EECTL),
			threshold << PAMU_EECTL_THR_SHIFT);
	}
	if (pamu_enable_ints & (1 << pamu_int_multibit))
		ecc_val |= PAMU_MB_ECC_ERR;

	out32((uint32_t *)(pamu_offset + PAMU_PICS), pics_val);
	out32((uint32_t *)(pamu_offset + PAMU_EEINTEN), ecc_val);
	out32((uint32_t *)(pamu_offset + PAMU_EEDIS), ~ecc_val & PAMU_ECC_ERR_MASK);

	out32(pc, PAMU_PC_PE | PAMU_PC_OCE | PAMU_PC_SPCC | PAMU_PC_PPCC);
	return 0;
}

ppaace_t *pamu_get_ppaace(uint32_t liodn)
{
	if (!ppaact)
		return NULL;

	return &ppaact[liodn];
}

ome_t *pamu_get_ome(uint8_t omi)
{
	if (omt)
		return &omt[omi];
	return NULL;
}

void setup_default_xfer_to_host_ppaace(ppaace_t *ppaace)
{
	ppaace->pt = PAACE_PT_PRIMARY;
	ppaace->domain_attr.to_host.coherency_required = PAACE_M_COHERENCE_REQ;
}

void setup_default_xfer_to_host_spaace(spaace_t *spaace)
{
	spaace->pt = PAACE_PT_SECONDARY;
	spaace->domain_attr.to_host.coherency_required = PAACE_M_COHERENCE_REQ;
}

spaace_t *pamu_get_spaace(unsigned long fspi_index, uint32_t wnum)
{
	return &spaact[fspi_index + wnum];
}

unsigned long get_fspi_and_increment(uint32_t subwindow_cnt)
{
	unsigned long tmp;

	do {
		tmp = fspi;
		/*
		 * This check should be MP-safe as atomic compare_and_swap()
		 * will ensure that we re-iterate here if "fspi" gets updated.
		 */
		if ((tmp + subwindow_cnt) > SPAACE_NUMBER_ENTRIES)
			return ULONG_MAX;
	} while (!compare_and_swap(&fspi, tmp, tmp + subwindow_cnt));

	return tmp;
}

unsigned int pamu_get_max_subwindow_count(void)
{
	return max_subwindow_count;
}
