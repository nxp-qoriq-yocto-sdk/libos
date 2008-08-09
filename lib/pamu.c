
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

#include <libos/pamu.h>
#include <libos/console.h>
#include <libos/io.h>

pamu_mmap_regs_t *pamu_regs;

int pamu_hw_init(unsigned long pamu_reg_base, unsigned long pamu_reg_size)
{
    	uint32_t pamu_offset;
    	uintptr_t *pc;	
	uint32_t table_size;
	static uintptr_t ppaact_pointer, ppaact_pointer_end;
	static uintptr_t spaact_pointer, spaact_pointer_end;
	static uintptr_t omt_pointer, omt_pointer_end;

	printlog(LOGTYPE_MISC, LOGLEVEL_DEBUG, "Starting PAMU init\n");

	if (!ppaact_pointer) {
		table_size = sizeof(ppaace_t) * PAACE_NUMBER_ENTRIES;
		ppaact_pointer = (uintptr_t)
			 alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!ppaact_pointer) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU ppaact.\n");
			return -1;
		}
		ppaact_pointer_end = ppaact_pointer + table_size;

		table_size = sizeof(spaace_t) * SPAACE_NUMBER_ENTRIES;
		spaact_pointer = (uintptr_t)
			alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!spaact_pointer) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU spaact.\n");
			return -1;
		}
		spaact_pointer_end = spaact_pointer + table_size;

		table_size = sizeof(ome_t) * OME_NUMBER_ENTRIES;
		omt_pointer = (uintptr_t)
			alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!omt_pointer) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU omt.\n");
			return -1;
		}
		omt_pointer_end = omt_pointer + table_size;
	}

	pamu_offset = CCSRBAR_VA + pamu_reg_base;
	pc = (uintptr_t *) (pamu_offset + PAMU_PC);
	pamu_regs = (pamu_mmap_regs_t *) (pamu_offset + PAMU_MMAP_REGS_BASE);

	/* 
	 * Clear the Gate bit, and 
	 * set PAMU enable bit, 
	 * plus allow ppaact and spaact to be cached 
	 */

    	out32((uint32_t *)pc, PAMU_PC_PE | PAMU_PC_SPCC | PAMU_PC_PPCC);

	/* 
	 * set up pointers to corenet control blocks 
	 * since we are currently only 32 bit, set the high
	 * end addresses to zero
	 */

	out32(&pamu_regs->ppbah, 0);
    	out32(&pamu_regs->pplah, 0);
    	out32(&pamu_regs->spbah, 0);
    	out32(&pamu_regs->splah, 0);
	out32(&pamu_regs->obah, 0);
	out32(&pamu_regs->olah, 0);

	// FIXME: this assumes that hv is running at phys addr 0x0
	// which may not be the case in the future
	out32(&pamu_regs->ppbal, ppaact_pointer-PHYSBASE);
	out32(&pamu_regs->pplal, ppaact_pointer_end-PHYSBASE);
	out32(&pamu_regs->spbal, spaact_pointer-PHYSBASE);
	out32(&pamu_regs->splal, spaact_pointer_end-PHYSBASE);
	out32(&pamu_regs->obal, omt_pointer-PHYSBASE);
	out32(&pamu_regs->olal, omt_pointer_end-PHYSBASE);

	return 0;
}

ppaace_t *get_ppaace(uint32_t liodn)
{
	ppaace_t *table_head;

	if (!pamu_regs)
		return NULL;
	
	table_head = (ppaace_t *) in32(&pamu_regs->ppbal);
	if (!table_head)
		return NULL;

	table_head = (ppaace_t *) ((unsigned char *)table_head + PHYSBASE);
		
	return table_head + liodn;
}

void setup_default_xfer_to_host_ppaace(ppaace_t *ppaace) 
{
	ppaace->wbah = 0;
	ppaace->ap = PAACE_AP_PERMS_ALL;
	ppaace->dd = PAACE_DD_TO_HOST;
	ppaace->pt = PAACE_PT_PRIMARY;
	ppaace->v  = PAACE_V_VALID;
	ppaace->atm = PAACE_ATM_NO_XLATE;
	ppaace->otm = PAACE_OTM_NO_XLATE;

	/* this is a guess - interleaved memory complex 1-4 */
	ppaace->domain_attr.to_host.did = PAACE_DID_MEM_1_4;

	/* Aquila platform does not support PID */
	ppaace->domain_attr.to_host.pid = PAACE_PID_0;	

	ppaace->domain_attr.to_host.coherency_required = PAACE_M_COHERENCE_REQ;
}
