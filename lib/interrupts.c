/*
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
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

#include <libos/interrupts.h>

void interrupt_reset(interrupt_t *irq)
{
	register_t save = spin_lock_mchksave(&irq->lock);
	irq->maskcnt = 1;
	irq->oldmask = 1;
	irq->ops->disable(irq);
	spin_unlock_mchksave(&irq->lock, save);
}

void interrupt_mask(interrupt_t *irq)
{
	register_t save = spin_lock_mchksave(&irq->lock);
	if (!irq->maskcnt++)
		irq->ops->disable(irq);
	spin_unlock_mchksave(&irq->lock, save);
}

void interrupt_unmask(interrupt_t *irq)
{
	register_t save = spin_lock_mchksave(&irq->lock);

	assert(irq->maskcnt > 0);

	if(!--irq->maskcnt)
		irq->ops->enable(irq);

	spin_unlock_mchksave(&irq->lock, save);
}
