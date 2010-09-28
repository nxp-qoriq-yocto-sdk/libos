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

#include <libos/fsl-booke-tlb.h>
#include <libos/errors.h>
#include <libos/bitops.h>

int hcall_opcode(void);

#ifdef CONFIG_LIBOS_HCALL_INSTRUCTIONS
int setup_hcall_instructions(uint8_t *opcodes, size_t len)
{
	uint32_t *hcall_addr = (uint32_t *)hcall_opcode;

	/* ePAPR defines up to 4 words of instructions */
	if (len > sizeof(uint32_t) * 4 || len % sizeof(uint32_t))
		return ERR_INVALID;

	for (int i = 0; i < len / sizeof(uint32_t); i++)
		hcall_addr[i] = *((uint32_t *)opcodes + i);

	/* Flush data cache and update the Icache of all CPUs */
	icache_block_sync((char *)hcall_addr);

	return 0;
}
#endif
