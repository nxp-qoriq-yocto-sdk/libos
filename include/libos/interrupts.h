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

#define IRQ_TYPE_MPIC_DIRECT 4

typedef struct irqaction {
	struct irqaction *next;
	int_handler_t handler;
	void *devid;
} irqaction_t;

typedef struct interrupt {
	struct int_ops *ops;
	irqaction_t *actions;
	int config;
	void *priv;
} interrupt_t;

typedef struct int_ops {
	interrupt_t *(*get_irq)(device_t *dev,
	                        const uint32_t *intspec,
	                        int ncells);
	int (*register_irq)(interrupt_t *irq, int_handler_t handler,
	                    void *devid, int flags);
	int (*unregister_irq)(interrupt_t *irq, void *devid);
	void (*eoi)(interrupt_t *irq);
	void (*enable)(interrupt_t *irq);
	void (*disable)(interrupt_t *irq);
	int (*is_disabled)(interrupt_t *irq);
	void (*set_delivery_type)(interrupt_t *irq, int type);
	int (*get_delivery_type)(interrupt_t *irq);
	void (*set_priority)(interrupt_t *irq, int priority);
	void (*set_cpu_dest_mask)(interrupt_t *irq, uint32_t cpu_dest_mask);
	void (*set_config)(interrupt_t *irq, int config);
	int (*get_priority)(interrupt_t *irq);
	int (*get_config)(interrupt_t *irq);
	uint32_t (*get_cpu_dest_mask)(interrupt_t *irq);
	int (*is_active)(interrupt_t *irq);
} int_ops_t;

#endif
