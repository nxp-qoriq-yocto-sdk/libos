
#ifndef PLAT_H
#define PLAT_H

/* MPIC defs */
#define MPIC 0x40000
#define CTPR 0x0080
#define GCR 0x1020
#define MPIC_IRQ_BASE 0x10000
#define IRQ_STRIDE 0x20
#define IIVPR 0x0
#define IIDR 0x10
#define IILR 0x18

/* interrupt type-- IILR */
#define TYPE_NORM	0x0
#define TYPE_CRIT	0x1
#define TYPE_MCHK	0x2

/* IRQ definitions */
#define DUART1_IRQ 0x24
#define DUART2_IRQ 0x25

typedef union {
	uint32_t data;
	struct {
		uint32_t msk:1;
		uint32_t A:1;
		uint32_t reserved1:6;
		uint32_t P:1;
		uint32_t reserved2:3;
		uint32_t priority:4;
		uint32_t vector:16;
	};
} iivpr_t;

typedef union {
	uint32_t data;
	struct {
		uint32_t reserved:24;
		uint32_t inttgt:8;
	};
} iilr_t;

#endif
