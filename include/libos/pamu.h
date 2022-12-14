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

#define PAMU_IDX(x)     ((x) >> 12)                 /* x = PAMU offset from base */
#define PAMU_BYP_BIT(x) (0x80000000 >> PAMU_IDX(x)) /* x = PAMU index -- valid x: 0-15 */

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
#define PAMU_POEAH 0x0048
#define PAMU_POEAL 0x004C
#define PAMU_AVS1  0x0050
#define PAMU_AVS1_AV    0x1
#define PAMU_AVS1_OTV   0x6
#define PAMU_AVS1_APV   0x78
#define PAMU_AVS1_WAV   0x380
#define PAMU_AVS1_LAV   0x1c00
#define PAMU_AVS1_GCV   0x2000
#define PAMU_AVS1_PDV   0x4000
#define PAMU_AV_MASK    (PAMU_AVS1_AV | PAMU_AVS1_OTV | PAMU_AVS1_APV | PAMU_AVS1_WAV \
			| PAMU_AVS1_LAV | PAMU_AVS1_GCV | PAMU_AVS1_PDV)
#define PAMU_AVS1_LIODN_SHIFT 16
#define PAMU_LAV_LIODN_NOT_IN_PPAACT 0x400

#define PAMU_POES1_POED 0x1

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

#define PAMU_SB_ECC_ERR 0x4
#define PAMU_MB_ECC_ERR 0x8
#define PAMU_ECC_ERR_MASK (PAMU_SB_ECC_ERR | PAMU_MB_ECC_ERR)

#define PAMU_ECC_ERR_ATTR_VAL 0x1

#define PAMU_EECTL_CNT_MASK 0xff
#define PAMU_EECTL_THR_SHIFT 16

typedef struct pamu_ecc_err_reg {
	uint32_t eccctl;
	uint32_t eccdis;
	uint32_t eccinten;
	uint32_t eccdet;
	uint32_t eccattr;
	uint32_t eccaddrhi;
	uint32_t eccaddrlo;
	uint32_t eccdatahi;
	uint32_t eccdatalo;
} pamu_ecc_err_reg_t;

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
#define PAMU_PC_OCE  0x00001000 /* OMT cache enable */

#define PAMU_PC3_MWCE(X) (((X) >> 21) & 0xf)

#define PAMU_PFA1 0x0C14
#define PAMU_PFA2 0x0C18

typedef enum pamu_ints {
	pamu_int_operation,
	pamu_int_singlebit,
	pamu_int_multibit,
	pamu_int_av,
	num_pamu_ints,
} pamu_ints_t;

/* PAMU Interrupt control and Status Register */
#define PAMU_PICS 0x0C1C
#define PAMU_ACCESS_VIOLATION_STAT   0x8
#define PAMU_ACCESS_VIOLATION_ENABLE 0x4
#define PAMU_OPERATION_ERROR_INT_STAT 0x2
#define PAMU_OPERATION_ERROR_INT_ENABLE 0x1

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

#define PAACE_WSE_4K           0xB
#define PAACE_WSE_8K           0xC
#define PAACE_WSE_16K          0xD
#define PAACE_WSE_32K          0xE
#define PAACE_WSE_64K          0xF
#define PAACE_WSE_128K         0x10
#define PAACE_WSE_256K         0x11
#define PAACE_WSE_512K         0x12
#define PAACE_WSE_1M           0x13
#define PAACE_WSE_2M           0x14
#define PAACE_WSE_4M           0x15
#define PAACE_WSE_8M           0x16
#define PAACE_WSE_16M          0x17
#define PAACE_WSE_32M          0x18
#define PAACE_WSE_64M          0x19
#define PAACE_WSE_128M         0x1A
#define PAACE_WSE_256M         0x1B
#define PAACE_WSE_512M         0x1C
#define PAACE_WSE_1G           0x1D
#define PAACE_WSE_2G           0x1E
#define PAACE_WSE_4G           0x1F

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
#define PAACE_DID_MEM_1_4       0x16
#define PAACE_DID_BM_SW_PORTAL  0x18
#define PAACE_DID_PAMU          0x1C
#define PAACE_DID_CAAM          0x21
#define PAACE_DID_QM_SW_PORTAL  0x3C
#define PAACE_DID_CORE0_INST    0x80
#define PAACE_DID_CORE0_DATA    0x81
#define PAACE_DID_CORE1_INST    0x82
#define PAACE_DID_CORE1_DATA    0x83
#define PAACE_DID_CORE2_INST    0x84
#define PAACE_DID_CORE2_DATA    0x85
#define PAACE_DID_CORE3_INST    0x86
#define PAACE_DID_CORE3_DATA    0x87
#define PAACE_DID_CORE4_INST    0x88
#define PAACE_DID_CORE4_DATA    0x89
#define PAACE_DID_CORE5_INST    0x8A
#define PAACE_DID_CORE5_DATA    0x8B
#define PAACE_DID_CORE6_INST    0x8C
#define PAACE_DID_CORE6_DATA    0x8D
#define PAACE_DID_CORE7_INST    0x8E
#define PAACE_DID_CORE7_DATA    0x8F
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

#define PAACE_NUMBER_ENTRIES    0x500
/*
 * SPAACT table size assumption : 8 devices * 8 partitions * 256 subwindows * 2
 */
#define SPAACE_NUMBER_ENTRIES   0x8000
#define	OME_NUMBER_ENTRIES      16   /* based on P4080 2.0 silicon plan */

/* PAACE Bit Field Defines */
#define PPAACE_AF_WBAL			0xfffff000
#define PPAACE_AF_WBAL_SHIFT		12
#define PPAACE_AF_WSE			0x00000fc0
#define PPAACE_AF_WSE_SHIFT		6
#define PPAACE_AF_MW			0x00000020
#define PPAACE_AF_MW_SHIFT		5

#define SPAACE_AF_LIODN			0xffff0000
#define SPAACE_AF_LIODN_SHIFT		16

#define PAACE_AF_AP			0x00000018
#define PAACE_AF_AP_SHIFT		3
#define PAACE_AF_DD			0x00000004
#define PAACE_AF_DD_SHIFT		2
#define PAACE_AF_PT			0x00000002
#define PAACE_AF_PT_SHIFT		1
#define PAACE_AF_V			0x00000001
#define PAACE_AF_V_SHIFT		0

#define PAACE_DA_HOST_CR		0x80
#define PAACE_DA_HOST_CR_SHIFT		7

#define PAACE_IA_CID			0x00FF0000
#define PAACE_IA_CID_SHIFT		16
#define PAACE_IA_WCE			0x000000F0
#define PAACE_IA_WCE_SHIFT		4
#define PAACE_IA_ATM			0x0000000C
#define PAACE_IA_ATM_SHIFT		2
#define PAACE_IA_OTM			0x00000003
#define PAACE_IA_OTM_SHIFT		0

#define PAACE_WIN_TWBAL			0xfffff000
#define PAACE_WIN_TWBAL_SHIFT		12
#define PAACE_WIN_SWSE			0x00000fc0
#define PAACE_WIN_SWSE_SHIFT		6

/* PAMU Data Structures */
/* primary / secondary paact structure */
typedef struct paace_t {
	/* PAACE Offset 0x00 */
	uint32_t wbah;				/* only valid for Primary PAACE */
	uint32_t addr_bitfields;		/* See P/S PAACE_AF_* */

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
			/* coherency_required : 1 reserved : 7 */
			uint8_t coherency_required; /* See PAACE_DA_* */
		} to_host;
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t  did; 
			uint8_t  reserved1;
			uint16_t reserved2;
		} to_io;
	} domain_attr;

	/* Implementation attributes + window count + address & operation translation modes */
	uint32_t impl_attr;			/* See PAACE_IA_* */

	/* PAACE Offset 0x10 */
	/* Translated window base address */
	uint32_t twbah;
	uint32_t win_bitfields;			/* See PAACE_WIN_* */

	/* PAACE Offset 0x18 */
	/* first secondary paace entry */
	uint32_t fspi;				/* only valid for Primary PAACE */
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
	} op_encode;

	/* PAACE Offset 0x20 */
	uint32_t reserved1[2];			/* not currently implemented */

	/* PAACE Offset 0x28 */
	uint32_t reserved2[2];			/* not currently implemented */

	/* PAACE Offset 0x30 */
	uint32_t reserved3[2];			/* not currently implemented */

	/* PAACE Offset 0x38 */
	uint32_t reserved4[2];			/* not currently implemented */

} paace_t;

/* MOE : Mapped Operation Encodings */
#define NUM_MOE 128
typedef struct ome_t {
	uint8_t moe[NUM_MOE];
} __attribute__((packed)) ome_t;

#define PPAACT_SIZE             (sizeof(paace_t) * PAACE_NUMBER_ENTRIES)
#define SPAACT_SIZE             (sizeof(paace_t) * SPAACE_NUMBER_ENTRIES)
#define OMT_SIZE                (sizeof(ome_t) * OME_NUMBER_ENTRIES)
#define PAMU_PAGE_SHIFT 12
#define PAMU_PAGE_SIZE  4096U

#define IOE_READ        0x00
#define IOE_READ_IDX    0x00
#define IOE_WRITE       0x81
#define IOE_WRITE_IDX   0x01
#define IOE_EREAD0      0x82    /* Enhanced read type 0 */
#define IOE_EREAD0_IDX  0x02    /* Enhanced read type 0 */
#define IOE_EWRITE0     0x83    /* Enhanced write type 0 */
#define IOE_EWRITE0_IDX 0x03    /* Enhanced write type 0 */
#define IOE_DIRECT0     0x84    /* Directive type 0 */
#define IOE_DIRECT0_IDX 0x04    /* Directive type 0 */
#define IOE_EREAD1      0x85    /* Enhanced read type 1 */
#define IOE_EREAD1_IDX  0x05    /* Enhanced read type 1 */
#define IOE_EWRITE1     0x86    /* Enhanced write type 1 */
#define IOE_EWRITE1_IDX 0x06    /* Enhanced write type 1 */
#define IOE_DIRECT1     0x87    /* Directive type 1 */
#define IOE_DIRECT1_IDX 0x07    /* Directive type 1 */
#define IOE_RAC         0x8c    /* Read with Atomic clear */
#define IOE_RAC_IDX     0x0c    /* Read with Atomic clear */
#define IOE_RAS         0x8d    /* Read with Atomic set */
#define IOE_RAS_IDX     0x0d    /* Read with Atomic set */
#define IOE_RAD         0x8e    /* Read with Atomic decrement */
#define IOE_RAD_IDX     0x0e    /* Read with Atomic decrement */
#define IOE_RAI         0x8f    /* Read with Atomic increment */
#define IOE_RAI_IDX     0x0f    /* Read with Atomic increment */

#define EOE_READ        0x00
#define EOE_WRITE       0x01
#define EOE_RAC         0x0c    /* Read with Atomic clear */
#define EOE_RAS         0x0d    /* Read with Atomic set */
#define EOE_RAD         0x0e    /* Read with Atomic decrement */
#define EOE_RAI         0x0f    /* Read with Atomic increment */
#define EOE_LDEC        0x10    /* Load external cache */
#define EOE_LDECL       0x11    /* Load external cache with stash lock */
#define EOE_LDECPE      0x12    /* Load external cache with preferred exclusive */
#define EOE_LDECPEL     0x13    /* Load external cache with preferred exclusive and lock */
#define EOE_LDECFE      0x14    /* Load external cache with forced exclusive */
#define EOE_LDECFEL     0x15    /* Load external cache with forced exclusive and lock */
#define EOE_RSA         0x16    /* Read with stash allocate */
#define EOE_RSAU        0x17    /* Read with stash allocate and unlock */
#define EOE_READI       0x18    /* Read with invalidate */
#define EOE_RWNITC      0x19    /* Read with no intention to cache */
#define EOE_WCI         0x1a    /* Write cache inhibited */
#define EOE_WWSA        0x1b    /* Write with stash allocate */
#define EOE_WWSAL       0x1c    /* Write with stash allocate and lock */
#define EOE_WWSAO       0x1d    /* Write with stash allocate only */
#define EOE_WWSAOL      0x1e    /* Write with stash allocate only and lock */
#define EOE_VALID       0x80

int32_t pamu_enable_liodn(uint32_t liodn);
int32_t pamu_disable_liodn(uint32_t liodn);
int32_t pamu_hw_init(void *pamu_reg_vbase, size_t reg_space_size,
		     void *pamubypenr_vaddr, void *pamu_tbl_vbase,
		     size_t pamu_tbl_size, int hw_ready);
void pamu_enable_interrupts(void *pamu_reg_vaddr, uint8_t pamu_enable_ints,
			    uint32_t threshold);
int32_t pamu_config_ppaace(uint32_t liodn, phys_addr_t win_addr, phys_addr_t win_size,
			   uint32_t omi, unsigned long rpn, uint32_t snoopid,
			   uint32_t stashid, uint32_t subwin_cnt);
int32_t pamu_config_spaace(uint32_t liodn, uint32_t subwin_cnt, phys_addr_t subwin_addr,
			   phys_addr_t subwin_size, uint32_t omi, unsigned long rpn,
			   uint32_t snoopid, uint32_t stash_dest);
int32_t pamu_reconfig_subwin(uint32_t liodn,  uint32_t subwin, unsigned long rpn);
int32_t pamu_reconfig_liodn(uint32_t liodn, unsigned long rpn);
paace_t *pamu_get_ppaace(uint32_t liodn);
paace_t *pamu_get_spaace(uint32_t fspi_index, uint32_t wnum);
ome_t *pamu_get_ome(uint8_t omi);
unsigned long pamu_get_fspi_and_allocate(uint32_t subwindow_cnt);
void pamu_setup_default_xfer_to_host_ppaace(paace_t *ppaace);
void pamu_setup_default_xfer_to_host_spaace(paace_t *spaace);
unsigned int pamu_get_max_subwindow_count(void);

#endif  /* __PAMU_H */

