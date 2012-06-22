

/*
 * Copyright (C) 2007-2011 Freescale Semiconductor, Inc.
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
#include <libos/asm-macros.S>

#ifndef CONFIG_LIBOS_64BIT
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
	bl calc_ea_phys_off
	add %r3, %r3, %r30 /* physical = IMA effective + %r30 */
	
	/* Issue INV_ALL Invalidate on TLB0 */
	li      %r16, 0x04
	tlbivax	0, %r16
	isync
	msync
	nop

/*
 * Use tlbsx to locate the TLB1 entry that maps kernel code
 */
	li	%r23, fixup_mmu_end - 1f
	b	fixup_mmu

1:	LOADIMM	%r2, cpu0
	mtsprg0	%r2
#ifdef CONFIG_LIBOS_64BIT
	mr	%r13, %r2
	/* Setup %r2 to toc_start + 0x8000 as expected by the 64-bit ABI */
	LOADIMM	%r2, toc_start + 0x8000
#endif

	bl	clear_bss

	mfspr	%r16, SPR_L1CFG0	/* Read CBSIZE */
	li	%r17, 32		/* Compute cache block as 32*2^CBSIZE */
	LOADIMM	%r18, cache_block_size
	rlwinm	%r16, %r16, 9, 3
	slw	%r17, %r17, %r16
	stw	%r17, 0(%r18)

	/* Build a mask of valid TSIZEs based on MMU version */
	mfspr	%r16, SPR_MMUCFG	/* Check MAVN */
	andi.	%r16, %r16, MMUCFG_MAVN
	beq	1f
	mfspr	%r17, SPR_TLB1PS	/* MMUv2: r17 = TLB1PS */
	b	2f
1:
	/* MMUv1: r17 = synthetic mask */
	LOADIMM %r17, 0b10101010101010101010100
2:
	LOADIMM %r18, valid_tsize_mask
	stw	%r17, 0(%r18)

	LOADIMM	%r1, init_stack_top - 16

	bl	ivor_setup
	bl	libos_client_entry

1:	lis	%r3, 0xffff
	mtctr	%r3
2:	bdnz	2b
	b	1b

#ifdef CONFIG_LIBOS_MP
	.global secondary_start
secondary_start:
	bl	calc_ea_phys_off

	li	%r23, fixup_mmu_end - 1f
	b	fixup_mmu

#ifndef CONFIG_LIBOS_64BIT
1:	mr	%r2, %r3
#else
1:	mr	%r13, %r3
	/* Setup %r2 to toc_start + 0x8000 as expected by the 64-bit ABI */
	LOADIMM	%r2, toc_start + 0x8000
#endif
	mtsprg0	%r3
	LOAD	%r1, CPU_KSTACK(%r3)

	bl	ivor_setup
	bl	secondary_init

1:	lis	%r3, 0xffff
	mtctr	%r3
2:	bdnz	2b
	b	1b
#endif

/*
 * Calculate in r30 the offset between phys addr and effective addr where .text is
 * mapped
 */
calc_ea_phys_off:
	/* Translate the program counter address to physical.
	 * FIXME: 36-bit physical
	 */

	mflr	%r16			/* save LR so we know where to return */
	bl	1f
1:
	mflr	%r19			/* r19 = program counter */
	mtlr	%r16			/* restore LR */

#ifdef HYPERVISOR
	li	%r16, 0
	mtspr	SPR_MAS5, %r16		/* SGS = SLPID = 0 */
#endif

	mfmsr	%r16
	mfspr	%r17, SPR_PID
	rlwinm	%r17, %r17, 16, 0x3fff0000 /* turn PID into MAS6[SPID] */
	rlwimi	%r17, %r16, 28, 0x00000001 /* turn MSR[DS] into MAS6[SAS] */
	mtspr	SPR_MAS6, %r17

	tlbsx	0, %r19			/* must succeed */
	mfspr	%r16, SPR_MAS1
	rlwinm	%r17, %r16, 25, 0x1e	/* r17 = tsize*2 */
	li	%r18, 1024
	slw	%r18, %r18, %r17	/* r18 = page size */
	addi	%r18, %r18, -1
	and	%r16, %r19, %r18	/* r16 = page offset */
	mr	%r30, %r19
	mfspr   %r19, SPR_MAS3		/* r19 = rpn */
	andc	%r17, %r19, %r18	/* r17 = page base */
	or	%r17, %r17, %r16	/* r17 = pc phys addr */
	subf	%r30, %r30, %r17	/* %r30 = physical - IMA effective */
	blr

/* input: r30 = physical - effective addr (returned by calc_ea_phys_off) */
fixup_mmu:
	bl	1f
	/* Find entry that maps current address */
1:	mflr	%r19
	li	%r16, 0			/* Start invalidate from Entry 0 */
	mfspr	%r17, SPR_PID
#ifdef HYPERVISOR
	mtspr	SPR_MAS5, %r16		/* SGS = SLPID = 0 */
#endif
	slwi	%r17, %r17, MAS6_SPID_SHIFT
	mtspr	SPR_MAS6, %r17		/* SPID = PID, SAS = 0 */
	isync
	tlbsx	0, %r19

	/* Copy entry number to r10 */
	mfspr	%r17, SPR_MAS0
	rlwinm	%r10, %r17, 16, 26, 31

	/* Invalidate TLB1, skipping our entry. */

	mfspr	%r17, SPR_TLB1CFG	/* Get number of entries */
	andi.	%r17, %r17, TLBCFG_NENTRY_MASK@l

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
	rlwimi	%r17, %r10, 16, 10, 15	/* Select our entry */
	mtspr	SPR_MAS0, %r17
	isync
	tlbre				/* Read it in */

	/* Prepare and write temp entry */
	lis	%r17, MAS0_TLBSEL1@h	/* Select TLB1 */
	addi	%r11, %r10, 0x1		/* Use next entry. */
	rlwimi	%r17, %r11, 16, 10, 15	/* Select temp entry */
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

#if defined(CONFIG_LIBOS_64BIT)
	oris	%r16, %r16, MSR_CM@h	/* Switch to 64 bit */
#endif
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
	#define MAP_MASK ((1024 << PHYSMAPSIZE) - 1)

	/* We may have relocated away from address zero,
	 * and want the secondaries to map PHYSBASE to whatever
	 * 1MiB region they are running at (we know the hv will
	 * be covered by the boot mapping).
	 */
	LOADIMM	%r15, PHYSBASE
	ori	%r15, %r15, MAS2_M
	mtspr	SPR_MAS2, %r15		/* Set final EPN, **M** */
	isync

	/* We assume that we're loaded at a multiple of PHYSMAPSIZE */ 
	add	%r17, %r19, %r30	/* phys addr of return address */
	rlwinm	%r17, %r17, 0, ~MAP_MASK
	ori	%r18, %r17, (MAS3_SX | MAS3_SW | MAS3_SR)@l
	mtspr	SPR_MAS3, %r18		/* Set RPN and protection */
	isync
	tlbwe
	isync
	msync

	/* Switch to the above TLB1[BASE_TLB_ENTRY] mapping */
	LOADIMM	%r20, 6f
	li	%r21, MSR_RI@l
#ifdef CONFIG_LIBOS_64BIT
	oris	%r21, %r21, MSR_CM@h
#endif
	mtspr   SPR_SRR0, %r20
	mtspr   SPR_SRR1, %r21
	rfi

6:
	LOADIMM	%r18, physbase_phys
	STORE	%r17, 0(%r18)

/*
 * Invalidate temp mapping
 */
	mr	%r22, %r11
	bl	tlb1_inval_entry

	LOADIMM	%r16, fixup_mmu_end
	subf	%r23, %r23, %r16
	mtlr	%r23
fixup_mmu_end:
	blr

tlb1_inval_entry:
	lis	%r17, MAS0_TLBSEL1@h	/* Select TLB1 */
	rlwimi	%r17, %r22, 16, 10, 15	/* Select our entry */
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
#ifdef CONFIG_LIBOS_64BIT
	lis	%r21, EPCR_ICM@h
	mtspr	SPR_EPCR, %r21
#endif
	/* Set base address of interrupt handler routines */
	lis	%r23, 0xffff		/* Sign extension on 32/64 bits */
	LOADIMM	%r21, interrupt_vector_base
	and	%r21, %r21, %r23	/* Skip the reserved low 16 bits */
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
#ifdef CONFIG_LIBOS_POWERISA_E_PC
	li	%r21, int_doorbell@l
	mtspr	SPR_IVOR36, %r21
	li	%r21, int_doorbell_critical@l
	mtspr	SPR_IVOR37, %r21
#endif
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
	LOADIMM	%r23, bss_start - 1
	LOADIMM	%r24, bss_end - 1

	sub	%r25, %r24, %r23
	mtctr	%r25

	li	%r26, 0
1:	stbu	%r26, 1(%r23)
	bdnz	1b

	blr

	/* This is written before the BSS is cleared, so put in .data */
	.section .data, "aw"
	.balign 8
	.global physbase_phys
physbase_phys:
	.space 8

	.section .stack, "aw", @nobits
	.balign 16
	/* Initial boot stack */
	.space KSTACK_SIZE
	.global init_stack_top
init_stack_top:

	.section .bss
	.global cache_block_size
	.balign 4
cache_block_size:
	.space 4
	.global valid_tsize_mask
valid_tsize_mask:
	.space 4
