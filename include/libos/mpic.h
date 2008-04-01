#ifndef MPIC_H
#define MPIC_H

#define MPIC_NUM_EXT_SRCS	12
#define MPIC_NUM_INT_SRCS	128
#define MPIC_INT_SRCS_START_OFFSET 16

#define GCR_COREINT_DELIVERY_MODE	0x40000000
#define GCR_MIXED_OPERATING_MODE	0x20000000

void mpic_init(unsigned long devtree_ptr);
void mpic_irq_mask(int irq);
void mpic_irq_unmask(int irq);
void mpic_irq_set_priority(int irq, uint8_t priority);
void mpic_irq_set_vector(int irq, uint32_t vector);
uint32_t mpic_irq_get_vector(int irq);
uint8_t mpic_irq_get_priority(int irq);
void mpic_irq_set_polarity(int irq, uint8_t polarity);
uint8_t mpic_irq_get_polarity(int irq);
void mpic_irq_set_inttype(int irq, uint8_t inttype);
uint8_t mpic_irq_get_inttype(int irq);
void mpic_irq_set_ctpr(uint8_t priority);
int32_t mpic_irq_get_ctpr(void);
void mpic_eoi(void);
uint16_t mpic_iack(void);
void mpic_irq_set_destcpu(int irq, uint8_t destcpu);
void mpic_reset_core(void);

#endif
