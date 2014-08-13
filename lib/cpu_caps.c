/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#include <libos/percpu.h>
#include <libos/io.h>
#include <libos/fsl-booke-tlb.h>
#include <libos/mp.h>
#include <libos/cpu_caps.h>

uint32_t cpu_ftrs;
cpu_caps_t cpu_caps;

static struct {
	uint32_t pvr;
	uint32_t pvr_mask;
	uint32_t ftrs;
} pvr_to_ftrs[] = {
	{	/* e500mc */
		0x80230000,
		0xffff0000,
		CPU_FTR_L2_CORE_LOCAL
	},
	{	/* e5500 */
		0x80240000,
		0xffff0000,
		CPU_FTR_L2_CORE_LOCAL
	},
	{	/* e6500 rev1 */
		0x80400010,
		0xffffffff,
		CPU_FTR_MMUV2 | CPU_FTR_THREADS | CPU_FTR_TLB0_HES |
			CPU_FTR_TLB1_IND | CPU_FTR_LRAT | CPU_FTR_ALTIVEC
	},
	{	/* e6500 rev2 */
		0x80400020,
		0xffff00ff,
		CPU_FTR_MMUV2 | CPU_FTR_THREADS | CPU_FTR_TLB0_HES |
			CPU_FTR_TLB1_IND | CPU_FTR_LRAT | CPU_FTR_ALTIVEC |
			CPU_FTR_PWRMGTCR0
	},
	{	/* Default generic core */
		0,
		0
	}
};

void init_cpu_caps(void)
{
	int i;
	register_t spr;

	spr = mfspr(SPR_PVR);
	for (i = 0; pvr_to_ftrs[i].pvr; i++) {
		if ((spr & pvr_to_ftrs[i].pvr_mask) == pvr_to_ftrs[i].pvr) {
			cpu_ftrs = pvr_to_ftrs[i].ftrs;
			break;
		}
	}

	/* if no pvr matched fallback to default features */
	if (!pvr_to_ftrs[i].pvr)
		cpu_ftrs = pvr_to_ftrs[i].ftrs;

	spr = mfspr(SPR_L1CFG0);
	cpu_caps.l1_size = (spr & L1CFG0_CSIZE) * 1024;
	cpu_caps.l1_blocksize = 32 << ((spr & L1CFG0_CBSIZE) >> L1CFG0_CBSIZE_SHIFT);
	cpu_caps.l1_nways = 1 + ((spr & L1CFG0_CNWAY) >> L1CFG0_CNWAY_SHIFT);
	if (cpu_has_ftr(CPU_FTR_L2_CORE_LOCAL)) {
		spr = mfspr(SPR_L2CFG0);
		cpu_caps.l2_size = (spr & L2CFG0_CSIZE) * 65536;
		cpu_caps.l2_blocksize = 32 << ((spr & L2CFG0_CBSIZE) >> L2CFG0_CBSIZE_SHIFT);
		cpu_caps.l2_nways = 1 + ((spr & L2CFG0_CNWAY) >> L2CFG0_CNWAY_SHIFT);
	}

	cpu_caps.threads_per_core = 1;
#if CONFIG_LIBOS_MAX_HW_THREADS > 1
	if (cpu_has_ftr(CPU_FTR_THREADS))
		cpu_caps.threads_per_core = mftmr(TMR_TMCFG0) & TMCFG0_NTHRD;
#endif

	if (cpu_has_ftr(CPU_FTR_LRAT))
		cpu_caps.lrat_nentries = mfspr(SPR_LRATCFG) & LRATCFG_NENTRY_MASK;
	if (cpu_has_ftr(CPU_FTR_MMUV2)) {
		cpu_caps.valid_tsizes = mfspr(SPR_TLB1PS);
	} else {
		int min_tsize, max_tsize;

		/*
		 * On cores with MMUv1 that don't have TLB1PS and support only
		 * power of 4KB TLB entry sizes, start wth a synthetic mask
		 * with all the possible TSIZEs and then mask out the unsupported
		 * ones as per TLB1CFG[MIN_TSIZE] and TLB1CFG[MAX_TSIZE].
		 */
		cpu_caps.valid_tsizes = 0x55555555;

		spr = mfspr(SPR_TLB1CFG);
		min_tsize = ((spr & TLBCFG_MINSIZE_MASK) >> TLBCFG_MINSIZE_SHIFT) * 2;
		max_tsize = ((spr & TLBCFG_MAXSIZE_MASK) >> TLBCFG_MAXSIZE_SHIFT) * 2;
		cpu_caps.valid_tsizes &= ~((1 << min_tsize) - 1);
		cpu_caps.valid_tsizes &= (2 << max_tsize) - 1;
	}
	spr = mfspr(SPR_TLB0CFG);
	cpu_caps.tlb0_nentries = spr & TLBCFG_NENTRY_MASK;
	cpu_caps.tlb0_assoc = (spr & TLBCFG_ASSOC_MASK) >> TLBCFG_ASSOC_SHIFT;
	spr = mfspr(SPR_TLB1CFG);
	cpu_caps.tlb1_nentries = spr & TLBCFG_NENTRY_MASK;
	cpu_caps.tlb1_assoc = (spr & TLBCFG_ASSOC_MASK) >> TLBCFG_ASSOC_SHIFT;
}
