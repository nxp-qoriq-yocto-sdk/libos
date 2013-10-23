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

/** @file
 * CPU capability querying
 */

#ifndef LIBOS_CPU_CAPS_H
#define LIBOS_CPU_CAPS_H

/// MMUv2 support
#define CPU_FTR_MMUV2		(1 << 0)
/// L2 cache is local to the core
#define CPU_FTR_L2_CORE_LOCAL	(1 << 1)
/// Hardware threads
#define CPU_FTR_THREADS		(1 << 2)
/// TLB0 supports hardware entry select
#define CPU_FTR_TLB0_HES	(1 << 3)
/// TLB1 supports indirect entries
#define CPU_FTR_TLB1_IND	(1 << 4)
/// LRAT support
#define CPU_FTR_LRAT		(1 << 5)
/// Altivec support
#define CPU_FTR_ALTIVEC     (1 << 6)
/// core specific power management features (PWRMGTCR0 SPR)
#define CPU_FTR_PWRMGTCR0	(1 << 7)

#if !defined(_ASM)

/// Structure containing cpu capabilities
typedef struct {
	/// number of entries in TLB0
	unsigned int	tlb0_nentries;
	/// associativity of TLB0
	unsigned int	tlb0_assoc;
	/// number of entries in TLB1
	unsigned int 	tlb1_nentries;
	/// associativity of TLB1
	unsigned int	tlb1_assoc;
	/// bitmask with valid TLB entry sizes
	uint32_t	valid_tsizes;
	/// number of entries in LRAT
	unsigned int	lrat_nentries;
	/// L1 cache size in bytes
	unsigned int	l1_size;
	/// L1 block size in bytes
	unsigned int	l1_blocksize;
	/// L1 number of ways
	unsigned int	l1_nways;
	/// L2 cache size in bytes (available only if L2 is local to the core)
	unsigned int	l2_size;
	/// L2 block size in bytes (available only if L2 is local to the core)
	unsigned int	l2_blocksize;
	/// L2 number of ways (available only if L2 is local to the core)
	unsigned int	l2_nways;
	/// Number of supported hardware threads per core
	uint8_t		threads_per_core;
} cpu_caps_t;

/// Globally accessible definition of the cpu capabilities structure
extern cpu_caps_t cpu_caps;

/** Initialize cpu capabilities and features
 *
 * This function is called by the library at startup.
 */
void init_cpu_caps(void);

/** Query for a specific cpu feature
 *
 * @param[in] cpu_ftr cpu feature to query for (one of the CPU_FTR_* defines)
 * @return non-zero if the feature is supported, zero otherwise
 */
static inline int cpu_has_ftr(unsigned int cpu_ftr)
{
	extern uint32_t cpu_ftrs;

	return cpu_ftrs & cpu_ftr;
}

#endif
#endif
