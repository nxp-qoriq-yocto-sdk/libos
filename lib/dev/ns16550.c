/** @file
 * This file contains the implementation of a ns16550 device driver
 */

/*
 * Copyright (C) 2008,2009 Freescale Semiconductor, Inc.
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

#include <string.h>

#include <libos/ns16550.h>
#include <libos/chardev.h>
#include <libos/libos.h>
#include <libos/io.h>
#include <libos/bitops.h>
#include <libos/uart.h>
#include <libos/interrupts.h>
#include <libos/errors.h>
#include <libos/percpu.h>
#include <libos/alloc.h>

// FIXME -- get clock from device tree
#define get_system_clock() 266

typedef struct {
	chardev_t cd;
	uint8_t *reg;
	interrupt_t *irq;
	uint32_t lock;
	int txfifo;
	int tx_counter;
	int rx_counter;
	int err_counter;
} ns16550;

#if defined(INTERRUPTS) && defined(CONFIG_LIBOS_QUEUE)
/** Transmit consumer callback
 *
 * Transmit as much as possible and activate Tx FIFO empty interrupt.
 *
 * @param[in] q 
 * @note Can be called from any core.
 */

static void __ns16550_tx_callback(ns16550 *priv)
{
	int i;

	while (1) {
		if (!(in8(&priv->reg[NS16550_LSR]) & NS16550_LSR_THRE)) {
			out8(&priv->reg[NS16550_IER],
			     in8(&priv->reg[NS16550_IER]) | NS16550_IER_ETHREI);

			return;
		}

		for (i = 0; i < priv->txfifo; i++) {
			int c = queue_readchar(priv->cd.tx, 0);
			if (c < 0) {
				out8(&priv->reg[NS16550_IER],
				     in8(&priv->reg[NS16550_IER]) & ~NS16550_IER_ETHREI);
			
				return;
			}
	
			priv->tx_counter++;
			out8(&priv->reg[NS16550_THR], c);
		}

		if (1) {
			out8(&priv->reg[NS16550_IER],
			     in8(&priv->reg[NS16550_IER]) | NS16550_IER_ETHREI);

			return;
		}
	}
}

static void ns16550_tx_callback(queue_t *q)
{
	ns16550 *priv = q->consumer;
	assert(q == priv->cd.tx);
	int lock = !unlikely(cpu->crashing);

	register_t saved = disable_int_save();

	if (lock)
		spin_lock(&priv->lock);

	__ns16550_tx_callback(priv);

	if (lock)
		spin_unlock(&priv->lock);

	restore_int(saved);
}

/*!

    @brief UART ISR for tx and rx

    The function handles hardware interrupt of UART

    @param [in]     lld_handler  IN a handler of UART LLD.

    @return void

    @note Called on primary partition

*/

static int ns16550_isr(void *arg)
{
	ns16550 *priv = arg;
	uint8_t iir, lsr;
	int rx_notify = 0, tx_notify = 0;

	spin_lock(&priv->lock);

	iir = in8(&priv->reg[NS16550_IIR]);
	if (iir & NS16550_IIR_NOIRQ) {
		spin_unlock(&priv->lock);
		return -1;
	}
	
	iir &= NS16550_IIR_IID;

	if (iir == NS16550_IIR_MSI)
		in8(&priv->reg[NS16550_MSR]);

	/* Receiver Line Status Error */
	if (iir == NS16550_IIR_RLSI) {
		lsr = in8(&priv->reg[NS16550_LSR]);
		if (lsr & NS16550_LSR_OE)
			priv->err_counter++;
		if (lsr & NS16550_LSR_RFE)
			priv->err_counter++;
	}

	/* Either receiver data available or receiver timeout, call store */
	if (iir == NS16550_IIR_RDAI || iir == NS16550_IIR_RXTIME) {
		while (in8(&priv->reg[NS16550_LSR]) & NS16550_LSR_DR) {
			uint8_t data = in8(&priv->reg[NS16550_RBR]);
			priv->rx_counter++;

			if (priv->cd.rx) {
				int ret = queue_writechar(priv->cd.rx, data);
				if (ret < 0)
					priv->err_counter++; 
				else
					rx_notify = 1;
			} else {
				/* Should never happen... */
				priv->err_counter++;
			}
		}
	}

	/* Transmitter holding register empty */
	/* Try to transmit data - if no data available desactivate ISR */
	if (iir == NS16550_IIR_THREI && priv->cd.tx) {
		__ns16550_tx_callback(priv);
		tx_notify = 1;
	}

	spin_unlock(&priv->lock);

	if (rx_notify)
		queue_notify_consumer(priv->cd.rx);

	if (tx_notify) {
		queue_notify_producer(priv->cd.tx);

		if (!queue_empty(priv->cd.tx)) {
			spin_lock(&priv->lock);
			out8(&priv->reg[NS16550_IER],
			     in8(&priv->reg[NS16550_IER]) | NS16550_IER_ETHREI);
			spin_unlock(&priv->lock);
		}
	}

	return 0;
}

static int ns16550_set_tx_queue(chardev_t *cd, queue_t *q)
{
	ns16550 *priv = to_container(cd, ns16550, cd);
	int ret = 0;

	if (!priv->irq)
		return ERR_INVALID;

	unsigned long saved = spin_lock_intsave(&priv->lock);
	
	if (q) {
		if (cd->tx) {
			ret = ERR_BUSY;
			goto out;
		}
	
		cd->tx = q;
		q->consumer = priv;
		smp_mbar();
		q->data_avail = ns16550_tx_callback;
		smp_sync();

		if (!queue_empty(q))
			out8(&priv->reg[NS16550_IER],
			     in8(&priv->reg[NS16550_IER]) | NS16550_IER_ETHREI);
	} else if (cd->tx) {
		cd->tx->data_avail = NULL;
		out8(&priv->reg[NS16550_IER],
		     in8(&priv->reg[NS16550_IER]) & ~NS16550_IER_ETHREI);
		// FIXME: sync with IRQ handlers/callbacks
		cd->tx->consumer = NULL;
		cd->tx = NULL;
	}

out:
	spin_unlock_intsave(&priv->lock, saved);
	return ret;
}

static int ns16550_set_rx_queue(chardev_t *cd, queue_t *q)
{
	ns16550 *priv = to_container(cd, ns16550, cd);
	int ret = 0;

	if (!priv->irq)
		return ERR_INVALID;

	unsigned long saved = spin_lock_intsave(&priv->lock);

	if (q && cd->rx) {
		ret = ERR_BUSY;
		goto out;
	}

	cd->rx = q;

	if (q)
		out8(&priv->reg[NS16550_IER],
		     in8(&priv->reg[NS16550_IER]) | NS16550_IER_ERDAI);
	else
		out8(&priv->reg[NS16550_IER],
		     in8(&priv->reg[NS16550_IER]) & ~NS16550_IER_ERDAI);

out:
	spin_unlock_intsave(&priv->lock, saved);
	return ret;
}
#endif

static ssize_t ns16550_tx(chardev_t *cd, const uint8_t *buf,
                          size_t count, int flags)
{
	ns16550 *priv = to_container(cd, ns16550, cd);
	size_t ret = 0;
	int i;

	do {
		while (!(in8(&priv->reg[NS16550_LSR]) & NS16550_LSR_THRE))
			if (!(flags & CHARDEV_BLOCKING))
				return ret;

		unsigned long saved = spin_lock_intsave(&priv->lock);

		if (!(in8(&priv->reg[NS16550_LSR]) & NS16550_LSR_THRE)) {
			spin_unlock_intsave(&priv->lock, saved);
			continue;
		}

		for (i = 0; i < priv->txfifo && ret < count; i++) {
			priv->tx_counter++;
			out8(&priv->reg[NS16550_THR], buf[ret++]);
		}

		spin_unlock_intsave(&priv->lock, saved);
	} while (ret < count);

	return ret;
}

static const chardev_ops ops = {
	.tx = ns16550_tx,
#if defined(INTERRUPTS) && defined(CONFIG_LIBOS_QUEUE)
	.set_tx_queue = ns16550_set_tx_queue,
	.set_rx_queue = ns16550_set_rx_queue,
#endif
};

/** Initialize a ns16550 UART.
 *
 * This function does not modify baud rate, parity, etc.
 * Use \ref ns16550_config for that.
 *
 * @param[in] reg virtual address of UART registers
 * @param[in] irq IRQ number of UART
 * @param[in] baudclock frequency to be divided to produce baud rate
 * @param[in] txfifo size of the transmit fifo, or 1 to disable.
 */
chardev_t *ns16550_init(uint8_t *reg, interrupt_t *irq,
                        int baudclock, int txfifo, int baud)
{
	ns16550 *priv;
	int divisor;

	priv = alloc_type(ns16550);
	if (!priv)
		return NULL;

	priv->cd.ops = &ops;
	priv->reg = reg;
	priv->txfifo = txfifo;

	baud *= 16;
	divisor = (baudclock + baud / 2) / baud;

	out8(&priv->reg[NS16550_LCR], NS16550_LCR_DLAB);

	out8(&priv->reg[NS16550_DMB], (divisor >> 8) & 0xff);
	out8(&priv->reg[NS16550_DLB], divisor & 0xff);

	out8(&priv->reg[NS16550_LCR], NS16550_LCR_8BIT);

	out8(&priv->reg[NS16550_FCR],
	     NS16550_FCR_FEN | NS16550_FCR_RFR | NS16550_FCR_TFR);

	in8(&priv->reg[NS16550_RBR]);  /* dummy read to clear any pending interrupts */

	out8(&priv->reg[NS16550_MCR],
	     in8(&priv->reg[NS16550_MCR]) | NS16550_MCR_RTS);

	out8(&priv->reg[NS16550_IER], 0);

#if defined(INTERRUPTS) && defined(CONFIG_LIBOS_QUEUE)
	if (irq && irq->ops->register_irq &&
		 irq->ops->register_irq(irq, ns16550_isr, priv, TYPE_CRIT) == 0)
		priv->irq = irq;
#endif

	return &priv->cd;
}
