#ifndef _READLINE_H
#define _READLINE_H

#include <libos/chardev.h>
#include <libos/queue.h>

/** Readline action handler; returns non-zero to discontinue session. */
typedef int (*rl_action_t)(char *buf);
typedef struct readline readline_t;

readline_t *readline_init(queue_t *in, queue_t *out,
                          const char *prompt, rl_action_t action);
void readline_suspend(readline_t *rl);
void readline_resume(readline_t *rl);

extern readline_t *rl_console;

#endif
