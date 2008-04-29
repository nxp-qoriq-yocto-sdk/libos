/*
 * Freescale hypervisor system call interface
 *
 * Author: Timur Tabi <timur@freescale.com>
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * Dual License:
 *
 * A) This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any warranty
 * of any kind, whether express or implied.
 *
 * B) Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This software is provided by the author "as is" and any express or
 * implied warranties, including, but not limited to, the implied warranties
 * of merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the author be liable for any direct, indirect,
 * incidental, special, exemplary, or consequential damages (including, but
 * not limited to, procurement of substitute goods or services; loss of use,
 * data, or profits; or business interruption) however caused and on any
 * theory of liability, whether in contract, strict liability, or tort
 * (including negligence or otherwise) arising in any way out of the use of
 * this software, even if advised of the possibility of such damage.
 */

#ifndef _FREESCALE_HCALLS_H
#define _FREESCALE_HCALLS_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <asm/byteorder.h>
#else
#define be32_to_cpu(x) (x)
#define cpu_to_be32(x) (x)
#endif

#define FH_CPU_WHOAMI                   1
#define FH_SET_PROCESSOR_ID             3
#define FH_PARTITION_RESTART            5
#define FH_PARTITION_GET_STATUS         6
#define FH_PARTITION_START              7
#define FH_PARTITION_STOP               8
#define FH_PARTITION_MEMCPY             9
#define FH_VMPIC_SET_INT_CONFIG         10
#define FH_VMPIC_GET_INT_CONFIG         11
#define FH_VMPIC_SET_MASK               14
#define FH_VMPIC_GET_MASK               15
#define FH_VMPIC_GET_ACTIVITY           16
#define FH_VMPIC_EOI                    17
#define FH_BYTE_CHANNEL_SEND            18
#define FH_BYTE_CHANNEL_RECEIVE         19
#define FH_BYTE_CHANNEL_POLL            20
#define FH_VMPIC_IACK                   21
#define FH_GPIO_GET_CONFIG              22
#define FH_GPIO_SET_CONFIG              23
#define FH_GPIO_GET_GPDAT               24
#define FH_GPIO_SET_GPDAT               25
#define FH_GPIO_GET_GPIER               26
#define FH_GPIO_SET_GPIER               27
#define FH_GPIO_GET_GPIMR               28
#define FH_GPIO_SET_GPIMR               29
#define FH_PARTITION_SEND_DBELL         32

/*
 * System call register clobber list
 *
 * These macros are used to define the list of clobbered registers during a
 * system call.  Technically, registers r0 and r3-r12 are always clobbered,
 * but the gcc inline assembly syntax does not allow us to specify registers
 * on the clobber list that are also on the input/output list.  Therefore,
 * the lists of clobbered registers depends on the number of register
 * parmeters ("+r" and "=r") passed to the system call.
 *
 * Each assembly block should use one of the SYSCALL_CLOBBERSx macros.  As a
 * general rule, 'x' is the number of parameters passed to the assembly
 * block *except* for r11.
 *
 * If you're not sure, just use the smallest value of 'x' that does not
 * generate a compilation error.  Because these are static inline functions,
 * the compiler will only check the clobber list for a function if you
 * compile code that calls that function.
 *
 * r3 and r11 are not included in any clobbers list because they are always
 * listed as output registers.
 *
 * XER, CTR, and LR are currently listed as clobbers because it's uncertain
 * whether they will be clobbered.
 *
 * Note that r11 can be used as an output parameter.
*/

/* List of core clobbered registers.  Do not use this macro. */
#define SYSCALL_CLOBBERS "r0", "r12", "xer", "ctr", "lr", "cc"

#define SYSCALL_CLOBBERS8 SYSCALL_CLOBBERS
#define SYSCALL_CLOBBERS7 SYSCALL_CLOBBERS8, "r10"
#define SYSCALL_CLOBBERS6 SYSCALL_CLOBBERS7, "r9"
#define SYSCALL_CLOBBERS5 SYSCALL_CLOBBERS6, "r8"
#define SYSCALL_CLOBBERS4 SYSCALL_CLOBBERS5, "r7"
#define SYSCALL_CLOBBERS3 SYSCALL_CLOBBERS4, "r6"
#define SYSCALL_CLOBBERS2 SYSCALL_CLOBBERS3, "r5"
#define SYSCALL_CLOBBERS1 SYSCALL_CLOBBERS2, "r4"

/*
 * We use "uintptr_t" to define a register because it's guaranteed to be a
 * 32-bit integer on a 32-bit platform, and a 64-bit integer on a 64-bit
 * platform.
 *
 * All registers are either input/output or output only.  Registers that are
 * initialized before making the system call are input/output.  All
 * input/output registers are represented with "+r".  Output-only registers
 * are represented with "=r".  Do not specify any unused registers.  The
 * clobber list will tell the compiler that the system call modifies those
 * registers, which is good enough.
 */

/**
 * get the index number of the running virtual CPU
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_cpu_whoami(unsigned int *cpu_index)
{
	register uintptr_t r11 __asm__("r11") = FH_CPU_WHOAMI;
	register uintptr_t r3 __asm__("r3");
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "=r" (r3), "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*cpu_index = r4;

	return r3;
}

/**
 * reboot the current partition
 *
 * Returns an error code if reboot failed.  Does not return if it succeeds.
 */
static inline int fh_partition_restart(unsigned int partition)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_RESTART;
	register uintptr_t r3 __asm__("r3") = partition;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : SYSCALL_CLOBBERS1
	);

	return r3;
}

/**
 * gets the status of a partition
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_partition_get_status(unsigned int partition,
	unsigned int *status, unsigned int *num_cpus, unsigned long *mem_size)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_GET_STATUS;
	register uintptr_t r3 __asm__("r3") = partition;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5), "=r" (r6)
		: : SYSCALL_CLOBBERS4
	);

	*status = r4;
	*num_cpus = r5;
	*mem_size = r6;

	return r3;
}

/**
 * boots and starts the execution of the specified partition
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_partition_start(unsigned int partition,
	uint32_t entry_point, unsigned long device_tree)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_START;
	register uintptr_t r3 __asm__("r3") = partition;
	register uintptr_t r4 __asm__("r4") = entry_point;
	register uintptr_t r5 __asm__("r5") = device_tree;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4), "+r" (r5)
		: : SYSCALL_CLOBBERS3
	);

	return r3;
}

/**
 * stops another partition
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_partition_stop(unsigned int partition)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_STOP;
	register uintptr_t r3 __asm__("r3") = partition;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : SYSCALL_CLOBBERS1
	);

	return r3;
}

/**
 * Structure definition for the fh_memcpy scatter-gather list
 *
 * This structure must be aligned on 32-byte boundary
 *
 * @source: guest physical address to copy from
 * @target: guest physical address to copy to
 * @size: number of bytes to copy
 * @reserved: reserved, must be zero
 */
struct fh_sg_list {
	uint64_t source;
	uint64_t target;
	uint64_t size;
	uint64_t reserved;
} __attribute__ ((aligned(32)));

/**
 * copies data from one guest to another
 *
 * @source: the ID of the partition to copy from
 * @target: the ID of the partition to copy to
 * @sg_list: guest physical address of an array of fh_sg_list structures
 * @count: the number of entries in sg_list[]
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_partition_memcpy(unsigned int source, unsigned int target,
	phys_addr_t sg_list, unsigned int count)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_MEMCPY;
	register uintptr_t r3 __asm__("r3") = source;
	register uintptr_t r4 __asm__("r4") = target;
	register uintptr_t r5 __asm__("r5") = (uint32_t) sg_list;
#ifdef CONFIG_PHYS_64BIT
	register uintptr_t r6 __asm__("r6") = sg_list >> 32;
#else
	register uintptr_t r6 __asm__("r6") = 0;
#endif
	register uintptr_t r7 __asm__("r7") = count;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11),
		  "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7)
		: : SYSCALL_CLOBBERS5
	);

	return r3;
}

/**
 * fh_vmpic_set_int_config - configures the specified interrupt
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_set_int_config(unsigned int interrupt,
	uint32_t config, unsigned int priority, uint32_t destination)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_SET_INT_CONFIG;
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4") = config;
	register uintptr_t r5 __asm__("r5") = priority;
	register uintptr_t r6 __asm__("r6") = destination;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
		: : SYSCALL_CLOBBERS4
	);

	return r3;
}

/**
 * returns the configuration of the specified interrupt
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_get_int_config(unsigned int interrupt,
	uint32_t *config, unsigned int *priority, uint32_t *destination)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_GET_INT_CONFIG;
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5), "=r" (r6)
		: : SYSCALL_CLOBBERS4
	);

	*config = r4;
	*priority = r5;
	*destination = r6;

	return r3;
}

/**
 * sets the mask for the specified interrupt source
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_set_mask(unsigned int interrupt, unsigned int mask)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_SET_MASK;
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4") = mask;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	return r3;
}

/**
 * returns the mask for the specified interrupt source
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_get_mask(unsigned int interrupt, unsigned int *mask)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_GET_MASK;
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*mask = r4;

	return r3;
}

/**
 * returns a value indicating the activity status of an interrupt source
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_get_activity(unsigned int interrupt,
	unsigned int *activity)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_GET_ACTIVITY;
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*activity = r4;

	return r3;
}

/**
 * signal the end of processing for the highest-priority interrupt currently
 * in service
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_eoi(unsigned int interrupt)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_EOI;
	register uintptr_t r3 __asm__("r3") = interrupt;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : SYSCALL_CLOBBERS1
	);

	return r3;
}

/**
 * send characters to a byte stream
 *
 * @handle byte stream handle
 * @count number of characters to send
 * @buffer pointer to a 16-byte buffer
 *
 * The buffer must be at least 16 bytes long, because all 16 bytes will be
 * read from memory into registers, even if count < 16.
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_byte_channel_send(unsigned int handle,
	unsigned int count, const char buffer[16])
{
	register uintptr_t r11 __asm__("r11") = FH_BYTE_CHANNEL_SEND;

	const uint32_t *p = (const uint32_t *) buffer;
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4") = count;
	register uintptr_t r5 __asm__("r5") = be32_to_cpu(p[0]);
	register uintptr_t r6 __asm__("r6") = be32_to_cpu(p[1]);
	register uintptr_t r7 __asm__("r7") = be32_to_cpu(p[2]);
	register uintptr_t r8 __asm__("r8") = be32_to_cpu(p[3]);

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3),
		  "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7), "+r" (r8)
		: : SYSCALL_CLOBBERS6
	);

	return r3;
}

/**
 * fetch characters from a byte stream
 *
 * @handle byte stream handle
 * @count (input) maximum number of characters to receive, (output) number
 *	of characters received
 * @str pointer to a 16-byte buffer
 *
 * Note that even if count < 16, we still write 16 bytes to 'str'.
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_byte_channel_receive(unsigned int handle,
	unsigned int *count, char str[16])
{
	register uintptr_t r11 __asm__("r11") = FH_BYTE_CHANNEL_RECEIVE;
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4") = *count;
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");
	register uintptr_t r7 __asm__("r7");
	register uintptr_t r8 __asm__("r8");

	uint32_t *p = (uint32_t *) str;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4),
		  "=r" (r5), "=r" (r6), "=r" (r7), "=r" (r8)
		: : SYSCALL_CLOBBERS6
	);

	*count = r4;
	p[0] = cpu_to_be32(r5);
	p[1] = cpu_to_be32(r6);
	p[2] = cpu_to_be32(r7);
	p[3] = cpu_to_be32(r8);

	return r3;
}

/**
 * returns the status of the byte-channel send and receive buffers
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_byte_channel_poll(unsigned int handle,
	unsigned int *rx_count,	unsigned int *tx_count)
{
	register uintptr_t r11 __asm__("r11") = FH_BYTE_CHANNEL_POLL;
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5)
		: : SYSCALL_CLOBBERS3
	);

	*rx_count = r4;
	*tx_count = r5;

	return r3;
}

/**
 * acknowledge an interrupt
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_vmpic_iack(unsigned int *vector)
{
	register uintptr_t r11 __asm__("r11") = FH_VMPIC_IACK;
	register uintptr_t r3 __asm__("r3");
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "=r" (r3), "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*vector = r4;

	return r3;
}

/**
 * returns the defined configuration of the individual ports for the
 * specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_get_config(unsigned int set, unsigned int *direction,
	unsigned int *config, unsigned int *control)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_GET_CONFIG;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5), "=r" (r6)
		: : SYSCALL_CLOBBERS4
	);

	*direction = r4;
	*config = r5;
	*control = r6;

	return r3;
}

/**
 * sets the defined configuration of the individual ports for the specified
 * GPIO set.
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_set_config(unsigned int set, unsigned int direction,
	unsigned int config, unsigned int control)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_SET_CONFIG;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4") = direction;
	register uintptr_t r5 __asm__("r5") = config;
	register uintptr_t r6 __asm__("r6") = control;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
		: : SYSCALL_CLOBBERS4
	);

	return r3;
}

/**
 * returns the GPIO port data associated with the specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_get_gpdat(unsigned int set, unsigned int *data)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_GET_GPDAT;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3),  "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*data = r4;

	return r3;
}

/**
 * sets the GPIO port data associated with the specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_set_gpdat(unsigned int set, unsigned int data)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_SET_GPDAT;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4") = data;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	return r3;
}

/**
 * returns information of the events that caused an interrupt for the
 * specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_get_gpier(unsigned int set, unsigned int *events)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_GET_GPIER;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3),  "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*events = r4;

	return r3;
}

/**
 * clear interrupts for the specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_set_gpier(unsigned int set, unsigned int events)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_SET_GPIER;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4") = events;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	return r3;
}

/**
 * returns interrupt masking configuration for the ports in the specified
 * GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_get_gpimr(unsigned int set, unsigned int *mask)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_GET_GPIMR;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3),  "=r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	*mask = r4;

	return r3;
}

/**
 * sets the GPIO port data associated with the specified GPIO set
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_gpio_set_gpimr(unsigned int set, unsigned int mask)
{
	register uintptr_t r11 __asm__("r11") = FH_GPIO_SET_GPIMR;
	register uintptr_t r3 __asm__("r3") = set;
	register uintptr_t r4 __asm__("r4") = mask;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : SYSCALL_CLOBBERS2
	);

	return r3;
}

/**
 * sends a doorbell to another partition
 *
 * Returns 0 for success, or an error code.
 */
static inline int fh_partition_send_dbell(unsigned int partition)
{
	register uintptr_t r11 __asm__("r11") = FH_PARTITION_SEND_DBELL;
	register uintptr_t r3 __asm__("r3") = partition;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : SYSCALL_CLOBBERS1
	);

	return r3;
}

#endif
