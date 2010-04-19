

/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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

#include <libos/core-regs.h>
#include <libos/fsl-booke-tlb.h>

#if 1
/* 32-bit */
#define LONGBYTES 4
#define LOAD lwz
#define STORE stw
#else
/* 64-bit */
#define LONGBYTES 8
#define LOAD ld
#define STORE std
#endif

#ifndef PHYSMAPSIZE
#define PHYSMAPSIZE TLB_TSIZE_256M
#endif

.text

/*
 * Assumptions at start are that hardware is in
 * a state as defined by the ePAPR
 *   R3 :  ptr to device tree
 *
 */

.globl _start
_start:

	/* Issue INV_ALL Invalidate on TLB0 */
	li      %r16, 0x04
	tlbivax	0, %r16
	isync
	msync
	nop

/*
 * Use tlbsx to locate the TLB1 entry that maps kernel code
 */
 	lis	%r16, 1f@h
 	ori	%r16, %r16, 1f@l
 	mtlr	%r16
	b	fixup_mmu

#ifndef CONFIG_LIBOS_64BIT
1:	lis	%r2, cpu0@ha
	addi	%r2, %r2, cpu0@l
	mtsprg0	%r2
#else
1:	lis	%r13, cpu0@ha
	addi	%r13, %r13, cpu0@l
	mtsprg0	%r13
	/* Setup %r2 to toc_start + 0x8000 as expected by the 64-bit ABI */
	lis	%r2, (toc_start)@h
	ori	%r2, %r2, (toc_start)@l
	addis	%r2, %r2, 0x8000@ha
	addi	%r2, %r2, 0x8000@l
#endif

	bl	clear_bss

	mfspr	%r16, SPR_L1CFG0
	li	%r17, 32
	lis	%r18, cache_block_size@ha
	rlwinm	%r16, %r16, 9, 3
	slw	%r17, %r17, %r16
	stw	%r17, cache_block_size@l(%r18)

	lis	%r1, (init_stack_top - 16)@ha
	addi	%r1, %r1, (init_stack_top - 16)@l

	bl	ivor_setup
	bl	libos_client_entry

1:	lis	%r3, 0xffff
	mtctr	%r3
2:	bdnz	2b
	b	1b

	.global secondary_start
secondary_start:

 	lis	%r16, 1f@h
 	ori	%r16, %r16, 1f@l
 	mtlr	%r16
	b	fixup_mmu

#ifndef CONFIG_LIBOS_64BIT
1:	mr	%r2, %r3
#else
1:	mr	%r13, %r3
	/* TBD: TOC setup for secondary cores */
#endif
	mtsprg0	%r3
	LOAD	%r1, CPU_KSTACK(%r3)

	bl	ivor_setup
	bl	secondary_init

1:	lis	%r3, 0xffff
	mtctr	%r3
2:	bdnz	2b
	b	1b

fixup_mmu:
	mflr	%r23

	bl	1f
	/* Find entry that maps current address */
1:	mflr	%r19
	mfspr	%r17, SPR_PID
	slwi	%r17, %r17, MAS6_SPID_SHIFT
	mtspr	SPR_MAS6, %r17
	isync
	tlbsx	0, %r19

	/* Copy entry number to r10 */
	mfspr	%r17, SPR_MAS0
	rlwinm	%r10, %r17, 16, 28, 31

	/* Invalidate TLB1, skipping our entry. */

	mfspr	%r17, SPR_TLB1CFG	/* Get number of entries */
	andi.	%r17, %r17, TLBCFG_NENTRY_MASK@l
	li	%r16, 0			/* Start from Entry 0 */

2:	lis	%r15, MAS0_TLBSEL1@h	/* Select TLB1 */
	rlwimi	%r15, %r16, 16, 4, 15
	mtspr	SPR_MAS0, %r15
	isync
	tlbre
	mfspr	%r15, SPR_MAS1
	cmpw	%r16, %r10
	beq	3f
	/* Clear VALID and IPROT bits for other entries */
	rlwinm	%r15, %r15, 0, 2, 31
	mtspr	SPR_MAS1, %r15
	isync
	tlbwe
	isync
	msync
3:	addi	%r16, %r16, 1           /* increment to next index */
	cmpw	%r16, %r17		/* Check if this is the last entry */
	bne	2b

/*
 * Create temporary mapping in the other Address Space
 */
	lis	%r17, MAS0_TLBSEL1@h	/* Select TLB1 */
	rlwimi	%r17, %r10, 16, 12, 15	/* Select our entry */
	mtspr	SPR_MAS0, %r17
	isync
	tlbre				/* Read it in */

	/* Prepare and write temp entry */
	lis	%r17, MAS0_TLBSEL1@h	/* Select TLB1 */
	addi	%r11, %r10, 0x1		/* Use next entry. */
	rlwimi	%r17, %r11, 16, 12, 15	/* Select temp entry */
	mtspr	SPR_MAS0, %r17
	isync

	mfspr	%r16, SPR_MAS1
	li	%r15, 1			/* AS 1 */
	rlwimi	%r16, %r15, 12, 19, 19
	mtspr	SPR_MAS1, %r16
	li	%r17, 0
	rlwimi	%r16, %r17, 0, 8, 15	/* Global mapping, TID=0 */
	isync

	tlbwe
	isync
	msync

	mfmsr	%r16
	ori	%r16, %r16, 0x30	/* Switch to AS 1. */

	mflr	%r15
	addi	%r15, %r15, 4f - 1b	/* Increment to instruction after rfi */
	mtspr	SPR_SRR0, %r15
	mtspr	SPR_SRR1, %r16
	rfi				/* Switch context */

/*
 * Invalidate initial entry
 */
4:	mr	%r22, %r10
	bl	tlb1_inval_entry


/*
 * Setup final mapping in TLB1[BASE_TLB_ENTRY] and switch to it
 */
	/* Final kernel mapping */
	lis	%r16, MAS0_TLBSEL1@h	/* Select TLB1 */
	li	%r17, BASE_TLB_ENTRY	/* Entry index  */
	rlwimi	%r16, %r17, 16, 10, 15
	mtspr	SPR_MAS0, %r16
	isync

	li	%r16, (PHYSMAPSIZE << MAS1_TSIZE_SHIFT)@l
	oris	%r16, %r16, (MAS1_VALID | MAS1_IPROT)@h
	mtspr	SPR_MAS1, %r16
	isync

	/* Align mapping to size */
	#define MAP_MASK ((1 << ((PHYSMAPSIZE - 1) * 2 + 12)) - 1)

	/* We may have relocated away from address zero,
	 * and want the secondaries to map PHYSBASE to whatever
	 * 1MiB region they are running at (we know the hv will
	 * be covered by the boot mapping).
	 */
	lis	%r15, PHYSBASE@h
	ori	%r15, %r15, MAS2_M
	mtspr	SPR_MAS2, %r15		/* Set final EPN, **M** */
	isync

	/* r7 contains the size of the IMA as per ePAPR */
	mfspr	%r17, SPR_MAS3
	addi	%r16, %r7, -1
	and	%r18, %r19, %r16 // extract IMA offset
	andc	%r17, %r17, %r16 // extract IMA base
	rlwinm	%r18, %r18, 0, ~MAP_MASK
	rlwinm	%r17, %r17, 0, ~MAP_MASK
	or	%r17, %r17, %r18
	ori	%r17, %r17, (MAS3_SX | MAS3_SW | MAS3_SR)@l
	mtspr	SPR_MAS3, %r17		/* Set RPN and protection */
	isync
	tlbwe
	isync
	msync

	/* Switch to the above TLB1[BASE_TLB_ENTRY] mapping */
	lis	%r20, 6f@h
	ori	%r20, %r20, 6f@l
	li	%r21, MSR_RI@l
#ifdef CONFIG_LIBOS_64BIT
	oris	%r21, %r21, MSR_CM@h
#endif
	mtspr   SPR_SRR0, %r20
	mtspr   SPR_SRR1, %r21
	rfi

/*
 * Invalidate temp mapping
 */
6:	mr	%r22, %r11
	bl	tlb1_inval_entry

	mtlr	%r23
	blr

tlb1_inval_entry:
	lis	%r17, MAS0_TLBSEL1@h	/* Select TLB1 */
	rlwimi	%r17, %r22, 16, 12, 15	/* Select our entry */
	mtspr	SPR_MAS0, %r17
	isync
	tlbre				/* Read it in */

	li	%r16, 0
	mtspr	SPR_MAS1, %r16
	isync
	tlbwe
	isync
	msync
	blr

ivor_setup:
	/* Set base address of interrupt handler routines */
	lis	%r21, interrupt_vector_base@h
	mtspr	SPR_IVPR, %r21

	/* Assign interrupt handler routines offsets */
	li	%r21, int_critical_input@l
	mtspr	SPR_IVOR0, %r21
	li	%r21, int_machine_check@l
	mtspr	SPR_IVOR1, %r21
	li	%r21, int_data_storage@l
	mtspr	SPR_IVOR2, %r21
	li	%r21, int_instr_storage@l
	mtspr	SPR_IVOR3, %r21
	li	%r21, int_external_input@l
	mtspr	SPR_IVOR4, %r21
	li	%r21, int_alignment@l
	mtspr	SPR_IVOR5, %r21
	li	%r21, int_program@l
	mtspr	SPR_IVOR6, %r21
	li	%r21, int_fpunavail@l
	mtspr	SPR_IVOR7, %r21
	li	%r21, int_syscall@l
	mtspr	SPR_IVOR8, %r21
	li	%r21, int_decrementer@l
	mtspr	SPR_IVOR10, %r21
	li	%r21, int_fixed_interval_timer@l
	mtspr	SPR_IVOR11, %r21
	li	%r21, int_watchdog@l
	mtspr	SPR_IVOR12, %r21
	li	%r21, int_data_tlb_error@l
	mtspr	SPR_IVOR13, %r21
	li	%r21, int_inst_tlb_error@l
	mtspr	SPR_IVOR14, %r21
	li	%r21, int_debug@l
	mtspr	SPR_IVOR15, %r21
	li	%r21, int_perf_mon@l
	mtspr	SPR_IVOR35, %r21
	li	%r21, int_doorbell@l
	mtspr	SPR_IVOR36, %r21
	li	%r21, int_doorbell_critical@l
	mtspr	SPR_IVOR37, %r21
#ifdef HYPERVISOR
	li	%r21, int_guest_doorbell@l
	mtspr	SPR_IVOR38, %r21
	li	%r21, int_guest_doorbell_critical@l
	mtspr	SPR_IVOR39, %r21
	li	%r21, int_hypercall@l
	mtspr	SPR_IVOR40, %r21
	li	%r21, int_ehpriv@l
	mtspr	SPR_IVOR41, %r21
#endif

	blr

clear_bss:
	lis	%r23, (bss_start - 1)@h
	lis	%r24, (bss_end - 1)@h
	ori	%r23, %r23, (bss_start - 1)@l
	ori	%r24, %r24, (bss_end - 1)@l

	sub	%r25, %r24, %r23
	mtctr	%r25

	li	%r26, 0
1:	stbu	%r26, 1(%r23)
	bdnz	1b

	blr

	.section .stack, "aw", @nobits
	.align 4
	/* Initial boot stack */
	.space 4096
	.global init_stack_top
init_stack_top:

	.section .bss
	.global cache_block_size
cache_block_size:
	.space 4
