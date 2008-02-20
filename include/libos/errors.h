/** @file
 * Error definitions.
 *
 * These start at -256 to avoid conflicts with libfdt errors.
 */

#ifndef LIBOS_ERRORS_H
#define LIBOS_ERRORS_H

#define ERR_BADTREE          (-256) /**< Semantic error in device tree. */
#define ERR_NOMEM            (-257) /**< Out of memory. */
#define ERR_NOTRANS          (-258) /**< No translation possible. */
#define ERR_BUSY             (-259) /**< Resource busy. */
#define ERR_INVALID          (-260) /**< Invalid request or argument. */

#endif
