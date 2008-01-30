
#include <libos/libos.h>
#include <libos/trap_booke.h>
#include <libos/trapframe.h>
#include <libos/console.h>
#include <libos/spr.h>

struct powerpc_exception {
	int vector;
	char *name;
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
	{ EXC_DOORBELL, "doorbell"},
	{ EXC_DOORBELLC, "doorbell critical"},
	{ EXC_GDOORBELL, "guest doorbell"},
	{ EXC_GDOORBELLC, "guest doorbell critical"},
	{ EXC_HCALL, "hcall"},
	{ EXC_EHPRIV, "ehpriv"},
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

void traceback(trapframe_t *regs)
{
	unsigned long *sp = ((unsigned long *)regs->gpregs[1]);
	printf("sp %p %lx %lx %lx %lx\n", sp, sp[0], sp[1], sp[2], sp[3]);
	sp = ((unsigned long **)regs->gpregs[1])[0];

	printf("Traceback: ");

	for (int i = 1; sp != NULL; i++, sp = (unsigned long *)sp[0]) {
		if ((i & 7) == 0)
			printf("\n");

		printf("0x%08lx ", sp[1] - 4);
	}

	printf("\n");
}

void dump_regs(trapframe_t *regs)
{
	printf("%s\n", trapname(regs->exc));

	printf("NIP 0x%08lx MSR 0x%08lx LR 0x%08lx ESR 0x%08lx EXC %d\n"
	       "CTR 0x%08lx CR 0x%08x XER 0x%08x DEAR 0x%08lx PIR %lu\n",
	       regs->srr0, regs->srr1, regs->lr, mfspr(SPR_ESR), regs->exc,
	       regs->ctr, regs->cr, regs->xer, mfspr(SPR_DEAR), mfspr(SPR_PIR));

	for (int i = 0; i < 32; i++) {
		printf("r%02d 0x%08lx  ", i, regs->gpregs[i]);

		if ((i & 3) == 3)
			printf("\n");
	}

	if (!(regs->srr1 & MSR_GS))
		traceback(regs);
}

void unknown_exception(trapframe_t *regs)
{
	printf("unknown exception: %s\n", trapname(regs->exc));
	dump_regs(regs); 
	
	stopsim();
}
