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

#define ASSERT

/* void *memset(void *s, int c, size_t n) */

.global memset
	/* OPT 64-bit */
memset:
	lis	%r9, cache_block_size@ha
	cmpwi	%r4, 0
	lwz	%r9, cache_block_size@l(%r9)
	bne-	memset_byte_tail   // if value != 0 do it by byte

	andi.	%r8, %r3, 3
	bne-	memset_byte_head   // memset the leading bytes
	
again:
	addi	%r6, %r9, -1       // create mask of block size
	subf.	%r7, %r9, %r5
	blt-	memset_word_tail

	and.	%r8, %r3, %r6
	bne-	memset_word_head

#ifdef ASSERT
	and	%r0, %r3, %r6
	twllei	%r5, %r6
	twnei	%r0, 0
#endif

1:	dcbz	0, %r3
	subf.	%r7, %r9, %r7
	add	%r3, %r3, %r9
	bge+	1b

	add.	%r5, %r7, %r9
	beqlr

memset_word_tail:
#ifdef ASSERT
	andi.	%r0, %r3, 3
	twnei	%r0, 0
#endif

	rlwinm.	%r7, %r5, 30, 2, 31
	mtctr	%r7
	beq-	memset_byte_tail

2:	stw	%r4, 0(%r3)
	addi	%r3, %r3, 4
	bdnz	2b

	andi.	%r5, %r5, 3
memset_byte_tail:
	cmpwi	%r5, 0
	mtctr	%r5
	beqlr

3:	stb	%r4, 0(%r3)
	addi	%r3, %r3, 1
	bdnz	3b
	blr

	// r8 = address & 3
	// We can get here with fewer than 3 bytes to set.
memset_byte_head:
#ifdef ASSERT
	twlgti	%r8, 3
#endif
	cmpwi	%r5, 3
	subfic	%r8, %r8, 4
	ble-	memset_byte_tail

	mtctr	%r8
4:	stb	%r4, 0(%r3)
	addi	%r3, %r3, 1
	bdnz	4b

	subf	%r5, %r8, %r5
	b	again

	// r8 = address & (cache block size - 1)
	// We only get here if we'll be setting into the
	// next cache block.
memset_word_head:
#ifdef ASSERT
	andi.	%r0, %r3, 3
	twlgt	%r8, %r6
	twnei	%r0, 0
#endif

	subf	%r8, %r8, %r9
#ifdef ASSERT
	twllt	%r5, %r8
#endif

	rlwinm.	%r7, %r8, 30, 2, 31
	mtctr	%r7

5:	stw	%r4, 0(%r3)
	addi	%r3, %r3, 4
	bdnz	5b

	subf.	%r5, %r8, %r5
	bne	again
	blr
