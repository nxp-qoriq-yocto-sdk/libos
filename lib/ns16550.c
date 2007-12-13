
#include <libos/uart_defs.h>
#include <libos/io.h>
#include <libos/uart.h>

unsigned long uart_vaddr = 0;

void uart_init(unsigned long vaddr)
{
	uart_vaddr = vaddr;
}

void uart_putc(uint8_t c) 
{
	uint8_t *addr;

	if (uart_vaddr == 0) {
		return;
	}

	addr = (uint8_t *)(uart_vaddr + REG_DATA);

	while (!(in8(addr + 5) & 0x20));
	out8(addr, c);
}
