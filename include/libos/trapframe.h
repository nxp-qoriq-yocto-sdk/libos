#ifndef LIBOS_TRAPFRAME_H
#define LIBOS_TRAPFRAME_H

#include <libos/libos.h>
#include <stdint.h>

typedef struct {
	register_t backchain, lrsave;
	register_t gpregs[32];
	register_t lr, ctr;
	uint32_t cr, xer;
	register_t srr0, srr1, dear, esr;
	uint32_t eplc, epsc;
	int exc;
	uint32_t pad[4];
} trapframe_t;

/* PPC ABI requires 16-byte-aligned stack frames. */
#define FRAMELEN roundup(sizeof(trapframe_t), 16)

void dump_regs(trapframe_t *regs);
void reflect_trap(trapframe_t *regs);

#endif
