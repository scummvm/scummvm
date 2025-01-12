/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _PORTDEFS_H_
#define _PORTDEFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <new>
#include <limits>

// This is defined in snprintf.c
#ifdef __cplusplus
extern "C" {
#endif
int rpl_vsnprintf(char *text, size_t maxlen, const char *fmt, va_list ap);
#ifdef __cplusplus
}
#endif

#define vsnprintf rpl_vsnprintf

// Bionic libc up to Android 6/API 23 (excluded) is non-conformant with ANSI C.
// It compares with integer character without casting it to unsigned char as required.
// On AArch64, another (optimized) implementation is used which behaves properly.
// If char is unsigned the problem does not exist.
// strchr calls are also replaced by memchr calls when the compiler knows the haystack size.
#if !defined(__CHAR_UNSIGNED__) && !defined(__aarch64__)
#define memchr(s,c,n) memchr(s, (unsigned char)(c), n)
#define strchr(s,c) strchr(s, (unsigned char)(c))
#endif

#endif // _PORTDEFS_H_
