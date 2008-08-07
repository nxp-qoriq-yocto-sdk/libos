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

#ifndef LIBOS_PAMU_H
#define LIBOS_PAMU_H

#include <stdint.h>
#include <libos/libos.h>

#define PAMU_TABLE_ALIGNMENT 0x00001000

/* PAMU CCSR space */
#define PAMU_PGC 0x00000000     /* Allows all peripheral accesses */
#define PAMU_PE 0x40000000      /* enable PAMU                    */

/* PAMU_OFFSET to the next pamu space in ccsr */
#define PAMU_OFFSET 0x1000	

#define PAMU_MMAP_REGS_BASE 0

typedef struct pamu_mmap_regs {
	uint32_t ppbah;
	uint32_t ppbal;
	uint32_t pplah;
	uint32_t pplal;
	uint32_t spbah;
	uint32_t spbal;
	uint32_t splah;
	uint32_t splal;
	uint32_t obah;
	uint32_t obal;
	uint32_t olah;
	uint32_t olal;
} pamu_mmap_regs_t; 

/* PAMU Error Registers */
#define PAMU_POES1 0x0040
#define PAMU_POES2 0x0044
#define PAMU_POEAH 0x0048
#define PAMU_POEAL 0x004C
#define PAMU_AVS1  0x0050
#define PAMU_AVS2  0x0054
#define PAMU_AVAH  0x0058
#define PAMU_AVAL  0x005C
#define PAMU_EECTL 0x0060
#define PAMU_EEDIS 0x0064
#define PAMU_EEINTEN 0x0068
#define PAMU_EEDET 0x006C
#define PAMU_EEATTR 0x0070
#define PAMU_EEAHI 0x0074
#define PAMU_EEALO 0x0078
#define PAMU_EEDHI 0X007C
#define PAMU_EEDLO 0x0080
#define PAMU_EECC  0x0084

/* PAMU Revision Registers */
#define PAMU_PR1 0x0BF8
#define PAMU_PR2 0x0BFC

/* PAMU Capabilities Registers */
#define PAMU_PC1 0x0C00
#define PAMU_PC2 0x0C04
#define PAMU_PC3 0x0C08
#define PAMU_PC4 0x0C0C

/* PAMU Control Register */
#define PAMU_PC 0x0C10

/* PAMU control defs */
#define PAMU_CONTROL 0x0C10
#define PAMU_PC_PGC 0x80000000 /* 1 = PAMU Gate Closed : block all 
peripheral access, 0 : may allow peripheral access */

#define PAMU_PC_PE   0x40000000 /* 0 = PAMU disabled, 1 = PAMU enabled */   
#define PAMU_PC_SPCC 0x00000010 /* sPAACE cache enable */
#define PAMU_PC_PPCC 0x00000001 /* pPAACE cache enable */

#define PAMU_PFA1 0x0C14
#define PAMU_PFA2 0x0C18

/* PAMU Interrupt control and Status Register */
#define PAMU_PICS 0x0C1C

/* PAMU Debug Registers */
#define PAMU_PD1 0x0F00
#define PAMU_PD2 0x0F04
#define PAMU_PD3 0x0F08
#define PAMU_PD4 0x0F0C

/* fields defined for PFA1 (PAMU Fetch Attributes 1) */
typedef struct pfa1_t {
	unsigned int pdid   : 8;  /**< PAMU destination id */
	unsigned int psnpid : 8;  /**< PAMU snoop id       */
	unsigned int pfoe   : 8;  /**< PAMU fetch operation encoding */
	unsigned int ppid   : 8;  /**< PAMU partition id   */
} pfa1_t;

#define PAACE_AP_PERMS_DENIED  0x0
#define PAACE_AP_PERMS_QUERY   0x1
#define PAACE_AP_PERMS_UPDATE  0x2
#define PAACE_AP_PERMS_ALL     0x3
#define PAACE_DD_TO_HOST       0x0
#define PAACE_DD_TO_IO         0x1
#define PAACE_PT_PRIMARY       0x0
#define PAACE_PT_SECONDARY     0x1
#define PAACE_V_INVALID        0x0
#define PAACE_V_VALID          0x1
#define PAACE_MW_SUBWINDOWS    0x1

#define PAACE_WSE_4K           0x11
#define PAACE_WSE_8K           0x12
#define PAACE_WSE_16K          0x13
#define PAACE_WSE_32K          0x14
#define PAACE_WSE_64K          0x15
#define PAACE_WSE_128K         0x16
#define PAACE_WSE_256K         0x17
#define PAACE_WSE_512K         0x18
#define PAACE_WSE_1M           0x19
#define PAACE_WSE_2M           0x20
#define PAACE_WSE_4M           0x21
#define PAACE_WSE_8M           0x22
#define PAACE_WSE_16M          0x23
#define PAACE_WSE_32M          0x24
#define PAACE_WSE_64M          0x25
#define PAACE_WSE_128M         0x26
#define PAACE_WSE_256M         0x27
#define PAACE_WSE_512M         0x28
#define PAACE_WSE_1G           0x29
#define PAACE_WSE_2G           0x30
#define PAACE_WSE_4G           0x31

#define PAACE_DID_PCI_EXPRESS_1 0x00
#define PAACE_DID_PCI_EXPRESS_2 0x01
#define PAACE_DID_PCI_EXPRESS_3 0x02
#define PAACE_DID_PCI_EXPRESS_4 0x03
#define PAACE_DID_LOCAL_BUS     0x04
#define PAACE_DID_SRIO          0x0C
#define PAACE_DID_MEM_1         0x10
#define PAACE_DID_MEM_2         0x11
#define PAACE_DID_MEM_3         0x12
#define PAACE_DID_MEM_4         0x13
#define PAACE_DID_MEM_1_2       0x14
#define PAACE_DID_MEM_3_4       0x15
#define PAACE_DID_MEM_1_4       16
#define PAACE_DID_BM_SW_PORTAL  18
#define PAACE_DID_PAMU          0x1C
#define PAACE_DID_CAAM          0x21
#define PAACE_DID_QMAN_1        0x3C
#define PAACE_DID_BROADCAST     0xFF

#define PAACE_ATM_NO_XLATE      0x00
#define PAACE_ATM_WINDOW_XLATE  0x01
#define PAACE_ATM_PAGE_XLATE    0x02
#define PAACE_ATM_WIN_PG_XLATE  \
                ( PAACE_ATM_WINDOW_XLATE | PAACE_ATM_PAGE_XLATE )
#define PAACE_OTM_NO_XLATE      0x00
#define PAACE_OTM_IMMEDIATE     0x01
#define PAACE_OTM_INDEXED       0x02
#define PAACE_OTM_RESERVED      0x03

#define PAACE_M_COHERENCE_REQ   0x01

#define PAACE_PID_0             0x0
#define PAACE_PID_1             0x1
#define PAACE_PID_2             0x2
#define PAACE_PID_3             0x3
#define PAACE_PID_4             0x4
#define PAACE_PID_5             0x5
#define PAACE_PID_6             0x6
#define PAACE_PID_7             0x7

#define PAACE_TCEF_FORMAT0_8B   0x00
#define PAACE_TCEF_FORMAT1_RSVD 0x01

#define PAACE_NUMBER_ENTRIES    0xFF
#define SPAACE_NUMBER_ENTRIES   0x14
#define	OME_NUMBER_ENTRIES      0xFF

/* PAMU Data Structures */

typedef struct ppaace_t {
	/* PAACE Offset 0x00 */
	/* Window Base Address */
	uint32_t        wbah;
	unsigned int    wbal : 20;
	/* Window Size, 2^(N+1), N must be > 10 */
	unsigned int    wse : 6;
	/* 1 Means there are secondary windows, wce is count */
	unsigned int    mw : 1;
	/* Permissions, see PAACE_AP_PERMS_* defines */
	unsigned int    ap : 2;
	/* 
	 * Destination Domain, see PAACE_DD_* defines,
	 * defines data structure reference for ingress ops into 
	 * host/coherency domain or ingress ops into I/O domain
	 */
	unsigned int    dd : 1;
	/* PAACE Type, see PAACE_PT_* defines */
	unsigned int    pt : 1;
	/* PAACE Valid, 0 is invalid */
	unsigned int    v : 1;

	/* PAACE Offset 0x08 */
	/* Interpretation of first 32 bits dependent on DD above */
	union {
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t did;  
			/* Partition ID */
			uint8_t pid;
			/* Snoop ID */
			uint8_t snpid;
			unsigned int coherency_required : 1;
			unsigned int reserved : 7;
		} to_host;
		struct {
			/* Destination ID, see PAACE_DID_* defines */
		} to_io;
	} __attribute__ ((packed)) domain_attr;
	/* Implementation attributes */
	unsigned int impl_attr : 24;
	/* Window Count; 2^(N+1) sub-windows; only valid for primary PAACE */
	unsigned int wce : 4;
	/* Address translation mode, see PAACE_ATM_* defines */
	unsigned int atm : 2;
	/* Operation translation mode, see PAACE_OTM_* defines */
	unsigned int otm : 2;
        
	/* PAACE Offset 0x10 */
	/* Translated window base address */
	uint32_t twbah;
	unsigned int twbal : 20;
	/* Subwindow size encoding; 2^(N+1), N > 10 */
	unsigned int swse : 6;
	unsigned int reserved4 : 6;

	/* PAACE Offset 0x18 */
	uint32_t fspi;
	union {
		struct {
			uint8_t ioea;
			uint8_t moea;
			uint8_t ioeb;
			uint8_t moeb;
		} immed_ot;
		struct {
			uint16_t reserved;
			uint16_t omi;
		} index_ot;
	} __attribute__ ((packed)) op_encode;

	/* PAACE Offset 0x20 */
	uint32_t sbah;
	unsigned int sbal : 20;
	unsigned int sse : 6;
	unsigned int reserved5 : 6;

	/* PAACE Offset 0x28 */
	uint32_t tctbah;
	unsigned int tctbal : 20;
	unsigned int pse : 6;
	unsigned int tcef :1;
	unsigned int reserved6 : 5;

	/* PAACE Offset 0x30 */
	uint32_t reserved7[2];

	/* PAACE Offset 0x38 */
	uint32_t reserved8[2];
} __attribute__ ((packed)) ppaace_t;

typedef struct spaace_t {
	/* PAACE Offset 0x00 */
	uint32_t reserved1;
	uint16_t liodn;
	unsigned int reserved2 : 11;
	/* Permissions, see PAACE_AP_PERMS_* defines */
	unsigned int ap : 2;
	/* Destination Domain, see PAACE_DD_* defines */
	unsigned int dd : 1;
	/* PAACE Type, see PAACE_PT_* defines */
	unsigned int pt : 1;
	
	/* PAACE Valid, 0 is invalid */
	unsigned int v : 1;
	/* PAACE Offset 0x08 */
	/* Interpretation of first 32 bits dependent on DD above */
	union {
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t did;  
			/* Partition ID */
			uint8_t pid;
			/* Snoop ID */
			uint8_t snpid;
			unsigned int coherency_required : 1;
			unsigned int reserved : 7;
		} to_host;
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t did;
			unsigned int reserved : 24;
		} to_io;
	} __attribute__ ((packed)) domain_attr;
	/* Implementation attributes */
	unsigned int impl_attr : 24;
	unsigned int reserved3 : 4;
	/* Address translation mode, see PAACE_ATM_* defines */
	unsigned int atm : 2;
	/* Operation translation mode, see PAACE_OTM_* defines */
	unsigned int otm : 2;
        
	/* PAACE Offset 0x10 */
	/* Translated window base address */
	uint32_t twbah;
	unsigned int twbal : 20;
	/* Subwindow size encoding; 2^(N+1), N > 10 */
	unsigned int swse : 6;
	unsigned int reserved4 : 6;
            
	/* PAACE Offset 0x18 */
	uint32_t reserved5;
	union {
		struct {
			uint8_t ioea;
			uint8_t moea;
			uint8_t ioeb;
			uint8_t moeb;
		}immed_ot;
		struct {
			uint16_t reserved;
			uint16_t omi;
		} index_ot;
	} __attribute__ ((packed)) op_encode;

	/* PAACE Offset 0x20 */
	uint32_t sbah;
	unsigned int sbal : 20;
	unsigned int sse : 6;
	unsigned int reserved6 : 6;

	/* PAACE Offset 0x28 */
	uint32_t tctbah;
	unsigned int tctbal : 20;
	unsigned int pse : 6;
	unsigned int tcef : 1;
	unsigned int reserved7 : 5;

	/* PAACE Offset 0x30 */
	uint32_t reserved8[2];

	/* PAACE Offset 0x38 */
	uint32_t reserved9[2];
} spaace_t;

/* MOE : Mapped Operation Encodings */
#define NUM_MOE 128
typedef struct ome_t {
	uint8_t moe[NUM_MOE];
} __attribute__((packed)) ome_t;

int pamu_hw_init(unsigned long pamu_reg_base, unsigned long pamu_reg_size);
ppaace_t *get_ppaace(uint32_t liodn);
void setup_default_xfer_to_host_ppaace(ppaace_t *ppaace);

#endif  /* __PAMU_H */
