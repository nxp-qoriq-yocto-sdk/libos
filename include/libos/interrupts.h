#ifndef LIBOS_INTERRUPTS_H
#define LIBOS_INTERRUPTS_H

#include <stdint.h>
#include <libos/bitops.h>

typedef int (*int_handler_t)(void *arg);

/* interrupt delivery type */
#define TYPE_NORM	0x0
#define TYPE_CRIT	0x1
#define TYPE_MCHK	0x2

#define IRQ_LOW   0
#define IRQ_HIGH  1
#define IRQ_EDGE  0
#define IRQ_LEVEL 2

typedef struct irqaction {
	struct irqaction *next;
	int_handler_t handler;
	void *devid;
} irqaction_t;

typedef struct interrupt {
	struct int_ops *ops;
	irqaction_t *actions;
	void *priv;
} interrupt_t;

typedef struct int_ops {
	int (*register_irq)(interrupt_t *irq, int_handler_t handler,
	                    void *devid);
	int (*unregister_irq)(interrupt_t *irq, void *devid);
	void (*eoi)(interrupt_t *irq);
	void (*enable)(interrupt_t *irq);
	void (*disable)(interrupt_t *irq);
	int (*is_enabled)(interrupt_t *irq);
	void (*set_delivery_type)(interrupt_t *irq, int type);
	int (*get_delivery_type)(interrupt_t *irq);
	void (*set_priority)(interrupt_t *irq, int priority);
	void (*set_cpu_dest_mask)(interrupt_t *irq, uint32_t cpu_dest_mask);
	void (*set_config)(interrupt_t *irq, int config);
	int (*get_priority)(interrupt_t *irq);
	int (*get_config)(interrupt_t *irq);
	uint32_t (*get_cpu_dest_mask)(interrupt_t *irq);
	int (*is_active)(interrupt_t *irq);
	int (*config_by_intspec)(interrupt_t *irq, const uint32_t *intspec,
	                         int ncells);
} int_ops_t;

#endif
