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

#ifndef LIBOS_QUEUE_H
#define LIBOS_QUEUE_H

#include <libos/types.h>
#include <libos/io.h>
#include <string.h>

/// Lockless single-producer, single-consumer queue.
typedef struct queue {
	uint8_t *buf;
	uint32_t head, tail;
	
	/// Size of queue, must be a power of two.
	uint32_t size;

	/** Consumer callback to indicate data available.
	 *
	 * @param[in] q the queue with data available.
	 * @param[in] blocking
	 *    If non-NULL, do not return until all data has been
	 *    processed that was present at the beginning of the call, if
	 *    possible.  This is not the thread-based blocking as in
	 *    queue_write_blocking, but spinning as in CHARDEV_BLOCKING.
	 *
	 * This pointer should be NULL when a callback is not requested. 
	 * This function must not be called from within a space_avail()
	 * callback for the same queue, only from asynchronous contexts. 
	 * This function may be called from interrupt context.
	 */
	void (*volatile data_avail)(struct queue *q, int blocking);

	/// Private data for the consumer.
	void *consumer;

	/** Producer callback to indicate space available.
	 *
	 * @param[in] q the queue with space available.
	 *
	 * This pointer should be NULL when a callback is not requested. 
	 * This function must not be called from within a data_avail()
	 * callback for the same queue, only from asynchronous contexts. 
	 * This function may be called from interrupt context.
	 */
	void (*volatile space_avail)(struct queue *q);

	/// Private data for the producer.
	void *producer;
} queue_t;

/** Initialize a queue.
 *
 * @param[in] q address of the queue to initialize.
 * @param[in] size size in bytes of the queue, must be a power of two.
 */
int queue_init(queue_t *q, size_t size);

void queue_destroy(queue_t *q);

/** Read from a specific offset in the queue.
 *
 * This requires consumer synchronization.  Reads "len" bytes from the
 * specified offset from the queue head.  The caller must ensure that at
 * least "off + len" bytes are available in the queue.  The data will not be
 * removed from the queue.
 *
 * @param[in] q address of the queue to read from.
 * @param[out] buf buffer to fill
 * @param[in] off offset from head to read from
 * @param[in] len maximum number of bytes to read
 * @return number of bytes read, or zero if queue is empty
 */
void queue_read_at(queue_t *q, uint8_t *buf, size_t off, size_t len);

/** Read from a queue.
 *
 * This requires consumer synchronization.  Reads up to "len" bytes from
 * the queue.
 *
 * @param[in] q address of the queue to read from.
 * @param[out] buf buffer to fill
 * @param[in] len maximum number of bytes to read
 * @param[in] peek If non-zero, copy rather than move the bytes from
 *                 the queue.
 * @return number of bytes read, or zero if queue is empty
 */
ssize_t queue_read(queue_t *q, uint8_t *buf, size_t len, int peek);

/** Read from a queue, blocking until all requested bytes are available.
 *
 * This requires consumer synchronization.  Reads "len" bytes from the
 * queue.  This must be called from a thread context which can block, and
 * will not return until all requested bytes have been read.
 *
 * @param[in] q address of the queue to read from.
 * @param[out] buf buffer to fill
 * @param[in] len maximum number of bytes to read
 * @param[in] peek If non-zero, copy rather than move the bytes from
 *                 the queue.  This can deadlock if "len" is larger
 *                 than q->size - 1, or possibly less if the queue
 *                 is not written to with byte granularity.
 * @return number of bytes read
 */
ssize_t queue_read_blocking(queue_t *q, uint8_t *buf, size_t len, int peek);

/** Write to a queue.
 *
 * This requires producer synchronization.  Writes up to "len" bytes to
 * the queue.
 *
 * @param[in] q address of the queue to write to.
 * @param[in] buf buffer to read from
 * @param[in] len maximum number of bytes to write
 * @return number of bytes written, or zero if queue is full
 */
ssize_t queue_write(queue_t *q, const uint8_t *buf, size_t len);

/** Write to a queue, blocking until all provided bytes are written.
 *
 * This requires producer synchronization.  Writes "len" bytes to
 * the queue.  This must be called from a thread context which can
 * block, and will not return until all requested bytes have been written.
 *
 * @param[in] q address of the queue to write to.
 * @param[in] buf buffer to read from
 * @param[in] len maximum number of bytes to write
 * @return number of bytes written
 */
ssize_t queue_write_blocking(queue_t *q, const uint8_t *buf, size_t len);

int queue_readchar(queue_t *q, int peek);
int queue_readchar_blocking(queue_t *q, int peek);
int queue_writechar(queue_t *q, uint8_t c);
int queue_writechar_blocking(queue_t *q, uint8_t c);
size_t qprintf(queue_t *q, int blocking, const char *str, ...)
	__attribute__((format(printf, 3, 4)));

/** Find a character in a queue.
 *
 * This requires consumer synchronization.  The caller must ensure that
 * there are at least "start" bytes on the queue.
 *
 * @param[in] q address of the queue to search.
 * @param[in] c byte to search for
 * @param[in] start Offset from head to search from, or zero for whole queue
 * @return Offset from head to first instance of character no earler
 *         than start, or negative if not found.
 */
ssize_t queue_memchr(queue_t *q, int c, size_t start);

static inline void queue_notify_consumer(queue_t *q, int blocking)
{
	void (*callback)(struct queue *q, int blocking) = q->data_avail;
	
	if (callback)
		callback(q, blocking);
}

static inline void queue_notify_producer(queue_t *q)
{
	void (*callback)(struct queue *q) = q->space_avail;
	
	if (callback)
		callback(q);
}

static inline size_t queue_wrap(const queue_t *q, size_t index)
{
	return index & (q->size - 1);
}

static inline size_t queue_get_avail(const queue_t *q)
{
	return queue_wrap(q, raw_in32(&q->tail) - raw_in32(&q->head));
}

static inline size_t queue_get_space(const queue_t *q)
{
	return queue_wrap(q, raw_in32(&q->head) - raw_in32(&q->tail) - 1);
}

static inline int queue_empty(const queue_t *q)
{
	return raw_in32(&q->head) == raw_in32(&q->tail);
}

static inline int queue_full(const queue_t *q)
{
	return queue_get_space(q) == 0;
}

static inline ssize_t queue_writestr(queue_t *q, const char *str)
{
	return queue_write(q, (const uint8_t *)str, strlen(str));
}

/** Purge all data from a queue.
 *
 * This requires consumer synchronization.  Consumes all available
 * data without needing a buffer to copy it to.
 *
 * @param[in] q address of the queue to purge.
 */
static inline void queue_purge(queue_t *q)
{
	raw_out32(&q->head, raw_in32(&q->tail));
}

struct chardev;

ssize_t queue_to_queue(queue_t *dest, queue_t *src, size_t len,
                       int peek, int blocking);
ssize_t queue_to_chardev(struct chardev *dest, queue_t *src,
                         size_t len, int peek, int flags);

size_t queue_discard(queue_t *q, size_t num);

#define DECLARE_QUEUE(Q, SIZE) \
	static uint8_t _##Q##_array[SIZE]; \
	queue_t Q = { \
	.buf = _##Q##_array, \
	.size = SIZE, \
}

#define DECLARE_STATIC_QUEUE(Q, SIZE) \
	static uint8_t _##Q##_array[SIZE]; \
	static queue_t Q = { \
	.buf = _##Q##_array, \
	.size = SIZE, \
}

#endif
