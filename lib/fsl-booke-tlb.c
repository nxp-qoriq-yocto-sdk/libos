/*-
 * Copyright (C) 2006 Semihalf, Marian Balakowicz <m8@semihalf.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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
 *
 * Some hw specific parts of this pmap were derived or influenced
 * by NetBSD's ibm4xx pmap module. More generic code is shared with
 * a few other pmap modules from the FreeBSD tree.
 */

#include <libos/fsl-booke-tlb.h>
#include <libos/core-regs.h>
#include <libos/percpu.h>
#include <libos/bitops.h>
#include <libos/printlog.h>

/*
 * Setup entry in a sw tlb1 table, write entry to TLB1 hardware.
 * This routine is used for low level operations on the TLB1,
 * for creating temporaray as well as permanent mappings (tlb_set_entry).
 *
 * We assume kernel mappings only, thus all entries created have supervisor
 * permission bits set nad user permission bits cleared.
 *
 * Provided mapping size must be a power of 4.
 * mas2flags must be a combination of MAS2_[WIMGE].
 * Entry TID is set to _tid which must not exceed 8 bit value.
 * Entry TS is set to either 0 or MAS1_TS based on provided _ts.
 */
void tlb1_set_entry(unsigned int idx, unsigned long va, phys_addr_t pa,
                    register_t tsize, register_t mas2flags, register_t mas3flags,
                    unsigned int _tid, unsigned int _ts, register_t mas8,
                    unsigned int indirect)
{
	register_t ts, tid;

	assert((1 << tsize) & valid_tsize_mask);

	tid = (_tid <<  MAS1_TID_SHIFT) & MAS1_TID_MASK;
	ts = (_ts) ? MAS1_TS : 0;
	cpu->tlb1[idx].mas1 = MAS1_VALID | MAS1_IPROT | ts | tid;
	cpu->tlb1[idx].mas1 |= ((tsize << MAS1_TSIZE_SHIFT) & MAS1_TSIZE_MASK);
	cpu->tlb1[idx].mas1 |= (indirect << MAS1_IND_SHIFT) & MAS1_IND;

	cpu->tlb1[idx].mas2 = (va & MAS2_EPN) | mas2flags;

	/* Set supervisor rwx permission bits */
	cpu->tlb1[idx].mas3 = (pa & MAS3_RPN) | mas3flags;

	cpu->tlb1[idx].mas7 = pa >> 32;
	cpu->tlb1[idx].mas8 = mas8;

	tlb1_write_entry(idx);
}

void tlb1_clear_entry(unsigned int idx)
{
	cpu->tlb1[idx].mas1 = 0;
	cpu->tlb1[idx].mas2 = 0;
	cpu->tlb1[idx].mas3 = 0;
	cpu->tlb1[idx].mas7 = 0;
	cpu->tlb1[idx].mas8 = 0;
	tlb1_write_entry(idx);
}

void tlb1_write_entry(unsigned int idx)
{
	register_t mas0;

	/* Select entry */
	mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(idx);

	mtspr(SPR_MAS0, mas0);
	mtspr(SPR_MAS1, cpu->tlb1[idx].mas1);
	mtspr(SPR_MAS2, cpu->tlb1[idx].mas2);
	mtspr(SPR_MAS3, cpu->tlb1[idx].mas3);
	mtspr(SPR_MAS7, cpu->tlb1[idx].mas7);
#ifdef HYPERVISOR
	mtspr(SPR_MAS8, cpu->tlb1[idx].mas8);
#endif
	asm volatile("isync; tlbwe; isync; msync" : : : "memory");
}
