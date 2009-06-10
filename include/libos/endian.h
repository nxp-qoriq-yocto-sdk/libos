/*
 * Copyright (C) 2007,2008 Freescale Semiconductor, Inc.
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

#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

static inline uint16_t swap16(uint16_t val)
{
	return (val & 0x00ff) << 8 | ((val >> 8) &  0x00FF);
}

static inline uint32_t swap32(uint32_t val)
{
	return ((val & 0x000000ff) << 24) |
	       ((val & 0x0000ff00) <<  8) |
	       ((val & 0x00ff0000) >>  8) |
	       ((val & 0xff000000) >> 24);
}

static inline uint64_t swap64(uint64_t val)
{
	return (uint64_t)swap32(val) | ((uint64_t)swap32(val >> 32) << 32);
}

#ifdef _BIG_ENDIAN
#define cpu_to_le16 swap16
#define cpu_to_le32 swap32
#define cpu_to_le64 swap64
#define cpu_to_be16(x) (x)
#define cpu_to_be32(x) (x)
#define cpu_to_be64(x) (x)
#define cpu_from_le16 swap16
#define cpu_from_le32 swap32
#define cpu_from_le64 swap64
#define cpu_from_be16(x) (x)
#define cpu_from_be32(x) (x)
#define cpu_from_be64(x) (x)
#elif defined(_LITTLE_ENDIAN)
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#define cpu_to_be16 swap16
#define cpu_to_be32 swap32
#define cpu_to_be64 swap64
#define cpu_from_le16(x) (x)
#define cpu_from_le32(x) (x)
#define cpu_from_le64(x) (x)
#define cpu_from_be16 swap16
#define cpu_from_be32 swap32
#define cpu_from_be64 swap64
#else
#error Please specify endianness.
#endif

#endif
