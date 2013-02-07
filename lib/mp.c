/*
 * Multiprocessing support
 *
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc.
 * Author: Scott Wood <scottwood@freescale.com>
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

#include <libos/percpu.h>
#include <libos/io.h>
#include <libos/printlog.h>
#include <libos/cache.h>

extern int secondary_start;

int start_secondary_spin_table(struct boot_spin_table *table, int num,
                               cpu_t *newcpu)
{
	phys_addr_t entry = virt_to_phys(&secondary_start);

	printlog(LOGTYPE_MP, LOGLEVEL_DEBUG,
	         "table %p addr %x pir %x entry %llx\n",
	         table, table->addr_lo, table->pir, entry);

#ifdef CONFIG_LIBOS_64BIT
	/*
	 * NOTE:
	 * HV spin table and trampoline code supports filling and restoring
	 * the entire 64-bit value of R3, as demanded by ePAPR for 64-bit
	 * chip implementation (regardless of MSR[CM] setting).
	 * U-Boot did not and may not support in the future full 64-bit R3.
	 * So, running these HV unit tests under U-Boot must be done while
	 * making sure the PHYSBASE is set in the 32-bit adress range, unlike
	 * running them under HV, where PHYSBASE is set in the 64-bit space.
	 */
	table->r3_hi = (uint32_t)((uintptr_t)newcpu >> 32);
#endif
	table->r3_lo = (uint32_t)(uintptr_t)newcpu;
	table->pir = num;

	out32((uint32_t *)&table->addr_hi, entry >> 32);
	out32((uint32_t *)&table->addr_lo, (uint32_t)entry);

	return 0;
}
