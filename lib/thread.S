/** @file
 * Thread switching
 */

/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
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

/* FIXME 64-bit */
.global switch_thread
switch_thread:
	stwu	%r1, -96(%r1)
	stmw	%r14, 24(%r1)
	mflr	%r4
	
	lwz	%r5, CPU_THREAD(%r2)
	bl	1f
1:	stw	%r3, CPU_THREAD(%r2)
	mflr	%r7
	stw	%r1, THREAD_STACK(%r5)
	addi	%r7, %r7, 2f - 1b
	mfcr	%r8
	stw	%r7, THREAD_PC(%r5)
	lwz	%r6, THREAD_PC(%r3)
	stw	%r4, 100(%r1)
	stw	%r8, 20(%r1)
	lwz	%r1, THREAD_STACK(%r3)
	mtlr	%r6
	blr	

2:	lwz	%r4, 100(%r1)
	lwz	%r8, 20(%r1)
	lmw	%r14, 24(%r1)
	mtlr	%r4
	mtcr	%r8
	addi	%r1, %r1, 96
	blr
