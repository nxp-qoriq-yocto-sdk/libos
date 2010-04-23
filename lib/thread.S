/** @file
 * Thread switching
 */

/*
 * Copyright (C) 2009,2010 Freescale Semiconductor, Inc.
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

#ifndef CONFIG_LIBOS_64BIT
/* 32-bit */
#define LONGBYTES 4
#define LOAD lwz
#define STORE stw
#define STOREU stwu
/* As defined by 32-bit PowerPC ABI specifications */
#define GPR_SAVE_AREA 18*LONGBYTES
#define GPR_SAVE_AREA_OFFSET 24
#define CR_SAVE_WORD_OFFSET 20
#define STACK_FRAMELEN (GPR_SAVE_AREA + GPR_SAVE_AREA_OFFSET)
#define LR_SAVE_WORD_OFFSET (STACK_FRAMELEN + 4)
#define PERCPU_REG %r2

	.macro SAVE_GPRs
	stmw	%r14, GPR_SAVE_AREA_OFFSET(%r1)
	.endm

	.macro RESTORE_GPRs
	lmw	%r14, GPR_SAVE_AREA_OFFSET(%r1)
	.endm

#else
/* 64-bit */
#define LONGBYTES 8
#define LOAD ld
#define STORE std
#define STOREU stdu
/* As defined by 64-bit PowerPC ELF ABI specifications */
#define GPR_SAVE_AREA 18*LONGBYTES
/* Minimum stack frame overhead on PPC64 */
#define GPR_SAVE_AREA_OFFSET 112
#define CR_SAVE_WORD_OFFSET LONGBYTES*1
#define STACK_FRAMELEN (GPR_SAVE_AREA + GPR_SAVE_AREA_OFFSET)
#define LR_SAVE_WORD_OFFSET (STACK_FRAMELEN + LONGBYTES * 2)
#define PERCPU_REG %r13

	.macro SAVE_GPRs
	.irp REG, 14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	std %r\REG, GPR_SAVE_AREA_OFFSET + (LONGBYTES * (\REG-14))(%r1)
	.endr
	.endm

	.macro RESTORE_GPRs
	.irp REG, 14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	ld %r\REG, GPR_SAVE_AREA_OFFSET + (LONGBYTES * (\REG-14))(%r1)
	.endr
	.endm

#endif

.global switch_thread
switch_thread:
	/* Save old thread's state */
	STOREU	%r1, -STACK_FRAMELEN(%r1)
	SAVE_GPRs
	mr	%r5, %r3
	mflr	%r4
	
	LOAD	%r3, CPU_THREAD(PERCPU_REG)
	LOAD	%r9, THREAD_KSTACK(%r5)
	bl	1f
1:	STORE	%r5, CPU_THREAD(PERCPU_REG)
	mflr	%r7
	STORE	%r1, THREAD_STACK(%r3)
	addi	%r7, %r7, 2f - 1b
	STORE	%r9, CPU_KSTACK(PERCPU_REG)
	mfcr	%r8
	STORE	%r7, THREAD_PC(%r3)
	LOAD	%r6, THREAD_PC(%r5)
	STORE	%r4, LR_SAVE_WORD_OFFSET(%r1)
	STORE	%r8, CR_SAVE_WORD_OFFSET(%r1)
	LOAD	%r1, THREAD_STACK(%r5)

	/* New threads will branch to their entry point; existing
	 * threads will fall through to 2:
	 */
	mtlr	%r6
	blr	

	/* Restore new thread's state, returning the
	 * switched-from thread in r3.
	 */
2:	LOAD	%r4, LR_SAVE_WORD_OFFSET(%r1)
	LOAD	%r8, CR_SAVE_WORD_OFFSET(%r1)
	RESTORE_GPRs
	mtlr	%r4
	mtcr	%r8
	addi	%r1, %r1, STACK_FRAMELEN
	blr
