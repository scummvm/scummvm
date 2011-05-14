/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#define M_PI		3.14159265358979323846

// does nothing
int printf(const char* format, ...);

// delegates to snprintf
// http://sourceware.org/newlib/libc.html#sprintf
int sprintf(char* str, const char* format, ...);

int	stricmp(const char*, const char*);

int vsnprintf(char* buf,
              size_t count,
              const char* format,
              va_list arg);

// http://www.ijs.si/software/snprintf/
// engines/scumm/detection.cpp
int snprintf(char *str, size_t size, const char *format, ...);

// engines/scumm/scumm.cpp
int sscanf(const char *buffer, const char *format, ...);
