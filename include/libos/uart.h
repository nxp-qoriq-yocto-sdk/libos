#ifndef LIBOS_UART_H
#define LIBOS_UART_H

#include <stdint.h>

typedef struct {
	int data_bits;
	int stop_bits;
	int parity; 
	int baudrate;
} uart_param_t;

#endif
