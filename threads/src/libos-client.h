/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#ifndef LIBOS_CLIENT_H
#define LIBOS_CLIENT_H

#define MAX_CORES 12

// PHYSBASE must match the starting address in the .lds file
#define PHYSBASE 0x40000000

#define UART_TLB_ENTRY 0
#define DEVTREE_TLB_ENTRY 1
#define SPINTABLE_TLB_ENTRY 2
#define MPIC_TLB_ENTRY 3
#define IPI_TLB_ENTRY 4
#define TIMER_TLB_ENTRY 5
#define BASE_TLB_ENTRY 15
#define KSTACK_SIZE 8192

#define PHYSMAPSIZE TLB_TSIZE_4M

#ifndef _ASM
#include <stdint.h>

typedef int client_cpu_t;

#include <libos/trapframe.h>

extern unsigned long CCSRBAR_VA;

#endif

#endif
