#ifndef LIBOS_UART_H
#define LIBOS_UART_H

#include <stdint.h>

void uart_init(unsigned long);
void uart_putc(uint8_t c);

#endif
