/** @file
 * Lockless single-producer, single-consumer queues.
 */
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

#include <libos/libos.h>
#include <libos/queue.h>
#include <libos/io.h>
#include <libos/errors.h>
#include <libos/bitops.h>

#include <string.h>


int queue_init(queue_t *q, size_t size)
{
	q->buf = alloc(size, 1);
	if (!q->buf)
		return ERR_NOMEM;

	q->head = 0;
	q->tail = 0;
	q->size = size;

	return 0;
};

void queue_destroy(queue_t *q)
{
#ifdef CONFIG_LIBOS_MALLOC
	free(q->buf);
#endif

	q->buf = NULL;
}

ssize_t queue_read(queue_t *q, uint8_t *buf, size_t len)
{
	size_t orig_len = len;

	if (q->head == q->tail)
		/* queue is empty */
		return -1;

	if (q->tail < q->head) {
		size_t chunk = q->size - q->head;
		if (chunk > len)
			chunk = len;

		memcpy(buf, &q->buf[q->head], chunk);

		smp_mbar();

		raw_out32(&q->head, queue_wrap(q, q->head + chunk));

		buf += chunk;
		len -= chunk;
	}

	if (len > 0) {
		size_t chunk = q->tail - q->head;
		if (chunk > len)
			chunk = len;

		memcpy(buf, &q->buf[q->head], chunk);

		smp_mbar();
		raw_out32(&q->head, q->head + chunk);

		len -= chunk;
	}

	return orig_len - len;
}

int queue_readchar(queue_t *q)
{
	if (q->head == q->tail)
		/* queue is empty */
		return -1;

	int ret = q->buf[q->head];
	smp_lwsync();
	raw_out32(&q->head, queue_wrap(q, q->head + 1));
	return ret;
}

ssize_t queue_write(queue_t *q, const uint8_t *buf, size_t len)
{
	size_t orig_len = len;
	size_t end = queue_wrap(q, q->head - 1);

	if (q->tail == end)
		/* queue is full */
		return -1;
	
	if (end < q->tail) {
		size_t chunk = q->size - q->tail;
		if (chunk > len)
			chunk = len;

		memcpy(&q->buf[q->tail], buf, chunk);

		smp_mbar();
		raw_out32(&q->tail, queue_wrap(q, q->tail + chunk));

		buf += chunk;
		len -= chunk;
	}

	if (len > 0) {
		size_t chunk = end - q->tail;
		if (chunk > len)
			chunk = len;

		memcpy(&q->buf[q->tail], buf, chunk);

		smp_mbar();
		raw_out32(&q->tail, q->tail + chunk);

		len -= chunk;
	}

	return orig_len - len;
}

int queue_writechar(queue_t *q, uint8_t c)
{
	if (q->tail == queue_wrap(q, q->head - 1))
		/* queue is full */
		return -1;

	q->buf[q->tail] = c;
	smp_mbar();
	raw_out32(&q->tail, queue_wrap(q, q->tail + 1));
	return 0;
}

size_t qprintf(queue_t *q, const char *str, ...)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	static uint32_t lock;
	size_t i;

	register_t saved = spin_lock_critsave(&lock);

	va_list args;
	va_start(args, str);
	size_t ret = vsnprintf(buffer, buffer_size, str, args);
	va_end(args);
	
	if (ret > buffer_size)
		ret = buffer_size;
	
	for (i = 0; i < ret; i++) {
		/* This is needed if the destination is a serial port.
		 * Is it safe for all destinations?
		 */
		if (buffer[i] == '\n')	
			queue_writechar(q, '\r');
			
		queue_writechar(q, buffer[i]);
	}

	spin_unlock_critsave(&lock, saved);
	return ret;
}
