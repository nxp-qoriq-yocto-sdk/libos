/*
 * Copyright (C) 2009, 2010 Freescale Semiconductor, Inc.
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

#ifndef PLATFORM_ERROR_H
#define PLATFORM_ERROR_H

typedef struct mcheck_error {
	uint32_t mcsr;
	uint64_t mcar;
	uint64_t mcsrr0;
	uint32_t mcsrr1;
} mcheck_error_t;

typedef struct pamu_error{
	uint32_t lpid;
	uint64_t access_violation_addr;
	uint32_t avs1;
	uint32_t avs2;
	uint32_t liodn_handle;
} pamu_error_t;

typedef struct hv_error {
	char domain[32];
	char error[128];
	char hdev_tree_path[256];
	char gdev_tree_path[256];

	union {
		mcheck_error_t mcheck;
		pamu_error_t pamu;

	};
} hv_error_t;

#define GUEST_ERROR_EVENT_QUEUE  0
#define GLOBAL_ERROR_EVENT_QUEUE 1

#endif
