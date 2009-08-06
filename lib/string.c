/** @file
 * string and memory functions.
 */

/*
 * Copyright (C) 2007-2009 Freescale Semiconductor, Inc.
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

/* 
 *  This software is copyright (c) 2007 Scott Wood <scott@buserror.net>.
 *  
 *  This software is provided 'as-is', without any express or implied warranty.
 *  In no event will the authors or contributors be held liable for any damages
 *  arising from the use of this software.
 *  
 *  Permission is hereby granted to everyone, free of charge, to use, copy,
 *  modify, prepare derivative works of, publish, distribute, perform,
 *  sublicense, and/or sell copies of the Software, provided that the above
 *  copyright notice and disclaimer of warranty be included in all copies or
 *  substantial portions of this software.
 */

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <malloc.h>

#include <libos/errors.h>
#include <libos/percpu.h>

void *memcpy(void *dest, const void *src, size_t len)
{
	const char *cs = src;
	char *cd = dest;
	const long *ls;
	long *ld;
	size_t i;
	const unsigned int lowbits = sizeof(long) - 1;

	if (__builtin_expect((((uintptr_t)dest) & lowbits) !=
	                     (((uintptr_t)src) & lowbits), 0)) {
		for (i = 0; i < len; i++)	
			cd[i] = cs[i];

		return dest;
	}

	while (len > 0 && (((uintptr_t)cd) & lowbits)) {
		*cd++ = *cs++;
		len--;
	}

	ls = (const long *)cs;
	ld = (long *)cd;

	while (len > lowbits) {
		*ld++ = *ls++;
		len -= sizeof(long);
	}
	
	cs = (const char *)ls;
	cd = (char *)ld;

	while (len > 0) {
		*cd++ = *cs++;
		len--;
	}
	
	return dest;
}

void *memmove(void *dest, const void *src, size_t len)
{
	const char *cs = src;
	char *cd = dest;
	size_t i;

	if (dest < src)
		return memcpy(dest, src, len);

	for (i = len - 1; i < len; i--)
		cd[i] = cs[i];

	return dest;
}

int memcmp(const void *b1, const void *b2, size_t len)
{
	size_t pos;
	const char *c1 = b1;
	const char *c2 = b2;
	
	for (pos = 0; pos < len; pos++) {
		if (c1[pos] != c2[pos])
			return (int)c1[pos] - (int)c2[pos];
	}
	
	return 0;
}

size_t strnlen(const char *s, size_t n)
{
	size_t pos = 0;

	while (pos < n && *s++)
		pos++;

	return pos;
}

size_t strlen(const char *s)
{
	size_t pos = 0;

	while (*s++)
		pos++;

	return pos;
}

char *strcpy(char *dest, const char *src)
{
	char *orig = dest;

	do {
		*dest = *src++;
	} while (*dest++);

	return orig;
}

char *strncpy(char *dest, const char *src, size_t len)
{
	size_t pos = 0;

	while (pos < len) {
		dest[pos] = src[pos];
		
		if (!dest[pos++])
			break;
	}
	
	memset(&dest[pos], 0, len - pos);
	return dest;
}

char *strcat(char *dest, const char *src)
{
	char *orig = dest;
	dest += strlen(dest);

	do {
		*dest = *src++;
	} while (*dest++);

	return orig;
}

char *strncat(char *dest, const char *src, size_t len)
{
	size_t pos = strlen(dest);
	
	while (pos < len) {
		dest[pos] = src[pos];
		
		if (!dest[pos++])
			break;
	}
	
	if (pos < len)
		memset(&dest[pos], 0, len - pos);

	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}

	return *s2 - *s1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	int i = 0;

	while (i < n && s1[i] && s2[i] && s1[i] == s2[i])
		i++;

	if (i == n)
		return 0;

	return s2[i] - s1[i];
}

char *strchr(const char *s, int c)
{
	while (*s && *s != c)
		s++;

	if (*s == c) 
		return (char *)s;

	return NULL;
}

char *strstr(const char *s1, const char *s2)
{
	size_t pos = 0, len = strlen(s1), len2 = strlen(s2);

	if (len2 == 0)
		return (char *)s1;

	while (1) {
		while (pos < len && s1[pos] != s2[0])
			pos++;

		if (pos >= len || len - pos < len2)
			return NULL;

		if (!memcmp(&s1[pos], s2, len2))
			return (char *)&s1[pos];

		pos++;
	}
}

char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *ret = malloc(len);

	if (ret)
		memcpy(ret, s, len);

	return ret;
}

void *memchr(const void *s, int c, size_t len)
{
	const char *cp = s;

	while (*cp && *cp != c && len-- != 0)
		cp++;

	if (*cp == c) 
		return (void *)cp;

	return NULL;
}

void *memrchr(const void *s, int c, size_t len)
{
	const char *cp = s;
	len--;

	while (cp[len] && cp[len] != c && (ssize_t)len >= 0)
		len--;

	if (cp[len] == c) 
		return (void *)&cp[len];

	return NULL;
}

/* NOTE: These functions do not strip initial whitespace. */
unsigned long long strtoull(const char *restrict str, char **restrict endptr,
                            int base)
{
	unsigned long long ret = 0;
	int maxnumdigit = base >= 10 ? '9' : base - 1;
	
	cpu->errno = ERR_INVALID;

	while (*str) {
		unsigned long long prev = ret;
		char c = *str;
		
		ret *= base;
	
		if (c >= '0' && c <= maxnumdigit)
			ret += c - '0';
		else if (c >= 'a' && c < 'a' + base - 10)
			ret += c - 'a' + 10;
		else if (c >= 'A' && c < 'A' + base - 10)
			ret += c - 'A' + 10;
		else
			break;

		if (ret < prev) {
			ret = 0;
			cpu->errno = ERR_RANGE;
			break;
		}
		
 		cpu->errno = 0;
		str++;
	}

	if (endptr)
		*endptr = (char *)str;

	return ret;
}

long long strtoll(const char *restrict str, char **restrict endptr, int base)
{
	long long ret;
	int neg = 0;
	
	if (*str == '-') {
		neg = 1;
		str++;
	}

	ret = strtoull(str, endptr, base);
	if (cpu->errno)
		return 0;
	
	if (neg) {
		if ((unsigned long long)ret > ((unsigned long long)LONG_LONG_MAX) + 1)
			cpu->errno = ERR_RANGE;

		ret = -ret;
	} else {
		if ((unsigned long long)ret > LONG_LONG_MAX)
			cpu->errno = ERR_RANGE;
	}
	
	return ret;
}
