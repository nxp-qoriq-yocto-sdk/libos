/** @file
 * Lockless single-producer, single-consumer queues.
 */
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

#include <string.h>

#include <libos/queue.h>
#include <libos/alloc.h>
#include <libos/io.h>
#include <libos/errors.h>
#include <libos/bitops.h>
#include <libos/chardev.h>
#include <libos/percpu.h>

int queue_init(queue_t *q, size_t size)
{
	if (size & (size - 1))
		return ERR_INVALID;

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
	free(q->buf);
	q->buf = NULL;
}

void queue_read_at(queue_t *q, uint8_t *buf, size_t off, size_t len)
{
	off = queue_wrap(q, q->head + off);
	size_t first = min(len, q->size - off);

	memcpy(buf, &q->buf[off], first);
	len -= first;

	if (len > 0)
		memcpy(buf + first, &q->buf[0], len);
}

ssize_t queue_read(queue_t *q, uint8_t *buf, size_t len, int peek)
{
	size_t head = q->head;
	size_t tail = raw_in32(&q->tail);
	size_t avail = queue_wrap(q, tail - head);

	len = min(avail, len);
	queue_read_at(q, buf, 0, len);

	if (!peek) {
		smp_lwsync();
		raw_out32(&q->head, queue_wrap(q, head + len));
	}

	return len;	
}

#ifdef CONFIG_LIBOS_SCHED_API
ssize_t queue_read_blocking(queue_t *q, uint8_t *buf, size_t len, int peek)
{
	ssize_t orig_len = len;

	while (len > 0)  {
		libos_prepare_to_block();

		ssize_t ret = queue_read(q, &buf[orig_len - len], len, 0);

		if (ret > 0) {
			len -= ret;
			queue_notify_producer(q);
			continue;
		}

		libos_block();
	}

	return orig_len;
}

int queue_readchar_blocking(queue_t *q, int peek)
{
	int ret;
	while (1) {
		libos_prepare_to_block();

		ret = queue_readchar(q, peek);
		if (ret >= 0)
			break;

		assert(ret == ERR_WOULDBLOCK);
		libos_block();
	}

	queue_notify_producer(q);

	return ret;
}
#endif

int queue_readchar(queue_t *q, int peek)
{
	if (q->head == raw_in32(&q->tail))
		/* queue is empty */
		return ERR_WOULDBLOCK;

	int ret = q->buf[q->head];

	if (!peek) {
		smp_lwsync();
		raw_out32(&q->head, queue_wrap(q, q->head + 1));
	}

	return ret;
}

static void queue_write_at(queue_t *q, const uint8_t *buf,
                           size_t off, size_t len)
{
	size_t first = min(len, q->size - off);

	memcpy(&q->buf[off], buf, first);
	len -= first;

	if (len > 0)
		memcpy(&q->buf[0], buf + first, len);
}

ssize_t queue_write(queue_t *q, const uint8_t *buf, size_t len)
{
	size_t tail = q->tail;
	size_t head = raw_in32(&q->head);
	size_t space = queue_wrap(q, head - tail - 1);

	len = min(space, len);
	queue_write_at(q, buf, tail, len);

	smp_lwsync();
	raw_out32(&q->tail, queue_wrap(q, tail + len));

	return len;	
}

#ifdef CONFIG_LIBOS_SCHED_API
ssize_t queue_write_blocking(queue_t *q, const uint8_t *buf, size_t len)
{
	ssize_t orig_len = len;

	while (len > 0) {
		libos_prepare_to_block();

		ssize_t ret = queue_write(q, buf, len);
		if (ret > 0) {
			len -= ret;
			queue_notify_consumer(q, 0);
			continue;
		}

		libos_block();
	}

	libos_unblock(cpu->thread);
	return orig_len;
}
#endif

int queue_writechar(queue_t *q, uint8_t c)
{
	if (q->tail == queue_wrap(q, raw_in32(&q->head) - 1))
		/* queue is full */
		return ERR_BUSY;

	q->buf[q->tail] = c;
	smp_lwsync();
	raw_out32(&q->tail, queue_wrap(q, q->tail + 1));
	return 0;
}

#ifdef CONFIG_LIBOS_SCHED_API
int queue_writechar_blocking(queue_t *q, uint8_t c)
{
	while (1) {
		libos_prepare_to_block();

		int ret = queue_writechar(q, c);
		if (ret == 0)
			break;
		
		assert(ret == ERR_BUSY);
		libos_block();
	}

	queue_notify_consumer(q, 0);
	libos_unblock(cpu->thread);
	return 0;
}

size_t qprintf(queue_t *q, int blocking, const char *str, ...)
{
	enum {
		buffer_size = 4096,
	};

	static char buffer[buffer_size];
	static uint32_t lock;
	size_t i;

	register_t saved = spin_lock_intsave(&lock);

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

		if (blocking) {
			if (buffer[i] == '\n')	
				queue_writechar_blocking(q, '\r');
			
			queue_writechar_blocking(q, buffer[i]);
		} else {
			if (buffer[i] == '\n')	
				queue_writechar(q, '\r');
			
			queue_writechar(q, buffer[i]);
		}
	}

	spin_unlock_intsave(&lock, saved);
	return ret;
}
#endif

ssize_t queue_to_chardev(chardev_t *dest, queue_t *src,
                         size_t len, int peek, int flags)
{
	size_t orig_len = len;
	size_t head = src->head;
	size_t tail = raw_in32(&src->tail);

	if (head == tail)
		/* queue is empty */
		return 0;

	if (tail < head) {
		size_t chunk = src->size - head;
		if (chunk > len)
			chunk = len;

		dest->ops->tx(dest, &src->buf[head], chunk, flags);

		head = queue_wrap(src, head + chunk);
		len -= chunk;

		if (!peek) {
			smp_lwsync();
			raw_out32(&src->head, head);
		}
	}

	if (len > 0) {
		size_t chunk = tail - head;
		if (chunk > len)
			chunk = len;

		dest->ops->tx(dest, &src->buf[head], chunk, flags);

		head = queue_wrap(src, head + chunk);
		len -= chunk;

		if (!peek) {
			smp_lwsync();
			raw_out32(&src->head, head);
		}
	}

	return orig_len - len;
}

ssize_t queue_to_queue(queue_t *dest, queue_t *src,
                       size_t len, int peek, int blocking)
{
	size_t orig_len = len;
	size_t head = src->head;
	size_t tail = src->tail;
	ssize_t ret;

	if (head == tail)
		/* queue is empty */
		return 0;

	if (tail < head) {
		size_t chunk = src->size - head;
		if (chunk > len)
			chunk = len;

#ifdef CONFIG_LIBOS_SCHED_API
		if (blocking)
			ret = queue_write_blocking(dest, &src->buf[head], chunk);
		else
#endif
			ret = queue_write(dest, &src->buf[head], chunk);

		if (ret < 0)
			return ret;

		head = queue_wrap(src, head + ret);
		len -= ret;

		if (!peek) {
			smp_lwsync();
			raw_out32(&src->head, head);
		}

		if ((size_t)ret < chunk)
			return ret;
	}

	if (len > 0) {
		size_t chunk = tail - head;
		if (chunk > len)
			chunk = len;

#ifdef CONFIG_LIBOS_SCHED_API
		if (blocking)
			ret = queue_write_blocking(dest, &src->buf[head], chunk);
		else
#endif
			ret = queue_write(dest, &src->buf[head], chunk);

		if (ret < 0)
			return ret;

		head = queue_wrap(src, head + ret);
		len -= ret;

		if (!peek) {
			smp_lwsync();
			raw_out32(&src->head, head);
		}
	}

	return orig_len - len;
}

size_t queue_discard(queue_t *q, size_t num)
{
	size_t avail = queue_get_avail(q);

	if (num > avail)
		num = avail;

	raw_out32(&q->head, queue_wrap(q, q->head + num));
	return num;
}

ssize_t queue_memchr(queue_t *q, int c, size_t start)
{
	uint32_t pos = queue_wrap(q, q->head + start);
	uint32_t tail = raw_in32(&q->tail);
	uint8_t *ret;

	if (pos > tail) {
		ret = memchr(&q->buf[pos], c, q->size - q->head);
		if (ret)
			return ret - &q->buf[q->head];

		pos = 0;
	}

	ret = memchr(&q->buf[pos], c, tail - q->head);
	if (ret)
		return queue_wrap(q, ret - &q->buf[q->head]);

	return -1;
}
