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

#ifndef LIBOS_CORE_REGS_H
#define LIBOS_CORE_REGS_H

// FIXME: Separate out APUs and other target-specific definitions

#define MSRBIT_GS        35 // Guest State
#define MSRBIT_UCLE      37 // User-mode Cache Lock Enable
#define MSRBIT_SPE       38 // SPE Available
#define MSRBIT_WE        45 // Wait Enable
#define MSRBIT_CE        46 // Critical Enable
#define MSRBIT_EE        48 // External Enable
#define MSRBIT_PR        49 // User Mode
#define MSRBIT_FP        50 // Floating Point Available
#define MSRBIT_ME        51 // Machine Check Available
#define MSRBIT_FE0       52 // Floating Point Exception 0
#define MSRBIT_DE        54 // Debug Mode Enable
#define MSRBIT_FE1       55 // Floating Point Exception 1
#define MSRBIT_IS        58 // Instruction Address Space
#define MSRBIT_DS        59 // Data Address Space
#define MSRBIT_PMM       61 // Performance Monitor Mask

#define MSR_GS           (1 << (63 - MSRBIT_GS))
#define MSR_UCLE         (1 << (63 - MSRBIT_UCLE))
#define MSR_SPE          (1 << (63 - MSRBIT_SPE))
#define MSR_WE           (1 << (63 - MSRBIT_WE))
#define MSR_CE           (1 << (63 - MSRBIT_CE))
#define MSR_EE           (1 << (63 - MSRBIT_EE))
#define MSR_PR           (1 << (63 - MSRBIT_PR))
#define MSR_FP           (1 << (63 - MSRBIT_FP))
#define MSR_ME           (1 << (63 - MSRBIT_ME))
#define MSR_FE0          (1 << (63 - MSRBIT_FE0))
#define MSR_DE           (1 << (63 - MSRBIT_DE))
#define MSR_FE1          (1 << (63 - MSRBIT_FE1))
#define MSR_IS           (1 << (63 - MSRBIT_IS))
#define MSR_DS           (1 << (63 - MSRBIT_DS))
#define MSR_PMM          (1 << (63 - MSRBIT_PMM))

// MOVE
#define MSR_HVPRIV       (MSR_GS | MSR_UCLE | MSR_DE | MSR_WE | MSR_PMM)

#define SPR_XER          1    // Integer Exception Register
#define SPR_LR           8    // Link Register
#define SPR_CTR          9    // Count Register
#define SPR_DEC          22   // Decrementer

#define SPR_SRR0         26   // Save/Restore PC register
#define SPR_SRR1         27   // Save/Restore MSR register

#define SPR_PID          48   // Process ID Register

#define SPR_DECAR        54   // Decrementer Auto-Reload

#define SPR_CSRR0        58   // Critical SRR0
#define SPR_CSRR1        59   // Critical SRR1
#define SPR_DEAR         61   // Data Error Address Register

#define SPR_ESR          62   // Exception Syndrome Register
#define   ESR_PIL          0x08000000 // Illegal Instruction (Prog)
#define   ESR_PPR          0x04000000 // Privileged Instruction (Prog)
#define   ESR_PTR          0x02000000 // Trap (Prog)
#define   ESR_FP           0x01000000 // Floating Point (Align, DSI, DTLB, Prog)
#define   ESR_ST           0x00800000 // Store Operation (Align, DSI, DTLB)
#define   ESR_DLK          0x00200000 // D-cache locking (DSI)
#define   ESR_ILK          0x00100000 // I-cache locking (DSI)
#define   ESR_APU          0x00080000 // Auxiliary Proc (Align, DSI, DTLB, Prog)
#define   ESR_PUO          0x00040000 // Unimplemented Operation (Prog)
#define   ESR_BO           0x00020000 // Byte-ordering (DSI, ISI)
#define   ESR_PIE          0x00010000 // Imprecise Exception (Prog)
#define   ESR_SPE          0x00000080 // SPE/Embedded FP/Altivec
                                      // (Align, DSI, DTLB,
                                      //  Embedded FP Data/Round)
#define   ESR_EPID         0x00000040 // External PID Access
#define   ESR_VLEMI        0x00000020 // VLE instruction
#define   ESR_MIF          0x00000002 // Misaligned Insn Fetch (ITLB, ISI)
#define   ESR_XTE          0x00000001 // External Transaction Err (ISI, DSI)


#define SPR_IVPR         63   // Interrupt Vector Prefix Register
#define IVPR_MASK        0xffff0000

#define SPR_TBL          268  // Timebase Lower
#define SPR_TBU          269  // Timebase Upper

// SPR General Registers
#define SPR_USPRG0       256
#define SPR_USPRG3       259
#define SPR_USPRG4       260
#define SPR_USPRG5       261
#define SPR_USPRG6       262
#define SPR_USPRG7       263

#define SPR_SPRG0        272
#define SPR_SPRG1        273
#define SPR_SPRG2        274
#define SPR_SPRG3        275
#define SPR_SPRG4        276
#define SPR_SPRG5        277
#define SPR_SPRG6        278
#define SPR_SPRG7        279

#define SPR_TBWL         284  // Timebase Lower for Writing
#define SPR_TBWU         285  // Timebase Upper for Writing

#define SPR_PIR          286  // Processor ID Register
#define SPR_PVR          287  // Processor Version Register

#define SPR_DBSRWR       306  // DBSR Write Register

#define SPR_EHCSR        307  // Embedded Hypervisor Control/Status
#define   EHCSR_EXTGS      0x80000000 // Guest gets external ints
#define   EHCSR_DTLBGS     0x40000000 // Guest gets DTLB errors
#define   EHCSR_ITLBGS     0x20000000 // Guest gets ITLB errors
#define   EHCSR_DSIGS      0x10000000 // Guest gets DSIs
#define   EHCSR_ISIGS      0x08000000 // Guest gets ISIs
#define   EHCSR_DUVD       0x04000000 // Disable Embedded HV Debug
#define   EHCSR_DGTMI      0x00800000 // Disable guest TLB management insns
#define   EHCSR_DMIUH      0x00400000 // Disable MAS int updates for hypervisor

#define SPR_DBSR         304  // Debug Status Register
#define   DBSR_ICMP        0x08000000 // Instruction complete debug event
#define   DBSR_BRT         0x04000000 // Branch taken debug event
#define   DBSR_IRPT        0x02000000 // Interrupt taken debug event
#define   DBSR_TRAP        0x01000000 // Trap instruction debug event
#define   DBSR_IAC1        0x00800000 // IAC1 debug event
#define   DBSR_IAC2        0x00400000 // IAC2 debug event
#define   DBSR_IAC3        0x00200000 // IAC3 debug event
#define   DBSR_IAC4        0x00100000 // IAC4 debug event
#define   DBSR_DAC1R       0x00080000 // DAC1 read debug event
#define   DBSR_DAC1W       0x00040000 // DAC1 write debug event
#define   DBSR_DAC2R       0x00020000 // DAC2 read debug event
#define   DBSR_DAC2W       0x00010000 // DAC2 write debug event
#define   DBSR_RET         0x00008000 // Return debug event

#define SPR_DBCR0        308  // Debug control register 0
#define   DBCR0_IDM        0x40000000 // Internal debug mode
#define   DBCR0_RST        0x30000000 // Reset mask
#define   DBCR0_ICMP       0x08000000 // Instruction complete debug event
#define   DBCR0_BRT        0x04000000 // Branch taken debug event
#define   DBCR0_IRPT       0x02000000 // Interrupt taken debug event
#define   DBCR0_TRAP       0x01000000 // Trap instruction debug event
#define   DBCR0_IAC1       0x00800000 // IAC1 debug event
#define   DBCR0_IAC2       0x00400000 // IAC2 debug event
#define   DBCR0_IAC3       0x00200000 // IAC3 debug event
#define   DBCR0_IAC4       0x00100000 // IAC4 debug event
#define   DBCR0_DAC1R      0x00080000 // DAC1 read debug event
#define   DBCR0_DAC1W      0x00040000 // DAC1 write debug event
#define   DBCR0_DAC2R      0x00020000 // DAC2 read debug event
#define   DBCR0_DAC2W      0x00010000 // DAC2 write debug event
#define   DBCR0_RET        0x00008000 // Return debug event

#define SPR_MSRP         311  // MSR Protect
#define   MSRP_UCLEP     0x04000000 // Protect MSR[UCLE]
#define   MSRP_DEP       0x00000200 // Protect MSR[DE]
#define   MSRP_PMMP      0x00000040 // Protect MSR[PMM]

#define SPR_TSR          336  //  Timer Status Register
#define   TSR_DIS          0x08000000 // Decrementer Int Pending
#define   TSR_FIS          0x04000000 // Fixed Interval Int Pending

#define SPR_LPIDR        338  // Logical Partition ID

#define SPR_TCR          340  //  Timer Control Register
#define   TCR_DIE          0x04000000 // Decrementer Int Enable
#define   TCR_DIE_SHIFT    26
#define   TCR_ARE          0x00400000 // Auto-reload enable
#define   TCR_FIE          0x00800000 // Fixed Interval Int Enable
#define   TCR_FIE_SHIFT    23

// Guest SPR General Registers
#define SPR_GSPRG0       368
#define SPR_GSPRG1       369
#define SPR_GSPRG2       370
#define SPR_GSPRG3       371

#define SPR_GSRR0        378  // Guest SRR0
#define SPR_GSRR1        379  // Guest SRR1
#define SPR_GEPR         380  // Guest EPR
#define SPR_GDEAR        381  // Guest DEAR
#define SPR_GPIR         382  // Guest PIR
#define SPR_GESR         383  // Guest ESR

#define IVOR_MASK        0x0000fff0
#define SPR_IVOR0        400  // Critical Input
#define SPR_IVOR1        401  // Machine Check
#define SPR_IVOR2        402  // Data Storage (DSI)
#define SPR_IVOR3        403  // Instruction Storage (ISI)
#define SPR_IVOR4        404  // External Input
#define SPR_IVOR5        405  // Alignment
#define SPR_IVOR6        406  // Program
#define SPR_IVOR7        407  // FP Unavailable
#define SPR_IVOR8        408  // System Call
#define SPR_IVOR9        409  // Auxiliary Processor Unavailable
#define SPR_IVOR10       410  // Decrementer
#define SPR_IVOR11       411  // Fixed Interval Timer
#define SPR_IVOR12       412  // Watchdog
#define SPR_IVOR13       413  // DTLB Error
#define SPR_IVOR14       414  // ITLB Error
#define SPR_IVOR15       415  // Debug

#define SPR_IVOR38       432  // Guest Processor Doorbell
#define SPR_IVOR39       433  // Guest Processor Doorbell Critical
#define SPR_IVOR40       434  // Hypervisor System Call
#define SPR_IVOR41       435  // Hypervisor Privelege

#define SPR_GIVOR2       440  // Guest DSI
#define SPR_GIVOR3       441  // guest ISI
#define SPR_GIVOR4       442  // Guest External Input
#define SPR_GIVOR8       443  // Guest System Call
#define SPR_GIVOR13      444  // Guest DTLB Error
#define SPR_GIVOR14      445  // Guest ITLB Error
#define SPR_GIVPR        447  // Guest IVPR

#define SPR_ATBL         526  // Alternate Time Base Lower
#define SPR_ATBU         527  // Alternate Time Base Upper

#define SPR_IVOR32       528  // Altivec/SPE/Embedded FP Unavailable
#define SPR_IVOR33       529  // Altivec Assist/Embedded FP Data
#define SPR_IVOR34       530  // Embedded FP Round
#define SPR_IVOR35       531  // Performance Monitor
#define SPR_IVOR36       532  // Processor Doorbell
#define SPR_IVOR37       533  // Processor Doorbell Critical

#define SPR_MCARU        569  // Machine Check Address Upper
#define SPR_MCSRR0       570  // Machine Check SRR0
#define SPR_MCSRR1       571  // Machine Check SRR1
#define SPR_MCSR         572  // Machine Check Status
#define   MCSR_MCP         0x80000000 // Input to core
#define   MCSR_NMI         0x00100000 // Non-Maskable Interrupt
#define   MCSR_MAV         0x00080000 // Address Valid
#define   MCSR_MEA         0x00040000 // MCAR is virtual
#define   MCSR_IF          0x00010000 // Instruction Fetch
#define   MCSR_LD          0x00008000 // Load
#define   MCSR_ST          0x00002000 // Store

#define SPR_MCAR         573  // Machine check Address

#define SPR_DSRR0        574  // Debug SRR0
#define SPR_DSRR1        575  // Debug SRR1
#define SPR_DDAM         576  // Debug Data Acquisition Message

// SPR General Registers
#define SPR_SPRG8        604
#define SPR_SPRG9        605

#define SPR_PID1         633  // Process ID Register 1 (e500v1, e500v2)
#define SPR_PID2         634  // Process ID Register 2 (e500v1, e500v2)

#define SPR_TLB0CFG      688  // TLB 0 Config Register
#define SPR_TLB1CFG      689  // TLB 1 Config Register
#define   TLBCFG_ASSOC_MASK  0xff000000 // Associativity of TLB
#define   TLBCFG_ASSOC_SHIFT 24
#define   TLBCFG_NENTRY_MASK 0x00000fff // Number of entries in TLB

#define SPR_CDCSR0       696  // Core Device Control and Status 0
#define   CDCSR0_SPE       0
#define   CDCSR0_MTHREAD   8
#define   CDCSR0_ALTIVEC   16
#define   CDCSR0_FPU       24
#define   CDCSR_AWARE      0x80
#define   CDCSR_PRESENT    0x40
#define   CDCSR_GET_STATE(x)   ((x) >> 3) & 7)
#define   CDCSR_GET_CONTROL(x) ((x) & 7)
#define   CDCSR_STATE(x)       ((x) << 3) & 7)
#define   CDCSR_CONTROL(x)     ((x) & 7)

#define SPR_EPR          702  // External Proxy Register

#define SPR_EPLC         947  // External PID Load Context
#define SPR_EPSC         948  // External PID Store Context
#define   EPC_EPR          0x80000000 // 1 = user, 0 = kernel
#define   EPCBIT_EPR       32
#define   EPC_EAS          0x40000000 // Address Space
#define   EPCBIT_EAS       33
#define   EPC_EGS          0x20000000 // 1 = guest, 0 = hypervisor
#define   EPCBIT_EGS       34
#define   EPC_ELPID        0x00ff0000
#define   EPC_ELPID_SHIFT  16
#define   EPC_EPID         0x00003fff
#define   EPC_EPID_SHIFT   0

#define SPR_HID0         1008 // Hardware Implementation Dependent 0
#define   HID0_EMCP        0x80000000 // Enable Machine Check Pin
#define   HID0_L2MMU_MHD   0x40000000 // Enable L2MMU Multi-Hit Detection
#define   HID0_DOZE        0x00800000
#define   HID0_NAP         0x00400000
#define   HID0_SLEEP       0x00200000
#define   HID0_DPM         0x00100000 // Dynamic Power Management
#define   HID0_EDPM        0x00080000 // Enhanced Dynamic Power Management
#define   HID0_ICR         0x00020000 // Interrupts Clear Reservation
#define   HID0_EIEC        0x00008000 // Enable Internal Error Checking
#define   HID0_ENMAS7      0x00000080 // Enable MAS7 Update
#define   HID0_DCFA        0x00000040 // Data Cache Flush Assist
#define   HID0_CIGLSO      0x00000010 // Cache-Inhibited Guarded Ordering
#define   HID0_NOPTI       0x00000001 // No-op Cache Touch Instructions

#define SPR_L1CSR0       1010 // L1 Cache Control and Status Register 0
#define   L1CSR0_DCPE      0x00010000 // Data Cache Parity Enable
#define   L1CSR0_DCLFR     0x00000100 // Data Cache Lock Bits Flash Reset
#define   L1CSR0_DCBZ32    0x00000008 // dcbz works on 32-byte blocks
#define   L1CSR0_DCFI      0x00000002 // Data Cache Flash Invalidate
#define   L1CSR0_DCE       0x00000001 // Data Cache Enable
#define   L1CSR0_DCUL      0x00000400 // Data Cache Unable to Lock

#define SPR_L1CSR1       1011 // L1 Cache Control and Status Register 1
#define   L1CSR1_ICPE      0x00010000 // Instruction Cache Parity Enable
#define   L1CSR1_ICLFR     0x00000100 // Instruction Cache Lock Bits Flash Reset
#define   L1CSR1_ICFI      0x00000002 // Instruction Cache Flash Invalidate
#define   L1CSR1_ICE       0x00000001 // Instruction Cache Enable
#define   L1CSR1_ICUL      0x00000400 // Instruction Cache Unable to Lock

#define SPR_MMUCSR0      1012 // MMU Control and Status Register 0
#define   MMUCSR_L2TLB0_FI 0x00000004 // Invalidate TLB0
#define   MMUCSR_L2TLB1_FI 0x00000002 // Invalidate TLB1

#define SPR_BUCSR        1013
#define   BUCSR_BBFI       0x00000200 // Branch Buffer Flash Invalidate
#define   BUCSR_BPEN       0x00000001 // Branch Prediction Enable

#define SPR_MMUCFG       1015 // MMU Configuration Register
#define   MMUCFG_LPIDSIZE  0x0f000000 // Number of LPID bits
#define   MMUCFG_LPIDSIZE_SHIFT 24
#define   MMUCFG_RASIZE    0x00fe0000 // Number of real-address bits
#define   MMUCFG_RASIZE_SHIFT   17
#define   MMUCFG_NPIDS     0x00007800 // Number of PID registers
#define   MMUCFG_NPIDS_SHIFT    11
#define   MMUCFG_PIDSIZE   0x000007c0 // Bits in a PID register, minus one
#define   MMUCFG_PIDSIZE_SHIFT  6
#define   MMUCFG_NTLBS     0x0000000c // Number of TLBs
#define   MMUCFG_NTLBS_SHIFT    2
#define   MMUCFG_MAVN      0x00000003 // MMU architecture version

#define SPR_SVR          1023 // System Version Register

#endif
