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
#ifndef MPIC_H
#define MPIC_H

#include <libos/interrupts.h>

#define MPIC 0x40000

#define MPIC_CTPR 0x0080
#define MPIC_EOI  0x00B0
#define MPIC_IACK 0x00A0
#define MPIC_GCR  0x1020

#define MPIC_EXT_CRIT_SUMMARY 0x3b00
#define MPIC_INT_CRIT_SUMMARY 0x3b40

#define MPIC_IRQ_BASE 0x10000

#define MPIC_IVPR_MASK     0x80000000
#define MPIC_IVPR_ACTIVE   0x40000000
#define MPIC_IVPR_POLARITY 0x00800000
#define MPIC_IVPR_SENSE    0x00400000
#define MPIC_IVPR_CONFIG_SHIFT 22

typedef union {
	uint32_t data;
	struct {
		uint32_t msk:1;
		uint32_t active:1;
		uint32_t reserved1:6;
		uint32_t polarity:1;
		uint32_t sense:1;
		uint32_t reserved2:2;
		uint32_t priority:4;
		uint32_t vector:16;
	};
} vpr_t;

typedef struct mpic_hwirq {
	uint32_t vecpri;
	uint32_t reserved[3];
	uint32_t destcpu;
	uint32_t reserved2;
	uint32_t intlevel;
	uint32_t reserved3;
} mpic_hwirq_t;

#define MPIC_NUM_EXT_SRCS	12
#define MPIC_NUM_INT_SRCS	128
#define MPIC_NUM_SRCS (MPIC_NUM_INT_SRCS + 16)
#define MPIC_INT_SRCS_START_OFFSET 16

#define GCR_COREINT_DELIVERY_MODE	0x40000000
#define GCR_MIXED_OPERATING_MODE	0x20000000

void mpic_init(int coreint);
void mpic_irq_set_vector(interrupt_t *irq, uint32_t vector);
uint16_t mpic_irq_get_vector(interrupt_t *irq);
void mpic_irq_set_ctpr(uint8_t priority);
int32_t mpic_irq_get_ctpr(void);
uint16_t mpic_iack(void);
void mpic_reset_core(void);

#define MPIC_EXTERNAL_BASE  0
#define MPIC_INTERNAL_BASE  16

interrupt_t *get_mpic_irq(const uint32_t *irqspec, int ncells);
void do_mpic_critint(void);
extern int mpic_coreint;

#endif
