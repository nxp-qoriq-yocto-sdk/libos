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

#ifdef CONFIG_LIBOS_LIBC
.global bzero_aligned
	/* OPT 64-bit */
bzero_aligned:
	lis	%r5, cache_block_size@ha
	lwz	%r5, cache_block_size@l(%r5)
	
	addi	%r6, %r5, -1
	subf.	%r7, %r5, %r4
	blt-	bzero_not_cache_aligned

	and.	%r0, %r3, %r6
	bne-	bzero_not_cache_aligned
	
1:	dcbz	0, %r3
	subf.	%r7, %r5, %r7
	add	%r3, %r3, %r5
	bge+	1b

bzero_not_cache_aligned:
	and	%r4, %r4, %r6
	li	%r0, 0
	rlwinm.	%r7, %r4, 30, 2, 31
	mtctr	%r7
	beq-	bzero_no_words

2:	stw	%r0, 0(%r3)
	addi	%r3, %r3, 4
	bdnz	2b

bzero_no_words:
	andi.	%r4, %r4, 3
	mtctr	%r4
	beqlr

3:	stb	%r0, 0(%r3)
	addi	%r3, %r3, 1
	bdnz	3b
	blr

#endif
