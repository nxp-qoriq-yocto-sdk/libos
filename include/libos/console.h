#ifndef LIBOS_CONSOLE_H
#define LIBOS_CONSOLE_H

#include <stdint.h>

void console_init(void);
int putchar(int c);
int puts(const char *s);
void puts_len(const char *s, int len);
size_t printf(const char *str, ...);

#endif
