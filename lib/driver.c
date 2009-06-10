/** @file
 * Driver model infrastructure
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

#include <libos/libos.h>
#include <libos/errors.h>
#include <string.h>

extern driver_t driver_begin, driver_end;

int libos_bind_driver(device_t *dev, const char *compat_strlist, size_t compat_len)
{
	const char *end_compat = compat_strlist + compat_len;

	/* Search for matches from the most-specific name to the least-specific name */
	while (compat_strlist < end_compat) {
		for (driver_t *drv = &driver_begin; drv < &driver_end; drv++) {
			int ret;

			if (strcmp(compat_strlist, drv->compatible))
				continue;

			ret = drv->probe(drv, dev);
			if (ret == ERR_UNHANDLED)
				continue;

			return ret;
		}

		compat_strlist = memchr(compat_strlist, 0, end_compat - compat_strlist);

		if (!compat_strlist)
			break;

		compat_strlist++;
	}

	return ERR_UNHANDLED;
}

const char *strlist_iterate(const char *strlist, size_t len,
                            size_t *pos)
{
	const char *next, *ret;

	if (*pos >= len)
		return NULL;

	next = memchr(strlist + *pos, 0, len - *pos);
	if (!next)
		return NULL;

	ret = strlist + *pos;
	*pos = next - strlist + 1;

	return ret;
}
