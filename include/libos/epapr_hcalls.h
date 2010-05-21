/** @file
 * Freescale hypervisor call interface
 *
 * Author: Timur Tabi <timur@freescale.com>
 *
 * Copyright (C) 2007-2010 Freescale Semiconductor, Inc.
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

/* A "hypercall" is an "sc 1" instruction.  This header file file provides C
 * wrapper functions for the Freescale hypervisor interface.  It is inteded
 * for use by Linux device drivers and other operating systems.
 *
 * The hypercalls are implemented as inline assembly, rather than assembly
 * language functions in a .S file, for optimization.  It allows
 * the caller to issue the hypercall instruction directly, improving both
 * performance and memory footprint.
 *
 * If adding a hypercall, please make sure the functions are in the same
 * order as the corresponding hypercall numbers.
 */

#ifndef _EPAPR_HCALLS_H
#define _EPAPR_HCALLS_H

#include <libos/hcall-errors.h>
#include <libos/endian.h>
#include <libos/types.h>

/* For compatibility with Linux which shares a file like this one */
#define be32_to_cpu(x) cpu_from_be32(x)

#define FH_API_VERSION 1


#define  EV_BYTE_CHANNEL_SEND	      1
#define  EV_BYTE_CHANNEL_RECEIVE      2
#define  EV_BYTE_CHANNEL_POLL         3
#define  EV_INT_SET_CONFIG            4
#define  EV_INT_GET_CONFIG            5
#define  EV_INT_SET_MASK              6
#define	 EV_INT_GET_MASK              7
#define  EV_INT_GET_ACTIVITY          8
#define  EV_INT_IACK                  9
#define  EV_INT_EOI                  10
#define  EV_INT_SEND_IPI             11
#define  EV_INT_SET_TASK_PRIORITY    12
#define  EV_INT_GET_TASK_PRIORITY    13
#define  EV_DOORBELL_SEND            14
#define  EV_MSGSND                   15
#define  EV_IDLE                     16

/* vendor ID: epapr */
#define EV_VENDOR_ID                  0
#define EV_HCALL_TOKEN(hcall_number)  ((EV_VENDOR_ID << 16) | (hcall_number))

/*
 * Hypercall register clobber list
 *
 * These macros are used to define the list of clobbered registers during a
 * hypercall.  Technically, registers r0 and r3-r12 are always clobbered,
 * but the gcc inline assembly syntax does not allow us to specify registers
 * on the clobber list that are also on the input/output list.  Therefore,
 * the lists of clobbered registers depends on the number of register
 * parmeters ("+r" and "=r") passed to the hypercall.
 *
 * Each assembly block should use one of the HCALL_CLOBBERSx macros.  As a
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

/* List of common clobbered registers.  Do not use this macro. */
#define EV_HCALL_CLOBBERS "r0", "r12", "xer", "ctr", "lr", "cc"

#define EV_HCALL_CLOBBERS8 EV_HCALL_CLOBBERS
#define EV_HCALL_CLOBBERS7 EV_HCALL_CLOBBERS8, "r10"
#define EV_HCALL_CLOBBERS6 EV_HCALL_CLOBBERS7, "r9"
#define EV_HCALL_CLOBBERS5 EV_HCALL_CLOBBERS6, "r8"
#define EV_HCALL_CLOBBERS4 EV_HCALL_CLOBBERS5, "r7"
#define EV_HCALL_CLOBBERS3 EV_HCALL_CLOBBERS4, "r6"
#define EV_HCALL_CLOBBERS2 EV_HCALL_CLOBBERS3, "r5"
#define EV_HCALL_CLOBBERS1 EV_HCALL_CLOBBERS2, "r4"


/*
 * We use "uintptr_t" to define a register because it's guaranteed to be a
 * 32-bit integer on a 32-bit platform, and a 64-bit integer on a 64-bit
 * platform.
 *
 * All registers are either input/output or output only.  Registers that are
 * initialized before making the hypercall are input/output.  All
 * input/output registers are represented with "+r".  Output-only registers
 * are represented with "=r".  Do not specify any unused registers.  The
 * clobber list will tell the compiler that the hypercall modifies those
 * registers, which is good enough.
 */

/**
 * Configure the specified interrupt.
 * @param[in] interrupt the interrupt number
 * @param[in] config configuration for this interrupt
 * @param[in] priority interrupt priority
 * @param[in] destination destination CPU number
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_set_config(unsigned int interrupt,
	uint32_t config, unsigned int priority, uint32_t destination)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_SET_CONFIG);
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4") = config;
	register uintptr_t r5 __asm__("r5") = priority;
	register uintptr_t r6 __asm__("r6") = destination;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
		: : EV_HCALL_CLOBBERS4
	);

	return r3;
}

/**
 * Return the config of the specified interrupt.
 * @param[in] interrupt the interrupt number
 * @param[out] config configuration for this interrupt
 * @param[out] priority interrupt priority
 * @param[out] destination destination CPU number
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_get_config(unsigned int interrupt,
	uint32_t *config, unsigned int *priority, uint32_t *destination)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_GET_CONFIG);
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5), "=r" (r6)
		: : EV_HCALL_CLOBBERS4
	);

	*config = r4;
	*priority = r5;
	*destination = r6;

	return r3;
}


/**
 * Sets the mask for the specified interrupt source.
 * @param[in] interrupt the interrupt number
 * @param[in] mask 0=enable interrupts, 1=disable interrupts
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_set_mask(unsigned int interrupt,
	unsigned int mask)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_SET_MASK);
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4") = mask;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	return r3;
}

/**
 * Returns the mask for the specified interrupt source.
 * @param[in] interrupt the interrupt number
 * @param[out] mask mask for this interrupt (0=enabled, 1=disabled)
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_get_mask(unsigned int interrupt,
	unsigned int *mask)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_GET_MASK);
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	*mask = r4;

	return r3;
}

/**
 * Returns the activity status of an interrupt source.
 * @param[in] interrupt the interrupt number
 * @param[out] activity activity status.
 *
 * The activity status is a value that indicates whether an interrupt has
 * been requested (i.e. is in service).
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_get_activity(unsigned int interrupt,
	unsigned int *activity)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_GET_ACTIVITY);
	register uintptr_t r3 __asm__("r3") = interrupt;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	*activity = r4;

	return r3;
}

/**
 * Signal the end of interrupt processing.
 * @param[in] interrupt the interrupt number
 *
 * This function signals the end of processing for the the specified
 * interrupt, which must be the interrupt currently in service. By
 * definition, this is also the highest-priority interrupt.
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_eoi(unsigned int interrupt)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_EOI);
	register uintptr_t r3 __asm__("r3") = interrupt;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : EV_HCALL_CLOBBERS1
	);

	return r3;
}


/**
 * Send characters to a byte stream.
 * @param[in] handle byte stream handle
 * @param[in] count number of characters to send
 * @param[in,out] count input: number of characters to send,
 * output: number of characters sent
 * @param[in] buffer pointer to a 16-byte buffer
 *
 * 'buffer' must be at least 16 bytes long, because all 16 bytes will be
 * read from memory into registers, even if count < 16.
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_byte_channel_send(unsigned int handle,
	unsigned int *count, const char buffer[16])
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_BYTE_CHANNEL_SEND);

	const uint32_t *p = (const uint32_t *) buffer;
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4") = *count;
	register uintptr_t r5 __asm__("r5") = be32_to_cpu(p[0]);
	register uintptr_t r6 __asm__("r6") = be32_to_cpu(p[1]);
	register uintptr_t r7 __asm__("r7") = be32_to_cpu(p[2]);
	register uintptr_t r8 __asm__("r8") = be32_to_cpu(p[3]);

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3),
		  "+r" (r4), "+r" (r5), "+r" (r6), "+r" (r7), "+r" (r8)
		: : EV_HCALL_CLOBBERS6
	);

	*count = r4;

	return r3;
}

/**
 * Fetch characters from a byte channel.
 * @param[in] handle byte channel handle
 * @param[in,out] count input: max num of chars to receive, output: num
 * chars received
 * @param[out] buffer pointer to a 16-byte buffer
 *
 * The size of 'buffer' must be at least 16 bytes, even if you request fewer
 * than 16 charactgers, because we always write 16 bytes to 'buffer'.  This
 * is for performance reasons.
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_byte_channel_receive(unsigned int handle,
	unsigned int *count, char buffer[16])
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_BYTE_CHANNEL_RECEIVE);
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4") = *count;
	register uintptr_t r5 __asm__("r5");
	register uintptr_t r6 __asm__("r6");
	register uintptr_t r7 __asm__("r7");
	register uintptr_t r8 __asm__("r8");

	uint32_t *p = (uint32_t *) buffer;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "+r" (r4),
		  "=r" (r5), "=r" (r6), "=r" (r7), "=r" (r8)
		: : EV_HCALL_CLOBBERS6
	);

	*count = r4;
	p[0] = cpu_to_be32(r5);
	p[1] = cpu_to_be32(r6);
	p[2] = cpu_to_be32(r7);
	p[3] = cpu_to_be32(r8);

	return r3;
}

/**
 * Returns the status of the byte channel buffers.
 * @param[in] handle byte channel handle
 * @param[out] rx_count count of bytes in receive queue
 * @param[out] tx_count count of bytes in transmit queue
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_byte_channel_poll(unsigned int handle,
	unsigned int *rx_count,	unsigned int *tx_count)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_BYTE_CHANNEL_POLL);
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4");
	register uintptr_t r5 __asm__("r5");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4), "=r" (r5)
		: : EV_HCALL_CLOBBERS3
	);

	*rx_count = r4;
	*tx_count = r5;

	return r3;
}

/**
 * Acknowledge an interrupt.
 * @param[in] handle handle to the target interrupt controller
 * @param[out] vector interrupt vector
 *
 * If handle is zero, the function returns the next interrupt source
 * number to be handled irrespective of the hierarchy or cascading
 * of interrupt controllers. If non-zero, specifies a handle to the
 * interrupt controller that is the target of the acknowledge.
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_int_iack(unsigned int handle, unsigned int *vector)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_INT_IACK);
	register uintptr_t r3 __asm__("r3") = handle;
	register uintptr_t r4 __asm__("r4");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3), "=r" (r4)
		: : EV_HCALL_CLOBBERS2
	);

	*vector = r4;

	return r3;
}


/**
 * Send a doorbell to another partition.
 * @param[in] handle doorbell send handle
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_doorbell_send(unsigned int handle)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_DOORBELL_SEND);
	register uintptr_t r3 __asm__("r3") = handle;

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "+r" (r3)
		: : EV_HCALL_CLOBBERS1
	);

	return r3;
}


/**
 * Idle -- wait for next interrupt on this core
 *
 * @return 0 for success, or an error code.
 */
static inline unsigned int ev_idle(void)
{
	register uintptr_t r11 __asm__("r11") = EV_HCALL_TOKEN(EV_IDLE);
	register uintptr_t r3 __asm__("r3");

	__asm__ __volatile__ ("sc 1"
		: "+r" (r11), "=r" (r3)
		: : EV_HCALL_CLOBBERS1
	);

	return r3;
}

#endif
