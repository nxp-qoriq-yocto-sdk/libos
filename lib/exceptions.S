/*
 * Exception vectors
 *
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Author: Scott Wood <scottwood@freescale.com>
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

#include <libos/spr.h> 
#include <libos/percpu.h>
#include <libos/trap_booke.h>

/* TODO
 *    -watchdog should be critical
 *    -uncomment IVOR init in head.S
 *    -don't need any TLB stuff in CPU
 *    -need separate exit for guest only exceptions, if it didn't come
 *     guest it's fatal
 *    -hypercall exception doesn't need to save registers
 *     (but if it doesn't, it should zero out volatiles to avoid
 *      information leakage).
 */

/* SPRG allocation:
 * SPRG0: per-CPU data pointer
 * SPRG1: normal exception scratch
 * SPRG2: critical exception scratch
 * SPRG3: unused
 * SPRG4-7: reserved for guest
 * SPRG8: machine check scratch
 * SPRG9: debug exception scratch
 */

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

	.macro	exception, handler, exc, name, scratch, \
	        srr0, srr1, stack, savearea, traplevel, ret, disable
	.global \name
	.align	4
\name:
	mtspr	\scratch, %r2
	mfspr	%r2, SPR_SPRG0   /* r2 = percpu data pointer */

	STORE	%r0, LONGBYTES*0 + \savearea(%r2)
	STORE	%r1, LONGBYTES*1 + \savearea(%r2)
	mfcr	%r0
	\stack
	stw	%r0, FRAME_CR(%r1)

	/* fixme for 64-bit */
	stmw	%r3, LONGBYTES*3 + FRAME_GPREGS(%r1)

	LOAD	%r0, LONGBYTES*0 + \savearea(%r2)
	LOAD	%r3, LONGBYTES*1 + \savearea(%r2)
	mfspr	%r4, \scratch
	mflr	%r5
	mfctr	%r6
	mfxer	%r8
	mfspr	%r9, \srr0
	mfspr	%r10, \srr1
	mfspr	%r11, SPR_DEAR
	mfspr	%r12, SPR_ESR
	li	%r13, \exc

	STORE	%r0, LONGBYTES*0 + FRAME_GPREGS(%r1)
	STORE	%r3, LONGBYTES*1 + FRAME_GPREGS(%r1)
	STORE	%r4, LONGBYTES*2 + FRAME_GPREGS(%r1)
	STORE	%r5, FRAME_LR(%r1)
	STORE	%r6, FRAME_CTR(%r1)
	stw	%r8, FRAME_XER(%r1)
	STORE	%r9, FRAME_SRR0(%r1)
	STORE	%r10, FRAME_SRR1(%r1)

	lwz	%r3, CPU_TRAPLEVEL(%r2)
	STORE	%r11, FRAME_DEAR(%r1)
	STORE	%r12, FRAME_ESR(%r1)
	stw	%r13, FRAME_EXC(%r1)

	cmplwi	%r3, \traplevel
	stw	%r3, FRAME_TRAPLEVEL(%r1)
	bgt-	1f
	li	%r4, \traplevel
	stw	%r4, CPU_TRAPLEVEL(%r2)
1:
	
#ifdef HYPERVISOR
	andis.	%r0, %r10, MSR_GS@h
	beq	1f
	
	mfspr	%r3, SPR_EPLC
	mfspr	%r4, SPR_EPSC
	mfspr	%r5, SPR_PID
	mfspr	%r6, SPR_LPIDR
	
	stw	%r3, FRAME_EPLC(%r1)
	stw	%r4, FRAME_EPSC(%r1)
	
	/* Set the PID, LPIDR, guest/hypervisor, and user/kernel state for
	 * external PID access.  We don't set AS0/AS1 in EPLC, as it
	 * depends on whether it's a data or instruction access which MSR
	 * bit we copy from.  However, we shouldn't be storing to
	 * instruction addresses, so EPSC assumes data.
	 */
	
	rlwimi	%r3, %r5, EPC_EPID_SHIFT, EPC_EPID
	rlwimi	%r4, %r5, EPC_EPID_SHIFT, EPC_EPID
	rlwimi	%r3, %r6, EPC_ELPID_SHIFT, EPC_ELPID
	rlwimi	%r4, %r6, EPC_ELPID_SHIFT, EPC_ELPID

	rlwimi	%r3, %r10, MSRBIT_PR - EPCBIT_EPR, EPC_EPR
	rlwimi	%r4, %r10, MSRBIT_PR - EPCBIT_EPR, EPC_EPR

	rlwimi	%r4, %r10, MSRBIT_DS - EPCBIT_EAS, EPC_EAS
	
	/* Always guest access */
	oris	%r3, %r3, EPC_EGS@h
	oris	%r4, %r4, EPC_EGS@h
	
	mtspr	SPR_EPLC, %r3
	mtspr	SPR_EPSC, %r4

1:
	.if	\savearea == CPU_NORMSAVE
	mfmsr	%r0
	oris	%r0, %r0, MSR_CE@h
	mtmsr	%r0
	.endif
#endif
	
	/* Terminate backchain */
	li	%r0, 0
	STORE	%r0, 0(%r1)
	
	mr	%r3, %r1
	bl	\handler

	mfmsr	%r10
	\disable %r10
	mtmsr	%r10

#ifdef LIBOS_RET_USER_HOOK
	LOAD	%r10, FRAME_SRR1(%r1)
#ifdef HYPERVISOR
	andis.	%r11, %r10, MSR_GS@h
	bne+	2f
#endif

	andi.	%r10, %r10, MSR_PR@l
	beq-	1f
	
2:	lwz	%r10, CPU_RETUSERHOOK(%r2)
	cmpwi	%r10, 0
	beq+	1f
	
	mr	%r3, %r1
	bl	LIBOS_RET_USER_HOOK
1:
#endif

	LOAD	%r10, FRAME_SRR1(%r1)

#ifdef HYPERVISOR
	andis.	%r0, %r10, MSR_GS@h
	beq	2f

	lwz	%r3, FRAME_EPLC(%r1)
	lwz	%r4, FRAME_EPSC(%r1)
	mtspr	SPR_EPLC, %r3
	mtspr	SPR_EPSC, %r4

2:
#endif
	stwcx.	%r0, 0, %r1

	LOAD	%r5, FRAME_LR(%r1)
	LOAD	%r6, FRAME_CTR(%r1)
	lwz	%r7, FRAME_CR(%r1)
	lwz	%r8, FRAME_XER(%r1)
	LOAD	%r9, FRAME_SRR0(%r1)
	lwz	%r11, FRAME_TRAPLEVEL(%r1)

	mtlr	%r5
	mtctr	%r6
	mtcr	%r7
	mtxer	%r8
	mtspr	\srr0, %r9
	mtspr	\srr1, %r10
	stw	%r11, CPU_TRAPLEVEL(%r2)

	/* fixme for 64-bit */
	lmw	%r2, LONGBYTES*2 + FRAME_GPREGS(%r1)

	LOAD	%r0, LONGBYTES*0 + FRAME_GPREGS(%r1)
	LOAD	%r1, LONGBYTES*1 + FRAME_GPREGS(%r1)

	\ret
	.endm

	.macro	basestack
	mfspr	%r1, SPR_SRR1
#ifdef HYPERVISOR
	andis.	%r1, %r1, (MSR_GS | MSR_PR)@h
#else
	andis.	%r1, %r1, MSR_PR@h
#endif
	beq-	1f
	
	.subsection 1
1:	LOAD	%r1, LONGBYTES*1 + CPU_NORMSAVE(%r2)
	stwu	%r1, -FRAMELEN(%r1)
	b	2f
	.subsection 0

	lwz	%r1, CPU_KSTACK(%r2)
2:
	.endm

	.macro	basedisable, reg
	rlwinm	\reg, \reg, 0, ~MSR_EE
	.endm

	.macro	baseexception, exc, name, handler
#ifdef TOPAZ
	exception \handler \exc \name SPR_SPRG1 SPR_SRR0 SPR_SRR1 \
	          basestack CPU_NORMSAVE 0 rfi critdisable
#else
	exception \handler \exc \name SPR_SPRG1 SPR_SRR0 SPR_SRR1 \
	          basestack CPU_NORMSAVE 0 rfi basedisable
#endif
	.endm

#ifdef TOPAZ
	/* Topaz needs nestable critical interrupts */
	.macro	critstack
	lwz	%r1, CPU_TRAPLEVEL(%r2)
	cmpwi	%r1, 0
	bne-	1f
	
	.subsection 1
1:	LOAD	%r1, LONGBYTES*1 + CPU_CRITSAVE(%r2)
	stwu	%r1, -FRAMELEN(%r1)
	b	2f
	.subsection 0

	addi	%r1, %r2, CPU_CRITSTACK + KSTACK_SIZE - FRAMELEN
2:
	.endm
#else
	.macro	critstack
	addi	%r1, %r2, CPU_CRITSTACK + KSTACK_SIZE - FRAMELEN
	.endm
#endif

	.macro	critdisable, reg
	andis.	\reg, \reg, ~(MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_CE|MSR_EE)@l
	.endm

	.macro	critexception, exc, name, handler
	exception \handler \exc \name SPR_SPRG2 SPR_CSRR0 SPR_CSRR1 \
	          critstack CPU_CRITSAVE 1 rfci critdisable
	.endm

	.macro	mcheckstack
	addi	%r1, %r2, CPU_MCHECKSTACK + KSTACK_SIZE - FRAMELEN
	.endm

	.macro	mcdisable, reg
	andis.	\reg, \reg, ~(MSR_ME|MSR_DE|MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_ME|MSR_DE|MSR_CE|MSR_EE)@l
	.endm

	.macro	machinecheck, exc, name, handler
	exception \handler \exc \name SPR_SPRG8 SPR_MCSRR0 SPR_MCSRR1 \
	          mcheckstack CPU_MACHKSAVE 2 rfmci mcdisable
	.endm

	.macro	debugstack
	addi	%r1, %r2, CPU_DEBUGSTACK + KSTACK_SIZE - FRAMELEN
	.endm

	.macro	debugdisable, reg
	andis.	\reg, \reg, ~(MSR_DE|MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_DE|MSR_CE|MSR_EE)@l
	.endm

	.macro	debugexception, exc, name, handler
	exception \handler \exc \name SPR_SPRG9 SPR_DSRR0 SPR_DSRR1 \
	          debugstack CPU_DBGSAVE 3 rfdi debugdisable
	.endm

	.global interrupt_vector_base
	.align 5
interrupt_vector_base:
	critexception EXC_CRIT_INT int_critical_input EXC_CRIT_INT_HANDLER
	machinecheck EXC_MCHECK int_machine_check EXC_MCHECK_HANDLER
	baseexception EXC_DSI int_data_storage EXC_DSI_HANDLER
	baseexception EXC_ISI int_instr_storage EXC_ISI_HANDLER
	baseexception EXC_EXT_INT int_external_input EXC_EXT_INT_HANDLER
	baseexception EXC_ALIGN int_alignment EXC_ALIGN_HANDLER
	baseexception EXC_PROGRAM int_program EXC_PROGRAM_HANDLER
	baseexception EXC_FPUNAVAIL int_fpunavail EXC_FPUNAVAIL_HANDLER
	baseexception EXC_SYSCALL int_syscall EXC_SYSCALL_HANDLER
	baseexception EXC_AUXUNAVAIL int_auxunavail EXC_AUXUNAVAIL_HANDLER
	baseexception EXC_DECR int_decrementer EXC_DECR_HANDLER
	baseexception EXC_FIT int_fixed_interval_timer EXC_FIT_HANDLER
	critexception EXC_WDOG int_watchdog EXC_WDOG_HANDLER
	baseexception EXC_DTLB int_data_tlb_error EXC_DTLB_HANDLER
	baseexception EXC_ITLB int_inst_tlb_error EXC_ITLB_HANDLER
	debugexception EXC_DEBUG int_debug EXC_DEBUG_HANDLER
	baseexception EXC_PERFMON int_perf_mon EXC_PERFMON_HANDLER
	baseexception EXC_DOORBELL int_doorbell EXC_DOORBELL_HANDLER
	critexception EXC_DOORBELLC int_doorbell_critical EXC_DOORBELLC_HANDLER
#ifdef HYPERVISOR
	baseexception EXC_GDOORBELL int_guest_doorbell EXC_GDOORBELL_HANDLER
	critexception EXC_GDOORBELLC int_guest_doorbell_critical EXC_GDOORBELLC_HANDLER
	baseexception EXC_HCALL int_hypercall EXC_HCALL_HANDLER
	baseexception EXC_EHPRIV int_ehpriv EXC_EHPRIV_HANDLER
#endif
