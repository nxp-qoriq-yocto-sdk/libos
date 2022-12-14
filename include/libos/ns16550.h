/*!
 *  @file
 *  @brief  16550-family UART definitions
 */

/*
 * Copyright (C) 2008,2009 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NS16550_H
#define NS16550_H

#include <stdint.h>

#define NS16550_RBR  0x00 // receiver buffer register
#define NS16550_THR  0x00 // transmitter holding register
#define NS16550_DLB  0x00 // divisor least significant byte register
#define NS16550_IER  0x01 // interrupt enable register
#define NS16550_DMB  0x01 // divisor most significant byte register
#define NS16550_IIR  0x02 // interrupt ID register
#define NS16550_FCR  0x02 // FIFO control register
#define NS16550_AFR  0x02 // alternate function register
#define NS16550_LCR  0x03 // line control register
#define NS16550_MCR  0x04 // modem control register
#define NS16550_LSR  0x05 // line status register
#define NS16550_MSR  0x06 // modem status register
#define NS16550_SCR  0x07 // scratch register
#define NS16550_DSR  0x10 // DMA status register

#define NS16550_IIR_NOIRQ  0x01
#define NS16550_IIR_IID    0x0e
#define NS16550_IIR_RXTIME 0x0c // rx time-out
#define NS16550_IIR_RLSI   0x06 // receiver line status
#define NS16550_IIR_RDAI   0x04 // receiver data available
#define NS16550_IIR_THREI  0x02 // transmitter holding register empty
#define NS16550_IIR_MSI    0x00 // modem status

#define NS16550_LSR_RFE    0x80
#define NS16550_LSR_OE     0x20
#define NS16550_LSR_DR     0x01 // Data ready
#define NS16550_LSR_THRE   0x20 // transmitter holding register empty
#define NS16550_LSR_TEMT   0x40 // transmitter empty

#define NS16550_MCR_RTS    0x02 // Ready to send
#define NS16550_MCR_LOOP   0x10 // Loop enable

#define NS16550_MSR_CTS    0x10 // CTS status

#define NS16550_IER_ERDAI  0x01 // received data available
#define NS16550_IER_ETHREI 0x02 // transmitter holding register empty
#define NS16550_IER_ERLSI  0x04 // receiver line status interrupt
#define NS16550_IER_EMSI   0x08 // modem status interrput

#define NS16550_LCR_8BIT   0x03
#define NS16550_LCR_NTSB   0x04 // number of stop bits
#define NS16550_LCR_EPS    0x10 // Even parity selected
#define NS16550_LCR_PEN    0x08 // Parity enable
#define NS16550_LCR_SP     0x20 // Stick parity
#define NS16550_LCR_DLAB   0x80 // Divisor latch access bit

#define NS16550_FCR_FEN    0x01 // Enable FIFO
#define NS16550_FCR_RFR    0x02 // Receiver FIFO reset
#define NS16550_FCR_TFR    0x04 // Transmitter FIFO clear
#define NS16550_FCR_DMS    0x08 // DMA mode select
#define NS16550_FCR_RX1    0x00 // RX threshold 1 byte
#define NS16550_FCR_RX4    0x40 // RX threshold 4 bytes
#define NS16550_FCR_RX8    0x80 // RX threshold 8 bytes
#define NS16550_FCR_RX14   0xc0 // RX threshold 14 bytes

struct interrupt;

struct chardev *ns16550_init(uint8_t *reg, struct interrupt *irq,
                             int baudclock, int txfifo, int baud);

#endif
