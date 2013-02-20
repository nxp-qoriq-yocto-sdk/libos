/*-
 * Copyright (C) 2006 Semihalf, Rafal Jaworowski <raj@semihalf.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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

/* IVORs for exeception handling */

#ifndef TRAP_BOOKE_H
#define TRAP_BOOKE_H

#define EXC_CRIT_INT   0    /* critical input */
#define EXC_MCHECK     1    /* machine check, error report */
#define EXC_DSI        2    /* data storage */
#define EXC_ISI        3    /* instruction storage */
#define EXC_EXT_INT    4    /* external input */
#define EXC_ALIGN      5    /* alignment */
#define EXC_PROGRAM    6    /* program-- illegal, privileged, trap */
#define EXC_FPUNAVAIL  7    /* fpu unavailable */
#define EXC_SYSCALL    8    /* system call */
#define EXC_AUXUNAVAIL 9    /* aux proc unavail */
#define EXC_DECR       10   /* decrementer */
#define EXC_FIT        11   /* fixed interval */
#define EXC_WDOG       12   /* watchdog */
#define EXC_DTLB       13   /* data tlb error */
#define EXC_ITLB       14   /* instruction tlb error */
#define EXC_DEBUG      15   /* debug */
#define EXC_ALTIVECUNAVAIL 32  /* altivec unvailable */
#define EXC_ALTIVECASSIST  33  /* altivec assist */
#define EXC_PERFMON    35   /* performance monitor */
#define EXC_DOORBELL   36   /* doorbell */
#define EXC_DOORBELLC  37   /* doorbell critical */
#define EXC_GDOORBELL  38   /* guest doorbell */
#define EXC_GDOORBELLC 39   /* guest doorbell critical, machine check */
#define EXC_HCALL      40   /* hcall */
#define EXC_EHPRIV     41   /* hypervisor privilege trap */
#define EXC_LRAT       42   /* LRAT error */

#define EXC_LAST 255

#ifndef EXC_CRIT_INT_HANDLER
#define EXC_CRIT_INT_HANDLER unknown_exception
#endif
#ifndef EXC_MCHECK_HANDLER
#define EXC_MCHECK_HANDLER unknown_exception
#endif
#ifndef EXC_DSI_HANDLER
#define EXC_DSI_HANDLER unknown_exception
#endif
#ifndef EXC_ISI_HANDLER
#define EXC_ISI_HANDLER unknown_exception
#endif
#ifndef EXC_EXT_INT_HANDLER
#define EXC_EXT_INT_HANDLER unknown_exception
#endif
#ifndef EXC_ALIGN_HANDLER
#define EXC_ALIGN_HANDLER unknown_exception
#endif
#ifndef EXC_PROGRAM_HANDLER
#define EXC_PROGRAM_HANDLER unknown_exception
#endif
#ifndef EXC_FPUNAVAIL_HANDLER
#define EXC_FPUNAVAIL_HANDLER unknown_exception
#endif
#ifndef EXC_SYSCALL_HANDLER
#define EXC_SYSCALL_HANDLER unknown_exception
#endif
#ifndef EXC_AUXUNAVAIL_HANDLER
#define EXC_AUXUNAVAIL_HANDLER unknown_exception
#endif
#ifndef EXC_DECR_HANDLER
#define EXC_DECR_HANDLER unknown_exception
#endif
#ifndef EXC_FIT_HANDLER
#define EXC_FIT_HANDLER unknown_exception
#endif
#ifndef EXC_WDOG_HANDLER
#define EXC_WDOG_HANDLER unknown_exception
#endif
#ifndef EXC_DTLB_HANDLER
#define EXC_DTLB_HANDLER unknown_exception
#endif
#ifndef EXC_ITLB_HANDLER
#define EXC_ITLB_HANDLER unknown_exception
#endif
#ifndef EXC_DEBUG_HANDLER
#define EXC_DEBUG_HANDLER unknown_exception
#endif
#ifndef EXC_ALTIVECUNAVAIL_HANDLER
#define EXC_ALTIVECUNAVAIL_HANDLER unknown_exception
#endif
#ifndef EXC_ALTIVECASSIST_HANDLER
#define EXC_ALTIVECASSIST_HANDLER  unknown_exception
#endif
#ifndef EXC_PERFMON_HANDLER
#define EXC_PERFMON_HANDLER unknown_exception
#endif
#ifndef EXC_DOORBELL_HANDLER
#define EXC_DOORBELL_HANDLER unknown_exception
#endif
#ifndef EXC_DOORBELLC_HANDLER
#define EXC_DOORBELLC_HANDLER unknown_exception
#endif
#ifndef EXC_GDOORBELL_HANDLER
#define EXC_GDOORBELL_HANDLER unknown_exception
#endif
#ifndef EXC_GDOORBELLC_HANDLER
#define EXC_GDOORBELLC_HANDLER unknown_exception
#endif
#ifndef EXC_HCALL_HANDLER
#define EXC_HCALL_HANDLER unknown_exception
#endif
#ifndef EXC_EHPRIV_HANDLER
#define EXC_EHPRIV_HANDLER unknown_exception
#endif
#ifndef EXC_LRAT_HANDLER
#define EXC_LRAT_HANDLER unknown_exception
#endif

#ifndef _ASM
extern void ret_from_exception(void);
extern void ret_from_crit(void);
extern void ret_from_mcheck(void);
extern void ret_from_debug(void);
#endif

#endif
