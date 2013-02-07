/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Freescale Semiconductor nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *
 * This software is provided by Freescale Semiconductor "as is" and any
 * express or implied warranties, including, but not limited to, the implied
 * warranties of merchantability and fitness for a particular purpose are
 * disclaimed. In no event shall Freescale Semiconductor be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential damages
 * (including, but not limited to, procurement of substitute goods or services;
 * loss of use, data, or profits; or business interruption) however caused and
 * on any theory of liability, whether in contract, strict liability, or tort
 * (including negligence or otherwise) arising in any way out of the use of
 * this software, even if advised of the possibility of such damage.
 */

#include <libos/cache.h>
#include <libos/libos.h>
#include <libos/bitops.h>

/** function to synchronize caches when modifying instructions
 * This follows the recommended sequence in the EREF for
 * self modifying code.
 */
int icache_range_sync(void *ptr, size_t len)
{
	uintptr_t start, end, addr;

	start = (uintptr_t)ptr & ~(uintptr_t)(cache_block_size - 1);
	end = ((uintptr_t)ptr + len - 1) & ~(uintptr_t)(cache_block_size - 1);

	for (addr = start; addr >= start && addr <= end;
	     addr += cache_block_size)
		icache_block_sync((char *)addr);

	return 0;
}

int dcache_range_flush(void *ptr, size_t len)
{
	uintptr_t start, end, addr;

	start = (uintptr_t)ptr & ~(uintptr_t)(cache_block_size - 1);
	end = ((uintptr_t)ptr + len - 1) & ~(uintptr_t)(cache_block_size - 1);

	for (addr = start; addr >= start && addr <= end;
	     addr += cache_block_size)
		dcache_block_flush((char *)addr);

	return 0;
}
