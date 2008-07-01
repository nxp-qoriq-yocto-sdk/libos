
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

uintptr_t pp;
uintptr_t ppl;
uintptr_t sp;
uintptr_t spl;
uintptr_t ob;
uintptr_t obl;
pamu_mmap_regs_t *pamu_regs;

void pamu_hw_init(unsigned long pamu_reg_base, unsigned long pamu_reg_size)
{
    	uint32_t pamu_offset;
    	uintptr_t *pc;	
	uint32_t table_size;

	printlog(LOGTYPE_MISC, LOGLEVEL_DEBUG, "Starting PAMU init\n");

	if (!pp) {
		table_size = sizeof(ppaace_t) * PAACE_NUMBER_ENTRIES;
		pp = (uintptr_t)alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!pp) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU ppaact.\n");
			return;
		}
   		ppl = pp + table_size;

		table_size = sizeof(spaace_t) * SPAACE_NUMBER_ENTRIES;
    		sp = (uintptr_t) alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!sp) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU spaact.\n");
			return;
		}
   		spl = sp + table_size;

		table_size = sizeof(ome_t) * OME_NUMBER_ENTRIES;
		ob = (uintptr_t) alloc(table_size, PAMU_TABLE_ALIGNMENT);
		if (!ob) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU omt.\n");
			return;
		}
		obl = ob + table_size;
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

	out32(&pamu_regs->ppbal, pp-PHYSBASE);
	out32(&pamu_regs->pplal, ppl-PHYSBASE);
	out32(&pamu_regs->spbal, sp-PHYSBASE);
	out32(&pamu_regs->splal, spl-PHYSBASE);
	out32(&pamu_regs->obal, ob-PHYSBASE);
	out32(&pamu_regs->olal, obl-PHYSBASE);
}

ppaace_t *get_ppaace(uint32_t liodn)
{
	ppaace_t *table_head;
	
	table_head = (ppaace_t *) in32(&pamu_regs->ppbal);
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
