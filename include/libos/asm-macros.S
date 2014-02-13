/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
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

	.macro LOADIMM, register, value
#ifndef CONFIG_LIBOS_64BIT
	lis     \register, (\value)@h
	ori     \register, \register, (\value)@l
#else
	lis     \register, (\value)@highest
	ori     \register, \register, (\value)@higher
	rldicr  \register, \register, 32, 31
	oris    \register, \register, (\value)@h
	ori     \register, \register, (\value)@l
#endif
	.endm

/* Reads the current timebase (only the low part for 32-bit)
 * with workaround for erratum A-006958.
 * The erratum states that: a 64-bit read of TBL may appear
 * to count backwards if the timebase increment causes a carry
 * out from TBL into TBU.
 *
 * The workaround consists in looping while the lower half of
 * timebase is zero. This simple workaround avoids using scratch
 * registers and overwrites only CR0.
 **/
	.macro GET_TBL, register, label
\label:
	mfspr   \register, SPR_TBL
#ifdef CONFIG_LIBOS_64BIT
	cmpwi   \register, 0
	beq     \label\()b
#endif
	.endm

