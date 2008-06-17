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

static inline int spin_lock_held(uint32_t *ptr)
{
	return *ptr == mfspr_nonvolatile(SPR_PIR) + 1;
}

static inline void spin_lock(uint32_t *ptr)
{
	uint32_t pir = mfspr_nonvolatile(SPR_PIR) + 1;
	uint32_t tmp;

	if (spin_lock_held(ptr)) {
		if (crashing)
			return;
		
		crashing = 1;
		printf("Recursive spin_lock detected on %p\n", ptr);
		BUG();
	}

	asm volatile("1: lwarx %0, %y1;"
	             "cmpwi %0, 0;"
	             "bne 2f;"
	             "stwcx. %2, %y1;"
	             "bne 1b;"
	             "mbar 1;"
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

static inline void spin_unlock(uint32_t *ptr)
{
	__attribute__((unused)) uint32_t pir = mfspr(SPR_PIR) + 1;

	assert(*ptr == pir);
	asm volatile("mbar 1; stw%U0%X0 %1, %0" : "=m" (*ptr) : "r" (0) : "memory");
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

static inline int count_msb_zeroes(unsigned long val)
{
	int ret;

	// FIXME 64-bit
	asm("cntlzw %0, %1" : "=r" (ret) : "r" (val));
	return ret;
}

static inline int count_lsb_zeroes(unsigned long val)
{
	return LONG_BITS - 1 - count_msb_zeroes(val & ~(val - 1));
}

#endif
