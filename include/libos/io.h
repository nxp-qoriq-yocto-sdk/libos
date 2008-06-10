#ifndef LIBOS_IO_H
#define LIBOS_IO_H

#include <stdint.h>
#include <libos/core-regs.h>

#include <libos/libos.h>
#include <stdint.h>

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

static inline register_t disable_critint_save(void)
{
	register_t ret, tmp;
	asm volatile("mfmsr %0; and %1, %0, %2; mtmsr %1" :
	             "=&r" (ret), "=&r" (tmp) : "r" (~(MSR_CE | MSR_EE)) :
	             "memory");
	return ret;
}

static inline void restore_critint(register_t saved)
{
	mtmsr(saved);
}

/* Note: unlike disable_critint_save, does *not* disable MSR[EE]. */
static inline void disable_critint(void)
{
	mtmsr(mfmsr() & ~MSR_CE);
}

static inline void enable_critint(void)
{
	mtmsr(mfmsr() | MSR_CE);
}

static inline void disable_extint(void)
{
	asm volatile("wrteei 0" : : : "memory");
}

static inline void enable_extint(void)
{
	asm volatile("wrteei 1" : : : "memory");
}

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
	asm volatile("stb%U1%X1 %0, %1" : : "r" (val), "m" (*ptr) : "memory");
}

static inline void raw_out16(uint16_t *ptr, uint16_t val)
{
	asm volatile("sth%U1%X1 %0, %1" : : "r" (val), "m" (*ptr) : "memory");
}

static inline void raw_out32(uint32_t *ptr, uint32_t val)
{
	asm volatile("stw%U1%X1 %0, %1" : : "r" (val), "m" (*ptr) : "memory");
}

static inline uint16_t raw_in16_rev(const uint16_t *ptr)
{
	uint16_t ret;
	asm volatile("lhbrx %0, 0, %1" : "=r" (ret) : "r" (ptr) : "memory");
	return ret;
}

static inline uint32_t raw_in32_rev(const uint32_t *ptr)
{
	uint32_t ret;
	asm volatile("lwbrx %0, 0, %1" : "=r" (ret) : "r" (ptr) : "memory");
	return ret;
}

static inline void raw_out16_rev(uint16_t *ptr, uint16_t val)
{
	asm volatile("sthbrx %0, 0, %1" : : "r" (val), "r" (ptr) : "memory");
}

static inline void raw_out32_rev(uint32_t *ptr, uint32_t val)
{
	asm volatile("stwbrx %0, 0, %1" : : "r" (val), "r" (ptr) : "memory");
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
	mbar(1); \
	return raw_##name(ptr); \
}

#define IO_DEF_OUT(name, type) \
static inline void name(type *ptr, type val) \
{ \
	mbar(1); \
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
