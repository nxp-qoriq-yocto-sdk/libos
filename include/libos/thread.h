/** @file
 * Thread switching
 */

#ifndef LIBOS_THREAD_H
#define LIBOS_THREAD_H

typedef struct libos_thread {
	void *stack;
	void *pc;
} libos_thread_t;

/** Switch threads.
 *
 * @param[in] new_thread thread to switch to
 * @return thread that was switched from
 */
libos_thread_t *switch_thread(libos_thread_t *new_thread);

void libos_prepare_to_block(void);
void libos_block(void);
void libos_unblock(libos_thread_t *thread);

#endif
