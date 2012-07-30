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
 */

#ifndef LIBOS_FSL_BOOKE_TLB_H
#define LIBOS_FSL_BOOKE_TLB_H

#define SPR_MAS0         624  // MMU Assist Register 0
#define   MAS0_TLBSEL(x)     ((x << 28) & 0x10000000)
#define   MAS0_ESEL(x)       ((x << 16) & 0x003F0000)
#define   MAS0_GET_TLBSEL(x) ((x >> 28) & 3)
#define   MAS0_GET_TLB0ESEL(x)((x >> 16) & 3)
#define   MAS0_GET_TLB1ESEL(x)((x >> 16) & 63)
#define   MAS0_TLBSEL1       0x10000000
#define   MAS0_TLBSEL0       0x00000000
#define   MAS0_LRATSEL       0x80000000
#define   MAS0_TLBSEL_MASK   0x30000000
#define   MAS0_ESEL_TLB1MASK 0x003F0000
#define   MAS0_ESEL_TLB0MASK 0x00030000
#define   MAS0_ESEL_MASK     0x0fff0000
#define   MAS0_ESEL_SHIFT    16
#define   MAS0_NV_MASK       0x00000003
#define   MAS0_NV_SHIFT      0
#define   MAS0_RESERVED      0xe000f000

#define SPR_MAS1         625  // MMU Assist Register 1
#define   MAS1_VALID         0x80000000
#define   MAS1_IPROT         0x40000000
#define   MAS1_IPROT_SHIFT   30
#define   MAS1_TID_MASK      0x00FF0000
#define   MAS1_TID_SHIFT     16
#define   MAS1_IND           0x00002000
#define   MAS1_IND_SHIFT     13
#define   MAS1_TS            0x00001000
#define   MAS1_TS_SHIFT      12
#define   MAS1_TSIZE_MASK    0x00000F80
#define   MAS1_TSIZE_SHIFT   7
#define   MAS1_GETTID(mas1)  (((mas1) & MAS1_TID_MASK) >> MAS1_TID_SHIFT)
#define   MAS1_GETTSIZE(mas1)(((mas1) & MAS1_TSIZE_MASK) >> MAS1_TSIZE_SHIFT)
#define   MAS1_RESERVED      0x0000C07f

#define SPR_MAS2         626  // MMU Assist Register 2
#define   MAS2_EPN           (~0xfffUL)
#define   MAS2_EPN_SHIFT     12
#define   MAS2_X0            0x00000040
#define   MAS2_X1            0x00000020
#define   MAS2_W             0x00000010
#define   MAS2_I             0x00000008
#define   MAS2_M             0x00000004
#define   MAS2_G             0x00000002
#define   MAS2_E             0x00000001
#define   MAS2_TLB0_ENTRY_IDX_MASK  0x0007f000
#define   MAS2_TLB0_ENTRY_IDX_SHIFT 12
#define   MAS2_RESERVED      0x00000f00
#define   MAS2_FLAGS         0x000000ff

#define SPR_MAS3         627  // MMU Assist Register 3
#define   MAS3_RPN           0xFFFFF000
#define   MAS3_RPN_SHIFT     12
#define   MAS3_USER          0x000003c0
#define   MAS3_U0            0x00000200
#define   MAS3_U1            0x00000100
#define   MAS3_U2            0x00000080
#define   MAS3_U3            0x00000040
#define   MAS3_UX            0x00000020
#define   MAS3_SX            0x00000010
#define   MAS3_UW            0x00000008
#define   MAS3_SW            0x00000004
#define   MAS3_UR            0x00000002
#define   MAS3_SR            0x00000001
#define   MAS3_RESERVED      0x00000c00
#define   MAS3_FLAGS         0x0000003f
#define   MAS3_SPSIZE_MASK   0x0000003e
#define   MAS3_SPSIZE_SHIFT  1
#define   MAS3_GETSPSIZE(mas3) (((mas3) & MAS3_SPSIZE_MASK) >> MAS3_SPSIZE_SHIFT)

#define SPR_MAS4         628  // MMU Assist Register 4
#define   MAS4_TLBSELD1      0x10000000
#define   MAS4_TLBSELD0      0x00000000
#define   MAS4_TSIZED_MASK   0x00000F00
#define   MAS4_TSIZED_SHIFT  8
#define   MAS4_X0D           0x00000040
#define   MAS4_X1D           0x00000020
#define   MAS4_WD            0x00000010
#define   MAS4_ID            0x00000008
#define   MAS4_MD            0x00000004
#define   MAS4_GD            0x00000002
#define   MAS4_ED            0x00000001

#define SPR_MAS5         339  // MMU Assist Register 5
#define   MAS5_SGS           0x80000000 // Search GS
#define   MAS5_SLPID         0x000000ff // Search LPID

#define SPR_MAS6         630  // MMU Assist Register 6
#define   MAS6_SPID_MASK     0x00FF0000
#define   MAS6_SPID_SHIFT    16
#define   MAS6_SAS           0x00000001
#define   MAS6_SIND          0x00000002
#define   MAS6_SIND_SHIFT    1

#define SPR_MAS7         944  // MMU Assist Register 7
#define   MAS7_RPN           0x0000000f
#define   MAS7_RESERVED      0xfffffff0

#define SPR_MAS8         341  // MMU Assist Register 8
#define   MAS8_GTS           0x80000000 /* Guest space */
#define   MAS8_VF            0x40000000 /* Virtualization Fault */
#define   MAS8_VF_SHIFT      30
#define   MAS8_TLPID         0x000000ff

#define TLB_TSIZE_4K   2
#define TLB_TSIZE_8K   3
#define TLB_TSIZE_16K  4
#define TLB_TSIZE_32K  5
#define TLB_TSIZE_64K  6
#define TLB_TSIZE_128K 7
#define TLB_TSIZE_256K 8
#define TLB_TSIZE_512K 9
#define TLB_TSIZE_1M   10
#define TLB_TSIZE_2M   11
#define TLB_TSIZE_4M   12
#define TLB_TSIZE_8M   13
#define TLB_TSIZE_16M  14
#define TLB_TSIZE_32M  15
#define TLB_TSIZE_64M  16
#define TLB_TSIZE_128M 17
#define TLB_TSIZE_256M 18
#define TLB_TSIZE_512M 19
#define TLB_TSIZE_1G   20
#define TLB_TSIZE_2G   21
#define TLB_TSIZE_4G   22
#define TLB_TSIZE_8G   23
#define TLB_TSIZE_16G  24
#define TLB_TSIZE_32G  25
#define TLB_TSIZE_64G  26
#define TLB_TSIZE_128G 27
#define TLB_TSIZE_256G 28
#define TLB_TSIZE_512G 29
#define TLB_TSIZE_1T   30

#define TLB_MAS2_IO    (MAS2_I | MAS2_G)
#define TLB_MAS2_MEM   (MAS2_M)

#define TLB_MAS3_KERN  (MAS3_SR | MAS3_SW | MAS3_SX)
#define TLB_MAS3_KDATA (MAS3_SR | MAS3_SW)

#define TLB_MAS8_HV    0
#define TLB_MAS8_GUEST MAS8_GTS

#define TLBIVAX_VA       (~0xfffUL)
#define TLBIVAX_RESERVED 0xff3
#define TLBIVAX_INV_ALL  0x004
#define TLBIVAX_TLB_NUM  0x018
#define TLBIVAX_TLB0     0x000
#define TLBIVAX_TLB1     0x008
#define TLBIVAX_TLB_NUM_SHIFT 3

/* page table entry */
#define HPTE_ARPN       (~0xffffffULL)
#define HPTE_ARPN_SHIFT 24
#define HPTE_WIMGE_W    0x00800000
#define HPTE_WIMGE_I    0x00400000
#define HPTE_WIMGE_M    0x00200000
#define HPTE_WIMGE_G    0x00100000
#define HPTE_WIMGE_E    0x00080000
#define HPTE_R          0x00040000
#define HPTE_U0         0x00020000
#define HPTE_U1         0x00010000
#define HPTE_SW0        0x00002000
#define HPTE_C          0x00001000
#define HPTE_PS         0x00000f00
#define HPTE_PS_SHIFT   8
#define HPTE_BAP_SR     0x00000004
#define HPTE_BAP_UR     0x00000008
#define HPTE_BAP_SW     0x00000010
#define HPTE_BAP_UW     0x00000020
#define HPTE_BAP_SX     0x00000040
#define HPTE_BAP_UX     0x00000080
#define HPTE_BAP_SW1    0x00000002
#define HPTE_VALID      0x00000001

#if !defined(_ASM)

#include <libos/libos.h>
#include <libos/bitops.h>

typedef struct tlb_entry {
	register_t mas0;
	register_t mas1;
	register_t mas2;
	register_t mas3;
	register_t mas7;
	register_t mas8;
} tlb_entry_t;

void tlb1_set_entry(unsigned int idx, unsigned long va, phys_addr_t pa,
                    register_t size, register_t mas2flags, register_t mas3flags,
                    unsigned int _tid, unsigned int _ts, register_t mas8,
                    unsigned int indirect);
void tlb1_clear_entry(unsigned int idx);
void tlb1_write_entry(unsigned int idx);

extern const uint32_t valid_tsize_mask;

static inline unsigned int pages_to_tsize_msb(unsigned long epn)
{
	return epn != 0 ? ilog2(epn) + 2 : 0;
}

static inline unsigned int pages_to_tsize_lsb(unsigned long epn)
{
	return epn != 0 ? count_lsb_zeroes(epn) + 2 : 0;
}

// Returns a maximum *valid* TSIZE, equal or smaller than the
// (potentially invalid on the core running on) input TSIZE.
// Note that the function expects a sane TSIZE value, between
// the minimum and maximum possible values (4K min, 1T max)
static inline int max_valid_tsize(unsigned int tsize)
{
	assert(tsize >= TLB_TSIZE_4K && tsize <= 31);

	return tsize - count_msb_zeroes_32(valid_tsize_mask << (31 - tsize));
}

static inline int natural_alignment(unsigned long epn)
{
	return epn != 0 ?
	       max_valid_tsize(pages_to_tsize_lsb(epn)) :
	       TLB_TSIZE_4G;
}

static inline unsigned long tsize_to_pages(unsigned int tsize)
{
	return tsize != 0 ? 1UL << (tsize - 2) : 0;
}

// return 1 page for 1K and 2K sizes. 1K and 2K are supported sizes
// for page table in case of indirect entries
static inline unsigned long tsize_to_pages_roundup(unsigned int tsize)
{
	return tsize > 1 ? 1UL << (tsize - 2) : 1;
}

// Return the tsize of the largest page size that can be used
// to map the specified range (in pages).
static inline int max_page_size(unsigned long start,
				unsigned long num)
{
	return min(natural_alignment(start),
		   max_valid_tsize(pages_to_tsize_msb(num)));
}

// Return the tsize of the largest page size that can be used
// to map the specified range (in tsize).
static inline int max_page_tsize(unsigned long start,
				 unsigned int tsize)
{
	return min(natural_alignment(start), max_valid_tsize(tsize));
}

#endif
#endif
