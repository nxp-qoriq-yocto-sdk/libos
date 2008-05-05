#ifndef LIBOS_CONSOLE_H
#define LIBOS_CONSOLE_H

#include <stdarg.h>
#include <stdint.h>
#include <libos/chardev.h>

void console_init(chardev_t *cd);
void qconsole_init(queue_t *q);
int putchar(int c);
int puts(const char *s);
void puts_len(const char *s, size_t len);
size_t printf(const char *str, ...);
size_t vprintf(const char *str, va_list args);
extern int crashing;

#define NUM_LOGTYPES 256
#define LIBOS_BASE_LOGTYPE  0
#define CLIENT_BASE_LOGTYPE 64

#define LOGTYPE_MISC      0
#define LOGTYPE_MMU       1
#define LOGTYPE_IRQ       2
#define LOGTYPE_MP        3

#define MAX_LOGLEVEL 15
#define LOGLEVEL_ALWAYS   0
#define LOGLEVEL_ERROR    2
#define LOGLEVEL_NORMAL   4
#define LOGLEVEL_DEBUG    8
#define LOGLEVEL_VERBOSE 12

extern uint8_t loglevels[NUM_LOGTYPES];
extern void invalid_logtype(void);

/* Unfortunately, GCC will not inline a varargs function. */
#define printlog(logtype, loglevel, fmt, args...) do { \
	/* Force a linker error if used improperly. */ \
	if (logtype >= NUM_LOGTYPES || loglevel > MAX_LOGLEVEL) \
		invalid_logtype(); \
	\
	if (loglevel <= CONFIG_LIBOS_MAX_BUILD_LOGLEVEL && \
	    (loglevel == 0 || loglevels[logtype] >= loglevel)) \
		printf(fmt, ##args); \
} while (0)
		
#endif
