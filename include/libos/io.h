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

#ifndef LIBOS_IO_H
#define LIBOS_IO_H

#include <libos/types.h>
#include <libos/core-regs.h>

static inline void mtspr(int reg, register_t val)
{
	asm volatile("mtspr %0, %1" : : "i" (reg), "r" (val) : "memory");
}

static inline register_t mfspr(int reg)
{
	register_t ret;
	asm volatile("mfspr %0, %1" : "=r" (ret) : "i" (reg) : "memory");
	return ret;
}

// Use this version when the compiler may combine multiple calls.
static inline register_t mfspr_nonvolatile(int reg)
{
	register_t ret;
	asm("mfspr %0, %1" : "=r" (ret) : "i" (reg));
	return ret;
}

static inline void mtpmr(int reg, register_t val)
{
	asm volatile("mtpmr %0, %1" : : "i" (reg), "r" (val) : "memory");
}

static inline register_t mfpmr(int reg)
{
	register_t ret;
	asm volatile("mfpmr %0, %1" : "=r" (ret) : "i" (reg) : "memory");
	return ret;
}

static inline void mtmsr(register_t val)
{
	asm volatile("mtmsr %0" : : "r" (val) : "memory");
}

static inline register_t mfmsr(void)
{
	register_t ret;
	asm volatile("mfmsr %0" : "=r" (ret) :  : "memory");
	return ret;
}

static inline void mtfsf(uint64_t val)
{
	register_t msr;
	uint64_t f = 0;
	msr = mfmsr();
	mtmsr(msr | MSR_FP);
	asm volatile("stfd 0, %[f];"
	             "lfd 0, %[val];"
	             "mtfsf 0xff, 0;"
	             "lfd 0, %[f];" : [f] "+m" (f) : [val] "m" (val));
	mtmsr(msr);
}

static inline uint64_t mffs(void)
{
	uint64_t ret = 0;
	register_t msr;
	uint64_t f = 0;
	msr = mfmsr();
	mtmsr(msr | MSR_FP);
	asm volatile("stfd 0, %[f];"
	             "mffs 0;"
	             "stfd 0, %[ret];"
	             "lfd 0, %[f];" : [f] "+m" (f), [ret] "=m" (ret));
	mtmsr(msr);
	return ret;
}

static inline void barrier(void)
{
	asm volatile("" : : : "memory");
}

static inline void isync(void)
{
	asm volatile("isync" : : : "memory");
}

static inline void sync(void)
{
	asm volatile("sync" : : : "memory");
}

static inline void lwsync(void)
{
	asm volatile("lwsync" : : : "memory");
}

static inline void mbar(int mo)
{
	asm volatile("mbar %0" : : "i" (mo) : "memory");
}

static inline void smp_mbar(void)
{
	// FIXME: ifdef SMP
	mbar(1);
}

static inline void smp_lwsync(void)
{
	// FIXME: ifdef SMP
	lwsync();
}

static inline void smp_sync(void)
{
	// FIXME: ifdef SMP
	sync();
}

static inline void tlb_inv_addr(register_t vaddr)
{
	asm volatile("tlbilxva 0, %0" : : "r" (vaddr));
}

static inline void tlb_inv_pid(void)
{
	asm volatile("tlbilxpid");
}

static inline void tlb_inv_lpid(void)
{
	asm volatile("tlbilxlpid");
}

static inline register_t disable_critint_save(void)
{
	register_t ret = mfmsr();
	mtmsr(ret & ~(MSR_CE | MSR_EE));
	return ret;
}

static inline void restore_critint(register_t saved)
{
	mtmsr(saved);
}

#ifdef CONFIG_LIBOS_CRITICAL_INTS
static inline register_t disable_int_save(void)
{
	return disable_critint_save();
}

static inline void restore_int(register_t saved)
{
	restore_critint(saved);
}

static inline void disable_int(void)
{
	mtmsr(mfmsr() & ~(MSR_CE | MSR_EE));
}

static inline void enable_int(void)
{
	mtmsr(mfmsr() | MSR_CE | MSR_EE);
}
#else
static inline register_t disable_int_save(void)
{
	register_t ret;
	asm volatile("mfmsr %0; wrteei 0" : "=r" (ret) : : "memory");
	return ret;
}

static inline void restore_int(register_t saved)
{
	asm volatile("wrtee %0" : : "r" (saved) : "memory");
}

static inline void disable_int(void)
{
	asm volatile("wrteei 0" : : : "memory");
}

static inline void enable_int(void)
{
	asm volatile("wrteei 1" : : : "memory");
}
#endif

/* Note: unlike disable_critint_save, does *not* disable MSR[EE]. */
static inline void disable_critint(void)
{
	mtmsr(mfmsr() & ~MSR_CE);
}

static inline void enable_critint(void)
{
	mtmsr(mfmsr() | MSR_CE);
}

static inline void disable_mcheck(void)
{
	mtmsr(mfmsr() & ~MSR_ME);
}

static inline void enable_mcheck(void)
{
	mtmsr(mfmsr() | MSR_ME);
}

/* Deprecated legacy names -- "external" is a bad name,
 * regardless of what the architecture calls it.
 */
#define disable_extint disable_int
#define enable_extint enable_int

static inline uint8_t raw_in8(const uint8_t *ptr)
{
	uint8_t ret;
	asm volatile("lbz%U1%X1 %0, %1" : "=r" (ret) : "m" (*ptr) : "memory");
	return ret;
}

static inline uint16_t raw_in16(const uint16_t *ptr)
{
	uint16_t ret;
	asm volatile("lhz%U1%X1 %0, %1" : "=r" (ret) : "m" (*ptr) : "memory");
	return ret;
}

static inline uint32_t raw_in32(const uint32_t *ptr)
{
	uint32_t ret;
	asm volatile("lwz%U1%X1 %0, %1" : "=r" (ret) : "m" (*ptr) : "memory");
	return ret;
}

static inline void raw_out8(uint8_t *ptr, uint8_t val)
{
	asm volatile("stb%U0%X0 %1, %0" : "=m" (*ptr) : "r" (val) : "memory");
}

static inline void raw_out16(uint16_t *ptr, uint16_t val)
{
	asm volatile("sth%U0%X0 %1, %0" : "=m" (*ptr) : "r" (val) : "memory");
}

static inline void raw_out32(uint32_t *ptr, uint32_t val)
{
	asm volatile("stw%U0%X0 %1, %0" : "=m" (*ptr) : "r" (val) : "memory");
}

static inline uint16_t raw_in16_rev(const uint16_t *ptr)
{
	uint16_t ret;
	asm volatile("lhbrx %0, %y1" : "=r" (ret) : "Z" (*ptr) : "memory");
	return ret;
}

static inline uint32_t raw_in32_rev(const uint32_t *ptr)
{
	uint32_t ret;
	asm volatile("lwbrx %0, %y1" : "=r" (ret) : "Z" (*ptr) : "memory");
	return ret;
}

static inline void raw_out16_rev(uint16_t *ptr, uint16_t val)
{
	asm volatile("sthbrx %1, %y0" : "=Z" (*ptr) : "r" (val) : "memory");
}

static inline void raw_out32_rev(uint32_t *ptr, uint32_t val)
{
	asm volatile("stwbrx %1, %y0" : "=Z" (*ptr) : "r" (val) : "memory");
}

#ifdef _BIG_ENDIAN
#define raw_in16_le raw_in16_rev
#define raw_in32_le raw_in32_rev
#define raw_out16_le raw_out16_rev
#define raw_out32_le raw_out32_rev
#define raw_in16_be raw_in16
#define raw_in32_be raw_in32
#define raw_out16_be raw_out16
#define raw_out32_be raw_out32
#elif defined(_LITTLE_ENDIAN)
#define raw_in16_be raw_in16_rev
#define raw_in32_be raw_in32_rev
#define raw_out16_be raw_out16_rev
#define raw_out32_be raw_out32_rev
#define raw_in16_le raw_in16
#define raw_in32_le raw_in32
#define raw_out16_le raw_out16
#define raw_out32_le raw_out32
#endif

#define IO_DEF_IN(name, type) \
static inline type name(const type *ptr) \
{ \
	type ret; \
	sync(); \
	ret = raw_##name(ptr); \
	sync(); \
	return ret; \
}

#define IO_DEF_OUT(name, type) \
static inline void name(type *ptr, type val) \
{ \
	sync(); \
	raw_##name(ptr, val); \
	sync(); \
}

IO_DEF_IN(in8, uint8_t)
IO_DEF_IN(in16, uint16_t)
IO_DEF_IN(in16_rev, uint16_t)
IO_DEF_IN(in16_be, uint16_t)
IO_DEF_IN(in16_le, uint16_t)
IO_DEF_IN(in32, uint32_t)
IO_DEF_IN(in32_rev, uint32_t)
IO_DEF_IN(in32_be, uint32_t)
IO_DEF_IN(in32_le, uint32_t)
IO_DEF_OUT(out8, uint8_t) 
IO_DEF_OUT(out16, uint16_t)
IO_DEF_OUT(out16_rev, uint16_t)
IO_DEF_OUT(out16_be, uint16_t)
IO_DEF_OUT(out16_le, uint16_t)
IO_DEF_OUT(out32, uint32_t)
IO_DEF_OUT(out32_rev, uint32_t)
IO_DEF_OUT(out32_be, uint32_t)
IO_DEF_OUT(out32_le, uint32_t)

#endif
