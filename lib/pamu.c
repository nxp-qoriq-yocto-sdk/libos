
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
#include <libos/printlog.h>
#include <libos/io.h>

static pamu_mmap_regs_t *pamu_regs;

static ppaace_t *ppaact;
static spaace_t *spaact;
static ome_t *omt;

int pamu_hw_init(unsigned long pamu_reg_base, unsigned long pamu_reg_size)
{
	uintptr_t pamu_offset;
	uint32_t *pc;
	phys_addr_t phys;

	printlog(LOGTYPE_MISC, LOGLEVEL_DEBUG, "Starting PAMU init\n");

	if (!ppaact) {
		ppaact = alloc(sizeof(ppaace_t) * PAACE_NUMBER_ENTRIES,
		               PAMU_TABLE_ALIGNMENT);
		if (!ppaact) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU ppaact.\n");
			return -1;
		}

		spaact = alloc(sizeof(spaace_t) * SPAACE_NUMBER_ENTRIES,
		               PAMU_TABLE_ALIGNMENT);
		if (!spaact) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU spaact.\n");
			return -1;
		}

		omt = alloc(sizeof(ome_t) * OME_NUMBER_ENTRIES,
		            PAMU_TABLE_ALIGNMENT);
		if (!omt) {
			printlog(LOGTYPE_MISC, LOGLEVEL_ERROR, "Unable to allocate space for PAMU omt.\n");
			return -1;
		}
	}

	pamu_offset = CCSRBAR_VA + pamu_reg_base;
	pc = (uint32_t *) (pamu_offset + PAMU_PC);
	pamu_regs = (pamu_mmap_regs_t *) (pamu_offset + PAMU_MMAP_REGS_BASE);

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

	out32((uint32_t *)(pamu_offset + PAMU_PICS), 
			PAMU_ACCESS_VIOLATION_ENABLE);
	out32(pc, PAMU_PC_PE | PAMU_PC_SPCC | PAMU_PC_PPCC);
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
