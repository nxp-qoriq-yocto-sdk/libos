/*!
 *
 *  @file mpic.c
 *
 *  (c) Copyright Freescale 2007, All Rights Reserved
 *
 */


#include <libos/console.h>
#include <libos/mpic.h>
#include <libos/io.h>
#include <libos/spr.h>
#include <libos/8578.h>
#include <libos/mpic.h>

static inline void mpic_write(uint32_t reg, uint32_t val)
{
	out32(((uint32_t *)(CCSRBAR_VA+MPIC+reg)),val);
}

static inline register_t mpic_read(uint32_t reg)
{
	return in32((uint32_t *)(CCSRBAR_VA+MPIC+reg));
}

/*
 * Global MPIC initialization routine
 */

void mpic_init(unsigned long devtree_ptr)
{
	int i;
	unsigned int gcr;
	vpr_t vpr;

	for (i = 0; i < MPIC_NUM_EXT_SRCS; i++) {
		mpic_irq_mask(i);
	}

	for (i = 0; i < MPIC_NUM_INT_SRCS; i++) {
		mpic_irq_mask(i+MPIC_INT_SRCS_START_OFFSET);
	}

	/* Set current processor priority to max */
	mpic_irq_set_ctpr(0xf);

	gcr = mpic_read(GCR);

#if 0	// FIXME : CoreInt mode is currently disabled for simics testing
	gcr |= GCR_COREINT_DELIVERY_MODE | GCR_MIXED_OPERATING_MODE;
#endif

	gcr |= GCR_MIXED_OPERATING_MODE;
	mpic_write(GCR, gcr);

	/*
	 * Initially set all interrupt sources to be directed to
	 * core 0, also setup interrupt vector and keep interrupts masked
	 */
	for (i = 0; i < MPIC_NUM_EXT_SRCS; i++) {
		mpic_irq_set_destcpu(i, 0);
		vpr.data = 0;
		vpr.eivpr.vector = i;
		vpr.eivpr.msk = 1;
		vpr.eivpr.polarity = 1;	/* Active High */
		vpr.eivpr.sense = 1;	/* Level Sensitive */
		vpr.eivpr.priority = 0;
		mpic_write(MPIC_IRQ_BASE+(i*IRQ_STRIDE)+IIVPR,vpr.data);
	}

	for (i = 0; i < MPIC_NUM_INT_SRCS; i++) {
		mpic_irq_set_destcpu((i+MPIC_INT_SRCS_START_OFFSET), 0);
		vpr.data = 0;
		vpr.iivpr.vector = (i+MPIC_INT_SRCS_START_OFFSET);
		vpr.iivpr.msk = 1;
		vpr.iivpr.polarity = 1;	/* Active High */
		vpr.iivpr.priority = 0;
		mpic_write(MPIC_IRQ_BASE+
			((i+MPIC_INT_SRCS_START_OFFSET)*IRQ_STRIDE)+IIVPR,
			vpr.data);
	}

	/* Set current processor priority to 0 */
	mpic_irq_set_ctpr(0);
	mpic_reset_core();
}

void mpic_eoi(void)
{
	mpic_write(EOI, 0);
}

uint16_t mpic_iack(void)
{
	return mpic_read(IACK);
}

void mpic_irq_mask(int irq)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	vpr.iivpr.msk = 1; /* mask */
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR,vpr.data);
}

void mpic_irq_unmask(int irq)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	vpr.iivpr.msk = 0; /* unmask */
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR,vpr.data);
}

void mpic_irq_set_vector(int irq, uint32_t vector)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	vpr.iivpr.vector = vector;
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR,vpr.data);
}

uint32_t mpic_irq_get_vector(int irq)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	return vpr.iivpr.vector;
}

void mpic_irq_set_ctpr(uint8_t priority)
{
	 mpic_write(CTPR,priority&0xF);
}

int32_t mpic_irq_get_ctpr(void)
{
	 return mpic_read(CTPR);
}

void mpic_irq_set_priority(int irq, uint8_t priority)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	vpr.iivpr.priority = priority;
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR,vpr.data);
}

uint8_t mpic_irq_get_priority(int irq)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	return vpr.iivpr.priority;
}

void mpic_irq_set_polarity(int irq, uint8_t polarity)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	vpr.iivpr.polarity = polarity;
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR,vpr.data);
}

uint8_t mpic_irq_get_polarity(int irq)
{
	vpr_t vpr;
	vpr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIVPR);
	return vpr.iivpr.polarity;
}

void mpic_irq_set_destcpu(int irq, uint8_t destcpu)
{
	uint32_t iidr = 1 << destcpu;
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIDR,iidr);
}

uint8_t mpic_irq_get_destcpu(int irq)
{
	uint32_t iidr;
	iidr = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IIDR);
	return iidr;
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

void mpic_irq_set_inttype(int irq, uint8_t inttype)
{
	iilr_t iilr;
	iilr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IILR);
	iilr.inttgt = inttype;
	mpic_write(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IILR,iilr.data);
}

uint8_t mpic_irq_get_inttype(int irq)
{
	iilr_t iilr;
	iilr.data = mpic_read(MPIC_IRQ_BASE+(irq*IRQ_STRIDE)+IILR);
	return iilr.inttgt;
}

/** Reset active interrupts on core.
 * Call this with all interrupts masked to clear any in-service interrupts.
 */
void mpic_reset_core(void)
{
	int i, vector;

	for (i = 0; i < 1000; i++) {
		/* Is this still valid with coreint, or do we need to read EPR? */
		vector = mpic_iack();
		
		if (vector == 0xffff)
			return;

		mpic_eoi();
	}
	
	printf("mpic_reset_core(): too many interrupts, vector %x\n", vector);
}
