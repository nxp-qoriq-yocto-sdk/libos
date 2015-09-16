
/*
 * Copyright (C) 2007-2011 Freescale Semiconductor, Inc.
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

#include <libos/libos.h>
#include <libos/trap_booke.h>
#include <libos/trapframe.h>
#include <libos/printlog.h>
#include <libos/core-regs.h>
#include <libos/bitops.h>
#include <libos/io.h>

struct powerpc_exception {
	int vector;
	const char *name;
};

static const struct powerpc_exception powerpc_exceptions[] = {
	{ EXC_CRIT_INT, "critical input" },
	{ EXC_MCHECK, "machine check" },
	{ EXC_DSI, "data storage interrupt" },
	{ EXC_ISI, "instruction storage interrupt" },
	{ EXC_EXT_INT, "external interrupt" },
	{ EXC_ALIGN, "alignment" },
	{ EXC_PROGRAM, "program" },
	{ EXC_SYSCALL, "system call" },
	{ EXC_DECR, "decrementer" },
	{ EXC_FIT, "fixed-interval timer" },
	{ EXC_WDOG, "watchdog timer" },
	{ EXC_DTLB, "data tlb miss" },
	{ EXC_ITLB, "instruction tlb miss" },
	{ EXC_DEBUG, "debug" },
	{ EXC_PERFMON, "performance monitoring" },
	{ EXC_ALTIVECUNAVAIL, "altivec unavailable" },
	{ EXC_ALTIVECASSIST, "altivec assist" },
	{ EXC_DOORBELL, "doorbell"},
	{ EXC_DOORBELLC, "doorbell critical"},
#ifdef HYPERVISOR
	{ EXC_GDOORBELL, "guest doorbell"},
	{ EXC_GDOORBELLC, "guest doorbell critical"},
	{ EXC_HCALL, "hcall"},
	{ EXC_EHPRIV, "ehpriv"},
	{ EXC_LRAT, "lrat miss"},
#endif
	{ EXC_LAST, NULL }
};

static const char *trapname(int vector)
{
	const struct powerpc_exception *pe;

	for (pe = powerpc_exceptions; pe->vector != EXC_LAST; pe++) {
		if (pe->vector == vector)
			return (pe->name);
	}

	return "unknown";
}

static void traceback(trapframe_t *regs)
{
	unsigned long *sp = ((unsigned long *)regs->gpregs[1]);
	printlog(LOGTYPE_MISC, LOGLEVEL_ALWAYS,
	         "sp %p %lx %lx %lx %lx\n", sp, sp[0], sp[1], sp[2], sp[3]);
	sp = ((unsigned long **)regs->gpregs[1])[0];

	printlog(LOGTYPE_MISC, LOGLEVEL_ALWAYS,"Traceback: ");

	for (int i = 1; sp != NULL; i++, sp = (unsigned long *)sp[0]) {
		if ((i % 7) == 0)
			printf("\n");

		printf("0x%08lx ", sp[1] - 4);
	}

	printf("\n");
}

void dump_regs(trapframe_t *regs)
{
	static uint32_t dump_lock;
	register_t saved = 0;
	int lock = 0;

	if (!spin_lock_held(&dump_lock)) {
		lock = 1;
		saved = spin_lock_intsave(&dump_lock);
	}

	printlog(LOGTYPE_MISC, LOGLEVEL_ALWAYS, "%s\n", trapname(regs->exc));

	printlog(LOGTYPE_MISC, LOGLEVEL_ALWAYS,
	         "NIP 0x%08lx MSR 0x%08lx LR 0x%08lx ESR 0x%08lx EXC %d\n"
	         "CTR 0x%08lx CR 0x%08x XER 0x%08x DEAR 0x%08lx PIR %lu\n"
	         "Prev trap level %d\n",
	         regs->srr0, regs->srr1, regs->lr, mfspr(SPR_ESR), regs->exc,
	         regs->ctr, regs->cr, regs->xer, mfspr(SPR_DEAR), mfspr(SPR_PIR),
	         regs->traplevel);

	for (int i = 0; i < 32; i++) {
		printf("r%02d 0x%08lx  ", i, regs->gpregs[i]);

		if ((i & 3) == 3)
			printf("\n");
	}

#ifdef HYPERVISOR
	if (!(regs->srr1 & (MSR_GS | MSR_PR)))
#else
	if (!(regs->srr1 & MSR_PR))
#endif
		traceback(regs);

	if (lock)
		spin_unlock_intsave(&dump_lock, saved);
}

void unknown_exception(trapframe_t *regs)
{
	printlog(LOGTYPE_MISC, LOGLEVEL_ALWAYS,
	         "unknown exception: %s\n", trapname(regs->exc));
	dump_regs(regs); 
	
	stopsim();
}
