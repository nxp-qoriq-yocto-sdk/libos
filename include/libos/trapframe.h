/*
 * Copyright (C) 2007-2009 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_TRAPFRAME_H
#define LIBOS_TRAPFRAME_H

#include <libos/libos.h>
#include <stdint.h>

typedef struct trapframe {
	register_t backchain, lrsave;
	register_t gpregs[32];
	register_t lr, ctr;
	uint32_t cr, xer;
	register_t srr0, srr1, dear, esr;
	uint32_t eplc, epsc;
	unsigned int exc, traplevel;
#ifdef CONFIG_LIBOS_STATISTICS
	int current_event;
	uint32_t initial_cycles;
#endif
	uint32_t pad[4];
} trapframe_t;

/* PPC ABI requires 16-byte-aligned stack frames. */
#define FRAMELEN roundup(sizeof(trapframe_t), 16)

void dump_regs(trapframe_t *regs);
void unknown_exception(trapframe_t *regs);

#endif
