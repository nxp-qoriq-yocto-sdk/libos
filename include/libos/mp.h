/** @file
 * Hardware threads API
 *
 * Author: Laurentiu Tudor <Laurentiu.Tudor@freescale.com>
 *
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_MP_H
#define LIBOS_MP_H

#include <libos/core-regs.h>
#include <libos/percpu.h>
#include <libos/cpu_caps.h>

static inline void mttmr(int reg, register_t val)
{
	asm volatile(".long 0x7c0003dc | "
			"(((%0 & 0x1f) << 16) | ((%0 & 0x3e0) << 6)) | "
			"(%1 << 21)" : : "i" (reg), "r" (val) : "memory");
}

static inline register_t mftmr(int reg)
{
	register_t ret;

	asm volatile(".long 0x7c0002dc | "
			"(((%1 & 0x1f) << 16) | ((%1 & 0x3e0) << 6)) | "
			"(%0 << 21)" : "=r" (ret) : "i" (reg) : "memory");

	return ret;
}

#if CONFIG_LIBOS_MAX_HW_THREADS > 1

/** Get the id of the thread currently running on
 *
 * @return: unsigned holding the current thread id
 */
static inline unsigned get_hw_thread_id(void)
{
	if (!cpu_has_ftr(CPU_FTR_THREADS))
		return 0;

	return mfspr(SPR_TIR);
}

/** Start a the specified hardware thread on the core currently running on
 *
 * @param[in] tid id of the thread to start. Must not be the id of the current thread
 * @param[in] msr initial value of MSR that the thread will start with.
 * @param[in] cpu pointer to the cpu_t structure of the newly started thread.
 * 		  Structure must be accessible (mapped in shared TLB1) in the
 * 		  starting thread and the stack in cpu->kstack must be
 * 		  initialized and pointing to stack[KSTACK_SIZE - FRAMELEN].
 */
static inline void start_hw_thread(unsigned int tid, uint32_t msr, cpu_t *cpu)
{
	void hw_thread_start(void);
	extern cpu_t *hw_thread_percpus[MAX_CORES * CONFIG_LIBOS_MAX_HW_THREADS];

	assert(tid != get_hw_thread_id());
	assert(tid < cpu_caps.threads_per_core);

	if (tid == 1) {
		mttmr(TMR_IMSR1, msr);
		mttmr(TMR_INIA1, (register_t)&hw_thread_start);
		hw_thread_percpus[mfspr(SPR_PIR) + 1] = cpu;
	} else if (tid == 0) {
		mttmr(TMR_IMSR0, msr);
		mttmr(TMR_INIA0, (register_t)&hw_thread_start);
		hw_thread_percpus[mfspr(SPR_PIR) - 1] = cpu;
	} else {
		assert(0);
	}
	mtspr(SPR_TENS, 1 << tid);
}

#else

static inline int get_hw_thread_id(void)
{
	return 0;
}

static inline void start_hw_thread(unsigned int tid, uint32_t msr, cpu_t *cpu)
{
	assert(0);
}

#endif

#endif
