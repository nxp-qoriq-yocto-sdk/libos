/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
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
#ifndef LIBOS_PERCPU_H
#define LIBOS_PERCPU_H

/*
 * Per-cpu data structures
 */

#ifndef _ASM
#include <stdint.h>
#include <libos/libos.h>
#include <libos/fsl-booke-tlb.h>
#endif

#define CPUSAVE_LEN 2
#define KSTACK_SIZE 2048

#define TLB1_SIZE 64

#ifndef _ASM
typedef uint8_t kstack_t[KSTACK_SIZE] __attribute__((aligned(16)));

typedef struct {
	client_cpu_t client;
	register_t normsave[CPUSAVE_LEN];
	register_t critsave[CPUSAVE_LEN];
	register_t machksave[CPUSAVE_LEN];
	register_t dbgsave[CPUSAVE_LEN];
	tlb_entry_t tlb1[TLB1_SIZE];
	uint8_t *kstack; // Set to stack[KSTACK_SIZE - FRAMELEN];
	kstack_t debugstack, critstack, mcheckstack;
	int coreid, console_ok, crashing;
	int traplevel; /**< Normally 0, 1 if in critical exception context */
	int errno; /**< Used for C/POSIX funcitons that set errno */
#ifdef LIBOS_RET_USER_HOOK
	int ret_user_hook;
#endif
} cpu_t;

register cpu_t *cpu asm("r2");

// Returns 0 on success, -1 on error (e.g. PIR mismatch in table)

struct boot_spin_table {
	unsigned long addr_hi;
	unsigned long addr_lo;
	unsigned long r3_hi;
	unsigned long r3_lo;
	unsigned long reserved;
	unsigned long pir;
	unsigned long r6_hi;
	unsigned long r6_lo;
};

typedef void (*entry_t)(void);
int start_secondary_spin_table(struct boot_spin_table *table, int num,
                               cpu_t *cpu);

#endif

#endif
