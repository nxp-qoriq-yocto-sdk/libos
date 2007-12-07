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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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
#include <libos/spr.h>
#include <libos/percpu.h>
#include <libos/bitops.h>

/*
 *    after tlb1_init:
 *        TLB1[0] = CCSR
 *        TLB1[1] = hv image 16M
 *
 *
 */

/* hardcoded hack for now */
#define CCSRBAR_PA              0xfe000000
#define CCSRBAR_VA              0x01000000
#define CCSRBAR_SIZE            TLB_TSIZE_16M

static int print_ok;

void tlb1_init(void)
{
	tlb1_set_entry(62, CCSRBAR_VA, CCSRBAR_PA, CCSRBAR_SIZE, TLB_MAS2_IO,
	               TLB_MAS3_KERN, 0, 0, TLB_MAS8_HV);
	print_ok = 1;
}

/*
 * Setup entry in a sw tlb1 table, write entry to TLB1 hardware.
 * This routine is used for low level operations on the TLB1,
 * for creating temporaray as well as permanent mappings (tlb_set_entry).
 *
 * We assume kernel mappings only, thus all entries created have supervisor
 * permission bits set nad user permission bits cleared.
 *
 * Provided mapping size must be a power of 4.
 * Mapping flags must be a combination of MAS2_[WIMG].
 * Entry TID is set to _tid which must not exceed 8 bit value.
 * Entry TS is set to either 0 or MAS1_TS based on provided _ts.
 */
void tlb1_set_entry(unsigned int idx, unsigned long va, physaddr_t pa,
                    uint32_t tsize, uint32_t mas2flags, uint32_t mas3flags,
                    unsigned int _tid, unsigned int _ts,
                    uint32_t mas8)
{
	uint32_t ts, tid;

#if 0
	if (print_ok)
		printf("__tlb1_set_entry: s (idx = %d va = 0x%08lx pa = 0x%08llx "
		       "tsize = 0x%08x mas2flags = 0x%08x mas3flags = 0x%08x "
		       "_tid = %d _ts = %d mas8 = 0x%08x\n",
		       idx, va, pa, tsize, mas2flags, mas3flags, _tid, _ts, mas8);
#endif

	tid = (_tid <<  MAS1_TID_SHIFT) & MAS1_TID_MASK;
	ts = (_ts) ? MAS1_TS : 0;
	cpu->tlb1[idx].mas1 = MAS1_VALID | MAS1_IPROT | ts | tid;
	cpu->tlb1[idx].mas1 |= ((tsize << MAS1_TSIZE_SHIFT) & MAS1_TSIZE_MASK);

	cpu->tlb1[idx].mas2 = (va & MAS2_EPN) | mas2flags;

	/* Set supervisor rwx permission bits */
	cpu->tlb1[idx].mas3 = (pa & MAS3_RPN) | mas3flags;
	 MAS3_SR | MAS3_SW | MAS3_SX;

	cpu->tlb1[idx].mas7 = pa >> 32;
	cpu->tlb1[idx].mas8 = mas8;

#if 0
	if (print_ok)
		printf("__tlb1_set_entry: mas1 = %08x mas2 = %08x mas3 = 0x%08x mas7 = 0x%08x\n",
		       cpu->tlb1[idx].mas1, cpu->tlb1[idx].mas2, cpu->tlb1[idx].mas3,
	   	    cpu->tlb1[idx].mas7);
#endif

	tlb1_write_entry(idx);
	//debugf("__tlb1_set_entry: e\n");
}


void tlb1_write_entry(unsigned int idx)
{
	uint32_t mas0, mas7;

	//debugf("tlb1_write_entry: s\n");

	/* Select entry */
	mas0 = MAS0_TLBSEL(1) | MAS0_ESEL(idx);
	//debugf("tlb1_write_entry: mas0 = 0x%08x\n", mas0);

	mtspr(SPR_MAS0, mas0);
	mtspr(SPR_MAS1, cpu->tlb1[idx].mas1);
	mtspr(SPR_MAS2, cpu->tlb1[idx].mas2);
	mtspr(SPR_MAS3, cpu->tlb1[idx].mas3);
	mtspr(SPR_MAS7, cpu->tlb1[idx].mas7);
	mtspr(SPR_MAS8, cpu->tlb1[idx].mas8);
	asm volatile("isync; tlbwe; isync; msync" : : : "memory");

	//debugf("tlb1_write_entry: e\n");;
}
