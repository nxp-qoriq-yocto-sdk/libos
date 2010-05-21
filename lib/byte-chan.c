/** @file character device driver for byte channel hcalls
 *
 * Polling only, no interrupt support yet.
 */
/*
 * Copyright (C) 2009,2010 Freescale Semiconductor, Inc.
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

#include <libos/chardev.h>
#include <libos/epapr_hcalls.h>
#include <libos/byte-chan.h>
#include <libos/interrupts.h>
#include <libos/libos.h>
#include <libos/alloc.h>

typedef struct byte_chan {
	chardev_t cd;
	interrupt_t *rxirq, *txirq;
	int handle;
} byte_chan_t;

static ssize_t byte_chan_rx(chardev_t *cd, uint8_t *buf,
                            size_t count, int flags)
{
	byte_chan_t *priv = to_container(cd, byte_chan_t, cd);
	size_t total = 0;
	int ret;

	while (count > 0) {
		unsigned int this_count = min(count, 16U);
	
		ret = ev_byte_channel_receive(priv->handle, &this_count, (char *)buf);
		if (ret)
			return total == 0 ? ret : (ssize_t)total;

		if (this_count == 0 && (flags & CHARDEV_BLOCKING))
			continue;

		buf += this_count;
		total += this_count;
		count -= this_count;
	}

	return total;
}

static ssize_t byte_chan_tx(chardev_t *cd, const uint8_t *buf,
                            size_t count, int flags)
{
	byte_chan_t *priv = to_container(cd, byte_chan_t, cd);
	size_t total = 0;
	int ret = 0;

	while (count > 0) {
		unsigned int sent = min(count, 16U);

		ret = ev_byte_channel_send(priv->handle, &sent, (const char *)buf);

		if (sent == 0 && !(ret == EV_EAGAIN && (flags & CHARDEV_BLOCKING)))
			break;

		buf += sent;
		total += sent;
		count -= sent;
	}

	if (ret)
		return total == 0 ? ret : (ssize_t)total;
	else
		return total;
}

static const chardev_ops ops = {
	.tx = byte_chan_tx,
	.rx = byte_chan_rx,
};

chardev_t *byte_chan_init(int handle, interrupt_t *rxirq, interrupt_t *txirq)
{
	byte_chan_t *priv = alloc_type(byte_chan_t);
	if (!priv)
		return NULL;

	priv->cd.ops = &ops;
	priv->handle = handle;
	priv->rxirq = rxirq;
	priv->txirq = txirq;

	return &priv->cd;
}
