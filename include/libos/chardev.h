#ifndef LIBOS_CHARDEV_H
#define LIBOS_CHARDEV_H

#include <stdint.h>

#include <libos/queue.h>

struct chardev_t;

#define CHARDEV_BLOCKING  0x0001

/// Character device operations.
typedef struct {
	/** Transmit data.
	 *
	 * @param[in] dev The character device instance.
	 * @param[in] buf The data to send.
	 * @param[in] count The number of bytes to send.
	 *
	 * @param flags
	 * If CHARDEV_BLOCKING is set, the function will not return until
	 * the request can be completed.
	 *
	 * @return -1 on error, 0 if CHARDEV_BLOCKING is not set and the
	 * request cannot be completed immediately.  Otherwise, the number of
	 * bytes transmitted is returned.
	 *
	 * This pointer may be NULL if the device does not support
	 * polled transmit.  The driver must set @ref chardevrx "dev->rx".
	 */
	ssize_t (*tx)(struct chardev_t *dev, const uint8_t *buf,
	              size_t count, int flags);

	/** Receive data.
	 *
	 * @param[in] dev The character device instance.
	 * @param[out] buf The buffer to receive the data.
	 * @param[in] count The size of the buffer.
	 *
	 * @param flags
	 * If CHARDEV_BLOCKING is set, the function will not return until
	 * the request can be completed.
	 *
	 * @return -1 on error, 0 if CHARDEV_BLOCKING is not set and the
	 * request cannot be completed immediately.  Otherwise, the number of
	 * bytes received is returned.
	 *
	 * This pointer may be NULL if the device does not support
	 * polled receive.  The driver must set @ref chardevtx "dev->tx".
	 */
	ssize_t (*rx)(struct chardev_t *dev, uint8_t *buf, size_t count, int flags);

	/** Start or stop interrupt driven operation on a receive queue.
	 *
	 * @param[in] dev The character device instance.
	 * @param[in] q The queue to transmit from, or NULL to stop tx interrupts.
	 *
	 * @return zero on success, -1 on error.
	 *
	 * This pointer may be NULL if the device does not support
	 * interrupt driven transmit.  The driver must set @ref chardevtx "dev->tx".
	 */
	int (*set_tx_queue)(struct chardev_t *dev, queue_t *q);

	/** Start or stop interrupt driven operation on a receive queue.
	 *
	 * @param[in] dev The character device instance.
	 * @param[in] q The queue to receive into, or NULL to stop rx interrupts.
	 *
	 * @return zero on success, -1 on error.
	 *
	 * This pointer may be NULL if the device does not support
	 * interrupt driven receive.  The driver must set @ref chardevrx "dev->rx".
	 */
	int (*set_rx_queue)(struct chardev_t *dev, queue_t *q);
} chardev_ops;

/// Represents a device which can transmit and receive a byte stream.
typedef struct chardev_t {
	const chardev_ops *ops;

	/// @anchor chardevtx
	/// The queue to receive data into, if interrupt driven.
	queue_t *tx;

	/// @anchor chardevrx
	/// The queue to receive data into, if interrupt driven.
	queue_t *rx;
} chardev_t;

#endif
