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
	uint32_t eccctl;
	uint32_t eccdis;
	uint32_t eccinten;
	uint32_t eccdet;
	uint32_t eccattr;
	uint64_t eccaddr;
	uint64_t eccdata;
	uint32_t poes1;
	uint64_t poeaddr;
} pamu_error_t;

typedef struct ccf_error {
	uint32_t cedr;
	uint32_t ceer;
	uint32_t cecar;
	uint64_t cecaddr;
	uint32_t cmecar;
} ccf_error_t;

typedef struct cpc_error {
	uint32_t cpcerrdet;
	uint64_t cpcerraddr;
	uint32_t cpcerrattr;
	uint32_t cpcerrctl;
	uint32_t cpcerrinten;
	uint32_t cpcerrdis;
	uint32_t cpccaptecc;
} cpc_error_t;

typedef struct ddr_error {
	uint32_t ddrerrdet;
	uint32_t ddrerrdis;
	uint32_t ddrerrinten;
	uint32_t ddrerrattr;
	uint64_t ddrerraddr;
	uint32_t ddrsbeccmgmt;
	uint32_t ddrcaptecc;
} ddr_error_t;

typedef struct hv_error {
	char domain[32];
	char error[128];
	char hdev_tree_path[256];
	char gdev_tree_path[256];

	union {
		mcheck_error_t mcheck;
		pamu_error_t pamu;
		ccf_error_t ccf;
		cpc_error_t cpc;
		ddr_error_t ddr;
	};
} hv_error_t;

#define GUEST_ERROR_EVENT_QUEUE  0
#define GLOBAL_ERROR_EVENT_QUEUE 1

#endif
