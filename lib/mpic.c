/** @file
 * MPIC interrupt controller driver
 */

/*
 * Copyright (C) 2008-2010 Freescale Semiconductor, Inc.
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
#include <libos/alloc.h>
#include <libos/errors.h>

typedef struct mpic_interrupt {
	interrupt_t irq;
	mpic_hwirq_t *hw;
	msi_hwirq_t *msi;
	uint32_t msi_reg;
	ipi_hwirq_t ipi;
	error_interrupt_t err;
	int config_done; /**< indicates that config of this int is done */
} mpic_interrupt_t;

static mpic_interrupt_t mpic_irqs[MPIC_NUM_SRCS];
static mpic_interrupt_t mpic_ipi_irqs[MPIC_NUM_IPI_SRCS];
static uint32_t mpic_lock, error_int_lock;
static error_sub_int_t error_subints[MPIC_NUM_ERR_SRCS];

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
static void mpic_irq_mask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	
	register_t saved = spin_lock_intsave(&mpic_lock);
	out32(&mirq->hw->vecpri, in32(&mirq->hw->vecpri) | MPIC_IVPR_MASK);
	spin_unlock_intsave(&mpic_lock, saved);
}

/* Non-critical interrupts only */
static void mpic_irq_unmask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);

	register_t saved = spin_lock_intsave(&mpic_lock);
	out32(&mirq->hw->vecpri, in32(&mirq->hw->vecpri) & ~MPIC_IVPR_MASK);
	spin_unlock_intsave(&mpic_lock, saved);
}

static int mpic_irq_get_mask(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return !!(in32(&mirq->hw->vecpri) & MPIC_IVPR_MASK);
}

void mpic_irq_set_vector(interrupt_t *irq, uint32_t vector)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	register_t saved = spin_lock_intsave(&mpic_lock);
	vpr.data = in32(&mirq->hw->vecpri);
	vpr.vector = vector;
	out32(&mirq->hw->vecpri, vpr.data);
	spin_unlock_intsave(&mpic_lock, saved);
}

uint16_t mpic_irq_get_vector(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	vpr.data = in32(&mirq->hw->vecpri);
	return vpr.vector;
}

static int mpic_irq_get_activity(interrupt_t *irq)
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

static void mpic_irq_set_priority(interrupt_t *irq, int priority)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	register_t saved = spin_lock_intsave(&mpic_lock);

	vpr.data = in32(&mirq->hw->vecpri);
	vpr.priority = priority;
	out32(&mirq->hw->vecpri, vpr.data);

	spin_unlock_intsave(&mpic_lock, saved);
}

static int mpic_irq_get_priority(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	vpr.data = in32(&mirq->hw->vecpri);
	return vpr.priority;
}

static void __mpic_irq_set_config(interrupt_t *irq, int config)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	vpr_t vpr;

	vpr.data = in32(&mirq->hw->vecpri);
	vpr.polarity = !!(config & IRQ_HIGH);
	vpr.sense = !!(config & IRQ_LEVEL);
	out32(&mirq->hw->vecpri, vpr.data);
}

static void mpic_irq_set_config(interrupt_t *irq, int config)
{
	register_t saved = spin_lock_intsave(&mpic_lock);
	__mpic_irq_set_config(irq, config);
	spin_unlock_intsave(&mpic_lock, saved);
}

static int mpic_irq_get_config(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return (in32(&mirq->hw->vecpri) >> MPIC_IVPR_CONFIG_SHIFT) & 3;
}

static void mpic_irq_set_destcpu(interrupt_t *irq, uint32_t destcpu)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	out32(&mirq->hw->destcpu, destcpu);
}

static uint32_t mpic_irq_get_destcpu(interrupt_t *irq)
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

static void mpic_irq_set_delivery_type(interrupt_t *irq, int inttype)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	out32(&mirq->hw->intlevel, inttype);
}

static int mpic_irq_get_delivery_type(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return in32(&mirq->hw->intlevel);
}

static uint32_t mpic_msi_get_msir(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	return in32(&mirq->msi->msir[mirq->msi_reg].msira);
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

static int error_int_p4080_rev1(void *arg)
{
	interrupt_t *irq = arg;

	set_crashing(1);
	printf("Shared error interrupt received, EISR0: %#x\n",
	       in32((uint32_t *)(CCSRBAR_VA + MPIC + MPIC_ERROR_INT_SUMMARY)));
	printf("Future error interrupts will be disabled "
	       "due to p4080 rev 1 limitations.\n");
	set_crashing(0);

	mpic_irq_mask(irq);
	mpic_irq_set_delivery_type(irq, TYPE_NORM);

	return 0;
}

static int mpic_register(interrupt_t *irq, int_handler_t handler,
                         void *devid, int flags)
{
	irqaction_t *action = alloc_type(irqaction_t);
	if (!action)
		return ERR_NOMEM;

#ifdef TOPAZ
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);

	/* On p4080 rev 1, don't allow handlers to be registered, but
	 * instead just print EISR0 and disable the interrupt.  This
	 * avoids problems with interrupt storms on error sources that
	 * can't be cleared.
	 */
	if (mfspr(SPR_SVR) == P4080REV1 && mirq == &mpic_irqs[16]) {
		register_t saved = spin_lock_intsave(&mpic_lock);

		if (!irq->actions) {
			action->handler = error_int_p4080_rev1;
			action->devid = irq;

			irq->actions = action;
			spin_unlock_intsave(&mpic_lock, saved);

			mpic_irq_set_delivery_type(irq, flags);
			mpic_irq_unmask(irq);
		} else {
			spin_unlock_intsave(&mpic_lock, saved);
			free(action);
		}

		return ERR_BUSY;
	}
#endif

	
	action->handler = handler;
	action->devid = devid;
	
	register_t saved = spin_lock_intsave(&mpic_lock);
	action->next = irq->actions;
	irq->actions = action;
	spin_unlock_intsave(&mpic_lock, saved);

	/* FIXME: only topaz wants critints */
	mpic_irq_set_delivery_type(irq, flags);
	mpic_irq_unmask(irq);
	return 0;
}

static void call_irq_handler(interrupt_t *irq)
{
	irqaction_t *action = irq->actions;

	assert(action);

	while (action) {
		action->handler(action->devid);
		action = action->next;
	}
}

static int get_internal_int(uint32_t reg)
{
	for (int i = 0; i < MPIC_NUM_INT_SRCS / 32; i++) {
		uint32_t summary = mpic_read(reg + i * 16);
		
		if (summary)
			return count_msb_zeroes(summary) + i * 32 + 16;
	}
	
	return -1;
}

static inline int get_ext_int(uint32_t reg)
{
	uint32_t summary = mpic_read(reg);

	return summary ? count_msb_zeroes(summary) : -1;
}

static int __mpic_get_critint(void)
{
	int ret = get_ext_int(MPIC_EXT_CRIT_SUMMARY);

	if (ret >= 0)
		return ret;

	return get_internal_int(MPIC_INT_CRIT_SUMMARY );
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
		call_irq_handler(irq);
	}
}

static int __mpic_get_mcheckint(void)
{
	int ret = get_ext_int(MPIC_EXT_MCHECK_SUMMARY);

	if (ret >= 0)
		return ret;

	return  get_internal_int(MPIC_INT_MCHECK_SUMMARY);
}

static interrupt_t *mpic_get_mcheckint(void)
{
	int irqnum = __mpic_get_mcheckint();

	if (irqnum >= 0)
		return &mpic_irqs[irqnum].irq;

	return NULL;
}

void do_mpic_mcheck(void)
{
	interrupt_t *irq;

	while ((irq = mpic_get_mcheckint())) {
		/* FIXME: race against unregistration without holding
		 * a global IRQ lock.
		 */
		call_irq_handler(irq);
	}
}

static const uint8_t mpic_intspec_to_config[4] = {
	IRQ_EDGE | IRQ_HIGH,
	IRQ_LEVEL | IRQ_LOW,
	IRQ_LEVEL | IRQ_HIGH,
	IRQ_EDGE | IRQ_LOW
};

static interrupt_t *get_mpic_irq(device_t *dev,
                                 const uint32_t *irqspec,
                                 int ncells);

int_ops_t mpic_ops = {
	.get_irq = get_mpic_irq,
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
};

int_ops_t mpic_msi_ops = {
	.get_irq = get_mpic_irq,
	.register_irq = mpic_register,
	.eoi = mpic_eoi,
	.enable = mpic_irq_unmask,
	.disable = mpic_irq_mask,
	.is_disabled = mpic_irq_get_mask,
	.set_cpu_dest_mask = mpic_irq_set_destcpu,
	.get_cpu_dest_mask = mpic_irq_get_destcpu,
	.set_priority = mpic_irq_set_priority,
	.get_priority = mpic_irq_get_priority,
	.set_config = mpic_irq_set_config,
	.get_config = mpic_irq_get_config,
	.is_active = mpic_irq_get_activity,
	.get_msir = mpic_msi_get_msir,
};

static int error_int_handler(void *arg)
{
	error_interrupt_t *err = arg;
	uint32_t val = in32(err->eisr0);
	int i;

	while (val) {
		i = count_lsb_zeroes(val);
		/* The irq handler must clear the condition by masking the corresponding
		 * EIMR0 bit.
		 */
		call_irq_handler(&error_subints[31 - i].dev_err_irq);
		val &= ~(1 << i);
	}

	return 0;
}

static int error_int_get_mask(interrupt_t *irq)
{
	assert(irq->parent);

	error_sub_int_t *err = to_container(irq, error_sub_int_t, dev_err_irq);
	mpic_interrupt_t *mirq = to_container(irq->parent, mpic_interrupt_t, irq);

	return !!(in32(mirq->err.eimr0) & (0x80000000 >> err->subintnum));
}

static void error_int_mask(interrupt_t *irq)
{
	assert(irq->parent);

	error_sub_int_t *err = to_container(irq, error_sub_int_t, dev_err_irq);
	mpic_interrupt_t *mirq = to_container(irq->parent, mpic_interrupt_t, irq);

	register_t saved = spin_lock_mchksave(&error_int_lock);
	out32(mirq->err.eimr0, in32(mirq->err.eimr0) | (0x80000000 >> err->subintnum));
	spin_unlock_mchksave(&error_int_lock, saved);
}

static void error_int_unmask(interrupt_t *irq)
{
	assert(irq->parent);

	error_sub_int_t *err = to_container(irq, error_sub_int_t, dev_err_irq);
	mpic_interrupt_t *mirq = to_container(irq->parent, mpic_interrupt_t, irq);

	register_t saved = spin_lock_mchksave(&error_int_lock);
	out32(mirq->err.eimr0, in32(mirq->err.eimr0) & ~(0x80000000 >> err->subintnum));
	spin_unlock_mchksave(&error_int_lock, saved);
}

static int error_int_register(interrupt_t *irq, int_handler_t handler,
                         void *devid, int flags)
{
	assert(irq->parent);

	/* Check if the irq handler for the mpic int0 has been registered */
	register_t saved = spin_lock_intsave(&error_int_lock);
	if (!irq->parent->actions) {
		error_interrupt_t *err = &(to_container(irq->parent, mpic_interrupt_t, irq))->err;
		irq->parent->ops->register_irq(irq->parent, error_int_handler,
						err, TYPE_MCHK);
	}

	irqaction_t *action = alloc_type(irqaction_t);
	if (!action) {
		spin_unlock_intsave(&error_int_lock, saved);
		return ERR_NOMEM;
	}

	action->handler = handler;
	action->devid = devid;

	action->next = irq->actions;
	irq->actions = action;
	spin_unlock_intsave(&error_int_lock, saved);

	error_int_unmask(irq);

	return 0;
}

int_ops_t error_int_ops = {
	.register_irq = error_int_register,
	.enable = error_int_unmask,
	.disable = error_int_mask,
	.is_disabled = error_int_get_mask,
};

static void ipi_irq_set_destcpu(interrupt_t *irq, uint32_t destcpu)
{
	register_t saved = spin_lock_intsave(&mpic_lock);
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	mirq->ipi.dispatch_cpu_mask |= destcpu;
	spin_unlock_intsave(&mpic_lock, saved);
}

void mpic_set_ipi_dispatch_register(interrupt_t *irq)
{
	mpic_interrupt_t *mirq = to_container(irq, mpic_interrupt_t, irq);
	out32(mirq->ipi.dr, mirq->ipi.dispatch_cpu_mask);
}

int_ops_t mpic_ipi_ops = {
	.eoi = mpic_eoi,
	.enable = mpic_irq_unmask,
	.disable = mpic_irq_mask,
	.is_disabled = mpic_irq_get_mask,
	.set_cpu_dest_mask = ipi_irq_set_destcpu,
	.set_priority = mpic_irq_set_priority,
	.get_priority = mpic_irq_get_priority,
	.is_active = mpic_irq_get_activity,
};

interrupt_t *mpic_get_ipi_irq(int irq)
{
	mpic_interrupt_t *ipi = &mpic_ipi_irqs[irq];
	return &ipi->irq;
}

static void error_int_init(mpic_interrupt_t *mirq)
{
	mirq->err.eisr0 = (uint32_t *)(CCSRBAR_VA + MPIC + MPIC_ERROR_INT_SUMMARY);
	mirq->err.eimr0 = (uint32_t *)(CCSRBAR_VA + MPIC + MPIC_ERROR_INT_MASK);

	/* At reset all the interrupts would be unmasked, so we mask them here */
	out32(mirq->err.eimr0, ~0);

	for (int i = 0; i < MPIC_NUM_ERR_SRCS; i++) {
		error_subints[i].dev_err_irq.ops = &error_int_ops;
		error_subints[i].dev_err_irq.parent = &mirq->irq;
		interrupt_reset(&error_subints[i].dev_err_irq);
	}
}

static interrupt_t *get_mpic_irq(device_t *dev,
                                 const uint32_t *intspec,
                                 int ncells)
{
	unsigned int irqnum;
	mpic_interrupt_t *mirq;
	interrupt_t *irq = NULL;

	assert(dev->irqctrl == &mpic_ops);
	
	if (ncells < 2) {
		printlog(LOGTYPE_IRQ, LOGLEVEL_ERROR,
		         "%s: bad intspec\n", __func__);
		return NULL;
	}

	if (intspec[1] > 3) {
		printlog(LOGTYPE_IRQ, LOGLEVEL_ERROR,
		         "%s: bad intspec\n", __func__);
		return NULL;
	}
	
	irqnum = intspec[0];
	if (irqnum > MPIC_NUM_SRCS)
		return NULL;

	mirq = &mpic_irqs[irqnum];

	if ((ncells == 4) && !(mfspr(SPR_SVR) == P4080REV1)) {
		switch (intspec[2]) {
		case MPIC_DEV_INT:
			irq = &mirq->irq;
			break;

		case MPIC_ERR_INT: {
			uint32_t subint = intspec[3];
			if (subint >= MPIC_NUM_ERR_SRCS) {
				printlog(LOGTYPE_IRQ, LOGLEVEL_ERROR, "Invalid sub interrupt\n");
			}

			if (!mirq->config_done)
				error_int_init(mirq);

			error_sub_int_t *err = &error_subints[subint];
			err->dev_err_irq.config = mpic_intspec_to_config[intspec[1]];
			err->subintnum = subint;
			irq = &err->dev_err_irq;
			break;
		}

		default:
			printlog(LOGTYPE_IRQ, LOGLEVEL_ERROR, "Unhandled interrupt type %d\n", intspec[2]);
			return irq;
		}
	} else {
		irq = &mirq->irq;
	}

	register_t saved = spin_lock_intsave(&mpic_lock);
	if (!mirq->config_done) {
		mirq->irq.config = mpic_intspec_to_config[intspec[1]] |
					 IRQ_TYPE_MPIC_DIRECT;
		__mpic_irq_set_config(&mirq->irq, mirq->irq.config);
		mirq->config_done = 1;
	}
	spin_unlock_intsave(&mpic_lock, saved);

	return irq;
}

/** Global MPIC initialization routine */
void mpic_init(int coreint)
{
	int i, j;
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
	
	/* First, init all external and internal interrupt sources */
	for (i = 0; i < MPIC_NUM_EXTINT_SRCS; i++) {
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

		interrupt_reset(&mirq->irq);

		out32(&mirq->hw->vecpri, vpr.data);
	}

	/* Next, init MSI interrupt sources */
	for (i = 0, j = MPIC_MSI_SRCS_START_OFFSET;
		i < MPIC_NUM_MSI_SRCS; i++, j++) {
		mirq = &mpic_irqs[j];
		mirq->hw = (mpic_hwirq_t *)(CCSRBAR_VA + MPIC + MPIC_IRQ_BASE);
		mirq->hw += j;
		mirq->msi = (msi_hwirq_t *)(CCSRBAR_VA + MPIC + MSI_INT_BASE);
		mirq->msi += (i / MPIC_NUM_REGS_MSI_BANK);
		mirq->msi_reg = (i % MPIC_NUM_REGS_MSI_BANK);
		mirq->irq.ops = &mpic_msi_ops;

		mpic_irq_set_destcpu(&mirq->irq, 1);

		vpr.data = 0;
		vpr.vector = j;
		vpr.msk = 1;
		vpr.priority = 0;

		interrupt_reset(&mirq->irq);

		out32(&mirq->hw->vecpri, vpr.data);
	}

	/* IPI interrupt sources */
	uint32_t *ipivpr;
	for (i = 0; i < MPIC_NUM_IPI_SRCS; i++) {
		ipivpr = (uint32_t *)(CCSRBAR_VA + MPIC + MPIC_IPIVPR_BASE);
		ipivpr += i * MPIC_IPIVPR_OFFSET / sizeof(uint32_t);

		mirq = &mpic_ipi_irqs[i];
		mirq->hw = (mpic_hwirq_t *)ipivpr;
		mirq->ipi.dr = (uint32_t *)(CCSRBAR_VA + MPIC +
							MPIC_IPIDR_BASE);
		mirq->ipi.dr += i * MPIC_IPIDR_OFFSET / sizeof(uint32_t);
		mirq->ipi.dispatch_cpu_mask = 0;
		mirq->irq.ops = &mpic_ipi_ops;

		vpr.data = 0;
		vpr.msk = 1;
		vpr.priority = 0;

		interrupt_reset(&mirq->irq);

		out32(&mirq->hw->vecpri, vpr.data);
	}

	mpic_reset_core();
}
