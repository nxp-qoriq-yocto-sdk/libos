/*
 * Exception vectors
 *
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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
 * SPRG3: reserved for client
 * SPRG4-7: reserved for guest
 * SPRG8: machine check scratch
 * SPRG9: debug exception scratch
 */

#ifndef CONFIG_LIBOS_64BIT
/* 32-bit */
#define LONGBYTES 4
#define LOAD lwz
#define STORE stw
#define PERCPU_REG %r2
#define PERCPU_REG_SAVE 2
#define VOLATILE_STORAGE 0

	.macro SAVE_GPRs
	stmw	%r3, LONGBYTES*3 + FRAME_GPREGS(%r1)
	.endm

	.macro RESTORE_GPRs
	lmw	%r2, LONGBYTES*2 + FRAME_GPREGS(%r1)
	.endm

#else
/* 64-bit */
#define LONGBYTES 8
#define LOAD ld
#define STORE std
#define PERCPU_REG %r13
#define PERCPU_REG_SAVE 13
/*
 * 288 bytes below stack pointer is volatile storage which should be preserved
 * by interrupt handlers as per 64-bit PowerPC ABI
 */
#define VOLATILE_STORAGE 288

	.macro SAVE_GPRs
	.irp REG, 2,3,4,5,6,7,8,9,10,11,12
	std %r\REG, LONGBYTES*2 + (FRAME_GPREGS + (\REG-2) * LONGBYTES)(%r1)
	.endr

	.irp REG, 14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	std %r\REG, LONGBYTES*14 + (FRAME_GPREGS + (\REG-14) * LONGBYTES)(%r1)
	.endr

	/* Setup %r2 to toc_start + 0x8000 as expected by the 64-bit ABI */
	lis	%r2, (toc_start + 0x8000)@h
	ori	%r2, %r2, (toc_start + 0x8000)@l
	.endm

	.macro RESTORE_GPRs
	.irp REG, 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23, \
		24,25,26,27,28,29,30,31
	ld %r\REG, LONGBYTES*2 + (FRAME_GPREGS + (\REG-2) * LONGBYTES)(%r1)
	.endr
	.endm
#endif

	.macro	exception, handler, type, exc, name, scratch, \
	        srr0, srr1, stack, savearea, traplevel, keepee
	.global \name
	.align	4
\name:
	mtspr	\scratch, PERCPU_REG
	mfspr	PERCPU_REG, SPR_SPRG0   /* PERCPU_REG = percpu data pointer */

	STORE	%r0, LONGBYTES*0 + \savearea(PERCPU_REG)
	STORE	%r1, LONGBYTES*1 + \savearea(PERCPU_REG)
	mfcr	%r0
	\stack
	stw	%r0, FRAME_CR(%r1)

	SAVE_GPRs

	LOAD	%r0, LONGBYTES*0 + \savearea(PERCPU_REG)
	LOAD	%r3, LONGBYTES*1 + \savearea(PERCPU_REG)
	mfspr	%r4, \scratch
	mflr	%r5
	mfctr	%r6
	mfxer	%r8
	mfspr	%r9, \srr0
	mfspr	%r10, \srr1
	mfspr	%r11, SPR_DEAR
	mfspr	%r12, SPR_ESR
	li	%r14, \exc     /* Avoid using %r13 as is PERCPU_REG in 64-bit */

	STORE	%r0, LONGBYTES*0 + FRAME_GPREGS(%r1)
	STORE	%r3, LONGBYTES*1 + FRAME_GPREGS(%r1)
	STORE	%r4, LONGBYTES*PERCPU_REG_SAVE + FRAME_GPREGS(%r1)
	STORE	%r5, FRAME_LR(%r1)
	STORE	%r6, FRAME_CTR(%r1)
	stw	%r8, FRAME_XER(%r1)
	STORE	%r9, FRAME_SRR0(%r1)
	STORE	%r10, FRAME_SRR1(%r1)

	lwz	%r3, CPU_TRAPLEVEL(PERCPU_REG)
	STORE	%r11, FRAME_DEAR(%r1)
	STORE	%r12, FRAME_ESR(%r1)
	stw	%r14, FRAME_EXC(%r1)

	cmplwi	%r3, \traplevel
	stw	%r3, FRAME_TRAPLEVEL(%r1)
	bgt-	1f
	li	%r4, \traplevel
	stw	%r4, CPU_TRAPLEVEL(PERCPU_REG)
1:
	
#ifdef HYPERVISOR
	andis.	%r0, %r10, MSR_GS@h
	beq-	2f
#ifdef CONFIG_LIBOS_STATISTICS
	mfspr	%r3, SPR_TBL
	li	%r4, 0
	STORE	%r3, FRAME_INITIAL_CYCLES(%r1)
	STORE	%r4, FRAME_CURRENT_EVENT(%r1)
#endif
	mfspr	%r3, SPR_EPLC
	mfspr	%r4, SPR_EPSC
	mfspr	%r5, SPR_PID
	mfspr	%r6, SPR_LPIDR
	
	stw	%r3, FRAME_EPLC(%r1)
	stw	%r4, FRAME_EPSC(%r1)

#ifdef TOPAZ	
	/* Set the PID, LPIDR, guest/hypervisor, and user/kernel state for
	 * external PID access.  We don't set AS0/AS1 in EPLC, as it
	 * depends on whether it's a data or instruction access which MSR
	 * bit we copy from.  However, we shouldn't be storing to
	 * instruction addresses, so EPSC assumes data.
	 *
	 * FIXME: don't do this here; EPSC/EPLC writes are expensive.
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
	isync
#endif

	/* Don't let the guest's clearing of EE/CE/ME/RI affect the hypervisor */
	.if	\savearea != CPU_MACHKSAVE
	mfmsr	%r0
	ori	%r0, %r0, MSR_RI@l
	.if	\savearea == CPU_NORMSAVE
	oris	%r0, %r0, MSR_CE@h
	.endif
	.if	(\savearea == CPU_NORMSAVE) && (\keepee != 0)
	ori	%r0, %r0, (MSR_ME|MSR_EE)@l
	.else
	ori	%r0, %r0, MSR_ME@l
	.endif
	mtmsr	%r0
	.endif

	b	3f

	/* If requested, preserve EE when not changing privilege levels */
	.if	\keepee != 0
	mfmsr	%r0
	rlwimi	%r0, %r10, 0, MSR_EE
	mtmsr	%r0
	.endif
2:	
#else
	.if	\keepee != 0
	andi.	%r0, %r10, MSR_PR
	bne+	3f
	mfmsr	%r0
	rlwimi	%r0, %r10, 0, MSR_EE
	mtmsr	%r0
	.endif
#endif

3:	mr	%r3, %r1
	bl	\handler
#ifdef CONFIG_LIBOS_STATISTICS
	LOAD	%r10, FRAME_SRR1(%r1)
	andis.	%r0, %r10, MSR_GS@h
	beq-	4f
	LOAD	%r3, FRAME_INITIAL_CYCLES(%r1)
	LOAD	%r4, FRAME_CURRENT_EVENT(%r1)
	bl	UPDATE_STATS
4:
#endif
	b	ret_from_\type
.endm

	.macro	ret_from_exc_type, type, srr0, srr1, ret, disable
.global ret_from_\type
ret_from_\type:
	mfmsr	%r10
	\disable %r10
	mtmsr	%r10

#ifdef LIBOS_RET_HOOK
	lwz	%r10, CPU_RETHOOK(PERCPU_REG)
	cmpwi	%r10, 0
	beq+	1f
	
	mr	%r3, %r1
	bl	LIBOS_RET_HOOK
1:
#endif

	LOAD	%r10, FRAME_SRR1(%r1)

#ifdef TOPAZ
	andis.	%r0, %r10, MSR_GS@h
	beq+	2f

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
	stw	%r11, CPU_TRAPLEVEL(PERCPU_REG)

	RESTORE_GPRs

	LOAD	%r0, LONGBYTES*0 + FRAME_GPREGS(%r1)
	LOAD	%r1, LONGBYTES*1 + FRAME_GPREGS(%r1)

	\ret
	.endm

	.macro	basestack
	mfspr	%r1, SPR_SRR1
#ifdef HYPERVISOR
	rlwinm	%r1, %r1, 3, ~0
	andis.	%r1, %r1, ((MSR_GS | MSR_PR) << 3)@h
#else
	andi.	%r1, %r1, MSR_PR@l
#endif
	beq-	1f
	
	.subsection 1
1:	LOAD	%r1, LONGBYTES*1 + CPU_NORMSAVE(PERCPU_REG)
	stwu	%r1, -(FRAMELEN + VOLATILE_STORAGE)(%r1)
	b	2f
	.subsection 0

	LOAD	%r1, CPU_KSTACK(PERCPU_REG)
2:
	.endm

	.macro	critdisable, reg
	andis.	\reg, \reg, ~(MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_CE|MSR_EE)@l
	.endm

	.macro	basedisable, reg
	rlwinm	\reg, \reg, 0, ~MSR_EE
	.endm

	.macro	baseexception, exc, name, handler, keepee
	exception \handler exception \exc \name SPR_SPRG1 SPR_SRR0 SPR_SRR1 \
	          basestack CPU_NORMSAVE TRAPLEVEL_NORMAL \keepee
	.endm

	ret_from_exc_type exception SPR_SRR0 SPR_SRR1 rfi basedisable

	.macro	critstack
#ifndef CONFIG_LIBOS_64BIT
	addi	%r1, PERCPU_REG, CPU_CRITSTACK + KSTACK_SIZE - FRAMELEN
#else
	addis	%r1, PERCPU_REG, (CPU_CRITSTACK + KSTACK_SIZE - FRAMELEN)@ha
	addi	%r1, PERCPU_REG, (CPU_CRITSTACK + KSTACK_SIZE - FRAMELEN)@l
#endif
	.endm

	.macro	critexception, exc, name, handler
	exception \handler crit \exc \name SPR_SPRG2 SPR_CSRR0 SPR_CSRR1 \
	          critstack CPU_CRITSAVE TRAPLEVEL_CRIT 0
	.endm

	ret_from_exc_type crit SPR_CSRR0 SPR_CSRR1 rfci critdisable

	.macro	mcheckstack
#ifndef CONFIG_LIBOS_64BIT
	addi	%r1, PERCPU_REG, CPU_MCHECKSTACK + KSTACK_SIZE - FRAMELEN
#else
	addis	%r1, PERCPU_REG, (CPU_MCHECKSTACK + KSTACK_SIZE - FRAMELEN)@ha
	addi	%r1, PERCPU_REG, (CPU_MCHECKSTACK + KSTACK_SIZE - FRAMELEN)@l
#endif
	.endm

	.macro	mcdisable, reg
	andis.	\reg, \reg, ~(MSR_ME|MSR_DE|MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_ME|MSR_DE|MSR_CE|MSR_EE)@l
	.endm

	.macro	machinecheck, exc, name, handler
	exception \handler mcheck \exc \name SPR_SPRG8 SPR_MCSRR0 SPR_MCSRR1 \
	          mcheckstack CPU_MACHKSAVE TRAPLEVEL_MCHECK 0
	.endm

	ret_from_exc_type mcheck SPR_MCSRR0 SPR_MCSRR1 rfmci mcdisable

	.macro	debugstack
	addi	%r1, PERCPU_REG, CPU_DEBUGSTACK + KSTACK_SIZE - FRAMELEN
	.endm

	.macro	debugdisable, reg
	andis.	\reg, \reg, ~(MSR_DE|MSR_CE|MSR_EE)@h
	andi.	\reg, \reg, ~(MSR_DE|MSR_CE|MSR_EE)@l
	.endm

	.macro	debugexception, exc, name, handler
	exception \handler debug \exc \name SPR_SPRG9 SPR_DSRR0 SPR_DSRR1 \
	          debugstack CPU_DBGSAVE TRAPLEVEL_DEBUG 0
	.endm

	ret_from_exc_type debug SPR_DSRR0 SPR_DSRR1 rfdi debugdisable

	.global interrupt_vector_base
	.align 5
interrupt_vector_base:
	critexception EXC_CRIT_INT int_critical_input EXC_CRIT_INT_HANDLER
	machinecheck EXC_MCHECK int_machine_check EXC_MCHECK_HANDLER
	baseexception EXC_DSI int_data_storage EXC_DSI_HANDLER 1
	baseexception EXC_ISI int_instr_storage EXC_ISI_HANDLER 1
	baseexception EXC_EXT_INT int_external_input EXC_EXT_INT_HANDLER 0
	baseexception EXC_ALIGN int_alignment EXC_ALIGN_HANDLER 1
	baseexception EXC_PROGRAM int_program EXC_PROGRAM_HANDLER 1
	baseexception EXC_FPUNAVAIL int_fpunavail EXC_FPUNAVAIL_HANDLER 1
	baseexception EXC_SYSCALL int_syscall EXC_SYSCALL_HANDLER 1
	baseexception EXC_AUXUNAVAIL int_auxunavail EXC_AUXUNAVAIL_HANDLER 1
	baseexception EXC_DECR int_decrementer EXC_DECR_HANDLER 0
	baseexception EXC_FIT int_fixed_interval_timer EXC_FIT_HANDLER 0
	critexception EXC_WDOG int_watchdog EXC_WDOG_HANDLER
	baseexception EXC_DTLB int_data_tlb_error EXC_DTLB_HANDLER 1
	baseexception EXC_ITLB int_inst_tlb_error EXC_ITLB_HANDLER 1
	debugexception EXC_DEBUG int_debug EXC_DEBUG_HANDLER
	baseexception EXC_PERFMON int_perf_mon EXC_PERFMON_HANDLER 0
	baseexception EXC_DOORBELL int_doorbell EXC_DOORBELL_HANDLER 0
	critexception EXC_DOORBELLC int_doorbell_critical EXC_DOORBELLC_HANDLER
#ifdef HYPERVISOR
	baseexception EXC_GDOORBELL int_guest_doorbell EXC_GDOORBELL_HANDLER 1
	critexception EXC_GDOORBELLC int_guest_doorbell_critical EXC_GDOORBELLC_HANDLER
	baseexception EXC_HCALL int_hypercall EXC_HCALL_HANDLER 1
	baseexception EXC_EHPRIV int_ehpriv EXC_EHPRIV_HANDLER 1
#endif
