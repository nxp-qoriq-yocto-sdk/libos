/** @file
 * Lockless single-producer, single-consumer queues.
 */

#ifndef LIBOS_QUEUE_H
#define LIBOS_QUEUE_H

#include <stdint.h>
#include <string.h>

/// Lockless single-producer, single-consumer queue.
typedef struct queue {
	uint8_t *buf;
	size_t head, tail;
	
	/// Size of queue, must be a power of two.
	size_t size;

	/** Producer callback to indicate space available.
	 *
	 * @param[in] q the queue with space available.
	 *
	 * This pointer should be NULL when a callback is not requested. 
	 * This function must not be called from within a space_avail()
	 * callback for the same queue, only from asynchronous contexts. 
	 * This function may be called from interrupt context.
	 */
	void (*volatile data_avail)(struct queue *q);

	/// Private data for the producer.
	void *producer;

	/** Consumer callback to indicate data available.
	 *
	 * @param[in] q the queue with data available.
	 *
	 * This pointer should be NULL when a callback is not requested. 
	 * This function must not be called from within a data_avail()
	 * callback for the same queue, only from asynchronous contexts. 
	 * This function may be called from interrupt context.
	 */
	void (*volatile space_avail)(struct queue *q);

	/// Private data for the consumer.
	void *consumer;
} queue_t;

/** Initialize a queue.
 *
 * @param[in] q address of the queue to initialize.
 * @param[in] size size in bytes of the queue, must be a power of two.
 */
int queue_init(queue_t *q, size_t size);

void queue_destroy(queue_t *q);
ssize_t queue_read(queue_t *q, uint8_t *buf, size_t len);
ssize_t queue_write(queue_t *q, const uint8_t *buf, size_t len);
int queue_readchar(queue_t *q);
int queue_writechar(queue_t *q, uint8_t c);
size_t qprintf(queue_t *q, const char *str, ...)
	__attribute__((format(printf, 2, 3)));

static inline void queue_notify_consumer(queue_t *q)
{
	void (*callback)(struct queue *q) = q->data_avail;
	
	if (callback)
		callback(q);
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
	return queue_wrap(q, q->tail - q->head);
}

static inline size_t queue_get_space(const queue_t *q)
{
	return q->size - queue_get_avail(q) - 1;
}

static inline int queue_empty(const queue_t *q)
{
	return q->head == q->tail;
}

static inline int queue_full(const queue_t *q)
{
	return queue_wrap(q, q->head + 1) == q->tail;
}

static inline ssize_t queue_writestr(queue_t *q, const char *str)
{
	return queue_write(q, (const uint8_t *)str, strlen(str));
}

#endif
