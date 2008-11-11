/** @file
 * MPIC interrupt controller driver
 */
/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
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

#include <libos/printlog.h>
#include <libos/mpic.h>
#include <libos/io.h>
#include <libos/core-regs.h>
#include <libos/mpic.h>
#include <libos/errors.h>

typedef struct mpic_interrupt {
	interrupt_t irq;
	mpic_hwirq_t *hw;
} mpic_interrupt_t;

static mpic_interrupt_t mpic_irqs[MPIC_NUM_SRCS];
static uint32_t mpic_lock;
int mpic_coreint;

static inline void mpic_write(uint32_t reg, uint32_t val)
{
	out32(((uint32_t *)(CCSRBAR_VA + MPIC + reg)), val);
}

static inline register_t mpic_read(uint32_t reg)
{
	return in32((uint32_t *)(CCSRBAR_VA + MPIC + reg));
}

static void mpic_eoi(interrupt_t *irq)
{
	mpic_write(MPIC_EOI, 0);
}

uint16_t mpic_iack(void)
{
	return mpic_read(MPIC_IACK);
}

/* Non-critical interrupts only */
void mpic_irq_mask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	
	register_t saved = spin_lock_critsave(&mpic_lock);
	out32(&mirq->hw->vecpri, in32(&mirq->hw->vecpri) | MPIC_IVPR_MASK);
	spin_unlock_critsave(&mpic_lock, saved);
}

/* Non-critical interrupts only */
void mpic_irq_unmask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);

	register_t saved = spin_lock_critsave(&mpic_lock);
	out32(&mirq->hw->vecpri, in32(&mirq->hw->vecpri) & ~MPIC_IVPR_MASK);
	spin_unlock_critsave(&mpic_lock, saved);
}

int mpic_irq_get_mask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return !!(in32(&mirq->hw->vecpri) & MPIC_IVPR_MASK);
}

void mpic_irq_set_vector(interrupt_t *irq, uint32_t vector)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	register_t saved = spin_lock_critsave(&mpic_lock);
	vpr.data = in32(&mirq->hw->vecpri);
	vpr.vector = vector;
	out32(&mirq->hw->vecpri, vpr.data);
	spin_unlock_critsave(&mpic_lock, saved);
}

uint16_t mpic_irq_get_vector(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	vpr.data = in32(&mirq->hw->vecpri);
	return vpr.vector;
}

int mpic_irq_get_enabled(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return !!(in32(&mirq->hw->vecpri) & MPIC_IVPR_MASK);
}

int mpic_irq_get_activity(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return !!(in32(&mirq->hw->vecpri) & MPIC_IVPR_ACTIVE);
}

void mpic_irq_set_ctpr(uint8_t priority)
{
	mpic_write(MPIC_CTPR, priority & 15);
}

int32_t mpic_irq_get_ctpr(void)
{
	 return mpic_read(MPIC_CTPR);
}

void mpic_irq_set_priority(interrupt_t *irq, int priority)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	register_t saved = spin_lock_critsave(&mpic_lock);

	vpr.data = in32(&mirq->hw->vecpri);
	vpr.priority = priority;
	out32(&mirq->hw->vecpri, vpr.data);

	spin_unlock_critsave(&mpic_lock, saved);
}

int mpic_irq_get_priority(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	vpr.data = in32(&mirq->hw->vecpri);
	return vpr.priority;
}

void mpic_irq_set_config(interrupt_t *irq, int config)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	register_t saved = spin_lock_critsave(&mpic_lock);

	vpr.data = in32(&mirq->hw->vecpri);
	vpr.polarity = !!(config & IRQ_HIGH);
	vpr.sense = !!(config & IRQ_LEVEL);
	out32(&mirq->hw->vecpri, vpr.data);

	spin_unlock_critsave(&mpic_lock, saved);
}

int mpic_irq_get_config(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return (in32(&mirq->hw->vecpri) >> MPIC_IVPR_CONFIG_SHIFT) & 3;
}

void mpic_irq_set_destcpu(interrupt_t *irq, uint32_t destcpu)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	out32(&mirq->hw->destcpu, destcpu);
}

uint32_t mpic_irq_get_destcpu(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return in32(&mirq->hw->destcpu);
}

/*
 * sets the IILR (interrupt level reg)
 *
 * int type is defined as
 *   0x0 - normal interrupt
 *   0x1 - critical interrupt
 *   0x2 - machine check
 *
 */

void mpic_irq_set_delivery_type(interrupt_t *irq, int inttype)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	out32(&mirq->hw->intlevel, inttype);
}

int mpic_irq_get_delivery_type(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return in32(&mirq->hw->intlevel);
}

/** Reset active interrupts on core.
 * Call this with all interrupts masked to clear any in-service interrupts.
 */
void mpic_reset_core(void)
{
	int i, vector;
	mpic_irq_set_ctpr(0);

	for (i = 0; i < 1000; i++) {
		/* FIXME: Is this still valid with coreint, or do we need to read EPR? */
		vector = mpic_iack();
		
		if (vector == 0xffff)
			return;

		mpic_eoi(NULL);
	}
	
	printf("mpic_reset_core(): too many interrupts, vector %x\n", vector);
}

static int mpic_register(interrupt_t *irq, int_handler_t handler,
                         void *devid)
{
	irqaction_t *action = alloc_type(irqaction_t);
	if (!action)
		return ERR_NOMEM;
	
	action->handler = handler;
	action->devid = devid;
	
	register_t saved = spin_lock_critsave(&mpic_lock);
	action->next = irq->actions;
	irq->actions = action;
	spin_unlock_critsave(&mpic_lock, saved);

	/* FIXME: only topaz wants critints */
	mpic_irq_set_delivery_type(irq, TYPE_CRIT);
	mpic_irq_unmask(irq);
	return 0;
}

static int __mpic_get_critint(void)
{
	uint32_t summary = mpic_read(MPIC_EXT_CRIT_SUMMARY);
	int i;
	
	if (summary)
		return count_msb_zeroes(summary);
	
	for (i = 0; i < MPIC_NUM_INT_SRCS / 32; i++) {
		summary = mpic_read(MPIC_INT_CRIT_SUMMARY + i * 16);
		
		if (summary)
			return count_msb_zeroes(summary) + i * 32 + 16;
	}
	
	return -1;
}

static interrupt_t *mpic_get_critint(void)
{
	int irqnum = __mpic_get_critint();
	
	if (irqnum >= 0)
		return &mpic_irqs[irqnum].irq;

	return NULL;
}

void do_mpic_critint(void)
{
	interrupt_t *irq;
	
	while ((irq = mpic_get_critint())) {
		/* FIXME: race against unregistration without holding
		 * a global IRQ lock.
		 */

		irqaction_t *action = irq->actions;
		
		while (action) {
			action->handler(action->devid);
			action = action->next;
		}
	}
}

static const uint8_t mpic_intspec_to_config[4] = {
	IRQ_EDGE | IRQ_HIGH,
	IRQ_LEVEL | IRQ_LOW,
	IRQ_LEVEL | IRQ_HIGH,
	IRQ_EDGE | IRQ_LOW
};

static int mpic_config_by_intspec(interrupt_t *irq,
                                  const uint32_t *intspec, int ncells)
{
	if (ncells < 2)
		return ERR_INVALID;
	
	if (intspec[1] > 3)
		return ERR_INVALID;
	
	mpic_irq_set_config(irq, mpic_intspec_to_config[intspec[1]]);
	return 0;
}                                  

int_ops_t mpic_ops = {
	.register_irq = mpic_register,
	.eoi = mpic_eoi,
	.enable = mpic_irq_unmask,
	.disable = mpic_irq_mask,
	.is_disabled = mpic_irq_get_mask,
	.set_cpu_dest_mask = mpic_irq_set_destcpu,
	.get_cpu_dest_mask = mpic_irq_get_destcpu,
	.set_priority = mpic_irq_set_priority,
	.get_priority = mpic_irq_get_priority,
	.set_delivery_type = mpic_irq_set_delivery_type,
	.get_delivery_type = mpic_irq_get_delivery_type,
	.set_config = mpic_irq_set_config,
	.get_config = mpic_irq_get_config,
	.is_active = mpic_irq_get_activity,
	.config_by_intspec = mpic_config_by_intspec,
};

interrupt_t *get_mpic_irq(const uint32_t *irqspec, int ncells)
{
	unsigned int irqnum;
	
	if (ncells < 2)
		return NULL;
	
	irqnum = irqspec[0];
	if (irqnum > MPIC_NUM_SRCS)
		return NULL;
	
	return &mpic_irqs[irqnum].irq;
}

/** Global MPIC initialization routine */
void mpic_init(unsigned long devtree_ptr, int coreint)
{
	int i;
	unsigned int gcr;
	vpr_t vpr;
	mpic_interrupt_t *mirq;

	mpic_coreint = coreint;

	/* Set current processor priority to max */
	mpic_irq_set_ctpr(0xf);

	gcr = mpic_read(MPIC_GCR);

	if (coreint)
		gcr |= GCR_COREINT_DELIVERY_MODE | GCR_MIXED_OPERATING_MODE;

	gcr |= GCR_MIXED_OPERATING_MODE;
	mpic_write(MPIC_GCR, gcr);
	
	for (i = 0; i < MPIC_NUM_SRCS; i++) {
		mirq = &mpic_irqs[i];
		mirq->hw = (mpic_hwirq_t *)(CCSRBAR_VA + MPIC + MPIC_IRQ_BASE);
		mirq->hw += i;
		mirq->irq.ops = &mpic_ops;

		mpic_irq_set_destcpu(&mirq->irq, 1);

		vpr.data = 0;
		vpr.vector = i;
		vpr.msk = 1;
		vpr.polarity = 1;	/* Active High */
		
		if (i < MPIC_NUM_EXT_SRCS)
			vpr.sense = 1;	/* Level Sensitive */

		vpr.priority = 0;

		out32(&mirq->hw->vecpri, vpr.data);
	}

	mpic_reset_core();
}
