
#include <libos/uart_defs.h>
#include <libos/io.h>
#include <libos/uart.h>

void uart_init(void)
{
}

#ifndef UART_OFFSET
#define UART_OFFSET 0x11c500
#endif

void uart_putc(uint8_t c) 
{
	uint8_t *addr = (uint8_t *)(0x01000000 + UART_OFFSET + REG_DATA);

	while (!(in8(addr + 5) & 0x20));
	out8(addr, c);
}
