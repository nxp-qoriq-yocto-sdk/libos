
/*
 * Copyright (C) 2008 Freescale Semiconductor, Inc.
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

#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdarg.h>

void *memcpy(void *dest, const void *src, size_t len);
void *memmove(void *dest, const void *src, size_t len);
int memcmp(const void *b1, const void *b2, size_t len);
void *memset(void *block, int c, size_t len);
void *memchr(const void *s, int c, size_t len);
void *memrchr(const void *s, int c, size_t len);
size_t strnlen(const char *s, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t len);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t len);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);
char *strchr(const char *s, int c);
size_t sprintf(char *buf, const char *str, ...);
size_t snprintf(char *buf, size_t size, const char *str, ...);
size_t vsnprintf(char *buf, size_t size, const char *str, va_list args);
unsigned long long strtoull(const char *restrict str, char **restrict endptr,
                            int base);
long long strtoll(const char *restrict str, char **restrict endptr, int base);

#endif
