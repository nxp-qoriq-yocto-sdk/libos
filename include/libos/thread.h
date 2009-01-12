/** @file
 * Thread switching
 */

#ifndef LIBOS_THREAD_H
#define LIBOS_THREAD_H

typedef struct thread {
	void *stack;
	void *pc;
} thread_t;

/** Switch threads.
 *
 * @param[in] new_thread thread to switch to
 * @return thread that was switched from
 */
thread_t *switch_thread(thread_t *new_thread);

#endif
