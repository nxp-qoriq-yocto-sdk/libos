#ifndef LIBOS_CONSOLE_H
#define LIBOS_CONSOLE_H

#include <stdint.h>
#include <libos/chardev.h>

void console_init(chardev_t *cd);
int putchar(int c);
int puts(const char *s);
void puts_len(const char *s, size_t len);
size_t printf(const char *str, ...);
extern int crashing;

#endif
