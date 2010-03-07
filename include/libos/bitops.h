/*
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_BITOPS_H
#define LIBOS_BITOPS_H

#include <libos/libos.h>
#include <libos/core-regs.h>
#include <libos/io.h>

// Returns non-zero if the operation succeeded.
static inline int compare_and_swap(unsigned long *ptr,
                                   unsigned long old,
                                   unsigned long new)
{
	unsigned long ret;

	// FIXME 64-bit
	asm volatile("1: lwarx %0, %y1;"
	             "cmpw %0, %2;"
	             "bne 2f;"
	             "stwcx. %3, %y1;"
	             "bne 1b;"
	             "2:" :
	             "=&r" (ret), "+Z" (*ptr) :
	             "r" (old), "r" (new) :
	             "memory", "cc");

	return ret == old;
}

// Returns non-zero if the operation succeeded.
// We duplicate this on 32-bit rather than reuse it with a cast
// to avoid violating strict aliasing rules.  I want templates.
static inline int compare_and_swap32(uint32_t *ptr, uint32_t old, uint32_t new)
{
	uint32_t ret;

	asm volatile("1: lwarx %0, %y1;"
	             "cmpw %0, %2;"
	             "bne 2f;"
	             "stwcx. %3, %y1;"
	             "bne 1b;"
	             "2:" :
	             "=&r" (ret), "+Z" (*ptr) :
	             "r" (old), "r" (new) :
	             "memory", "cc");

	return ret == old;
}

static inline int spin_lock_held(uint32_t *ptr)
{
	return *ptr == mfspr_nonvolatile(SPR_PIR) + 1;
}

static inline void raw_spin_lock(uint32_t *ptr)
{
	uint32_t pir = mfspr_nonvolatile(SPR_PIR) + 1;
	uint32_t tmp;

	asm volatile("1: lwarx %0, %y1;"
	             "cmpwi %0, 0;"
	             "bne 2f;"
	             "stwcx. %2, %y1;"
	             "bne 1b;"
	             "lwsync;"
	             ".subsection 1;"
	             "2: lwzx %0, %y1;"
	             "cmpwi %0, 0;"
	             "bne 2b;"
	             "b 1b;"
	             ".previous" :
	             "=&r" (tmp), "+Z" (*ptr) :
	             "r" (pir) :
	             "memory", "cc");
}

static inline void spin_lock(uint32_t *ptr)
{
#ifdef CONFIG_LIBOS_NO_BARE_SPINLOCKS
	assert(!ints_enabled());
#endif

	if (spin_lock_held(ptr)) {
		set_crashing(1);
		printf("Recursive spin_lock detected on %p\n", ptr);
		set_crashing(0);
		BUG();
	}

	raw_spin_lock(ptr);
}

static inline int raw_spin_trylock(uint32_t *ptr)
{
	return compare_and_swap32(ptr, 0, mfspr_nonvolatile(SPR_PIR) + 1);
}

/* Returns non-zero on success, zero if the lock is already held */
static inline int spin_trylock(uint32_t *ptr)
{
#ifdef CONFIG_LIBOS_NO_BARE_SPINLOCKS
	assert(!ints_enabled());
#endif

	return raw_spin_trylock(ptr);
}

static inline void spin_unlock(uint32_t *ptr)
{
	__attribute__((unused)) uint32_t pir = mfspr_nonvolatile(SPR_PIR) + 1;

	assert(*ptr == pir);
	asm volatile("lwsync; stw%U0%X0 %1, %0" : "=m" (*ptr) : "r" (0) : "memory");
}

static inline register_t spin_lock_critsave(uint32_t *ptr)
{
	register_t ret = disable_critint_save();
	spin_lock(ptr);
	return ret;
}

static inline void spin_unlock_critsave(uint32_t *ptr, register_t saved)
{
	spin_unlock(ptr);
	restore_critint(saved);
}

static inline register_t spin_lock_mchksave(uint32_t *ptr)
{
	register_t ret = disable_mchk_save();
	spin_lock(ptr);
	return ret;
}

static inline void spin_unlock_mchksave(uint32_t *ptr, register_t saved)
{
	spin_unlock(ptr);
	restore_mchk(saved);
}

static inline register_t spin_lock_intsave(uint32_t *ptr)
{
	register_t ret = disable_int_save();
	spin_lock(ptr);
	return ret;
}

static inline void spin_unlock_intsave(uint32_t *ptr, register_t saved)
{
	spin_unlock(ptr);
	restore_int(saved);
}

static inline void spin_lock_int(uint32_t *ptr)
{
	assert(ints_enabled());
	disable_int();
	spin_lock(ptr);
}

static inline void spin_unlock_int(uint32_t *ptr)
{
	spin_unlock(ptr);
	enable_int();
}

static inline unsigned long atomic_or(unsigned long *ptr, unsigned long val)
{
	unsigned long ret;

	// FIXME 64-bit
	asm volatile("1: lwarx %0, %y1;"
	             "or %0, %0, %2;"
	             "stwcx. %0, %y1;"
	             "bne 1b;" :
	             "=&r" (ret), "+Z" (*ptr) :
	             "r" (val) :
	             "memory", "cc");

	return ret;
}

static inline unsigned long atomic_and(unsigned long *ptr, unsigned long val)
{
	unsigned long ret;

	// FIXME 64-bit
	asm volatile("1: lwarx %0, %y1;"
	             "and %0, %0, %2;"
	             "stwcx. %0, %y1;"
	             "bne 1b;" :
	             "=&r" (ret), "+Z" (*ptr) :
	             "r" (val) :
	             "memory", "cc");

	return ret;
}

static inline unsigned long atomic_add(unsigned long *ptr, long val)
{
	unsigned long ret;

	// FIXME 64-bit
	asm volatile("1: lwarx %0, %y1;"
	             "add %0, %0, %2;"
	             "stwcx. %0, %y1;"
	             "bne 1b;" :
	             "=&r" (ret), "+Z" (*ptr) :
	             "r" (val) :
	             "memory", "cc");

	return ret;
}

// Undefined if val is zero
static inline int count_msb_zeroes(unsigned long val)
{
	return __builtin_clzl(val);
}

// Undefined if val is zero
static inline int count_lsb_zeroes(unsigned long val)
{
	return __builtin_ctzl(val);
}

// Undefined if val is zero
static inline int ilog2(unsigned long val)
{
	return LONG_BITS - 1 - count_msb_zeroes(val);
}

// Undefined if val is zero
static inline int ilog2_roundup(unsigned long val)
{
	return LONG_BITS - count_msb_zeroes(val - 1);
}

/* function to synchronize a cache block when modifying
 * instructions.  This follows the recommended sequence
 * in the EREF for self modifying code.
 */
static inline void icache_block_sync(char *ptr)
{
	asm volatile("dcbf %y0;"
	             "msync;"
	             "icbi %y0;"
	             "msync;"
	             "isync;" : : "Z" (*ptr) : "memory");
}

static inline void dcache_block_flush(char *ptr)
{
	asm volatile("dcbf %y0; msync" : : "Z" (*ptr) : "memory");
}

#endif
