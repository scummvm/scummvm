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

#ifndef PORT_DEFS_H
#define PORT_DEFS_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "system.h"

#define M_PI 3.14159265358979323846

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

C_LINKAGE_BEGIN

void voidFunc(void*, const char*, ...);

#undef fprintf
#undef vfprintf
#undef FILE
#undef stderr
#undef stdout
#undef fputs
#undef fflush

#define fprintf voidFunc
#define vfprintf voidFunc
#define FILE void
#define stderr (void*)0
#define stdout (void*)1
#define fputs(str, file)
#define fflush(file)

int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int stricmp(const char*, const char*);
int vsnprintf(char* buf, size_t count, const char* format, va_list arg);
int snprintf(char* str, size_t size, const char* format, ...);
int sscanf(const char* buffer, const char* format, ...);
char* strdup(const char* s1);
int vsprintf(char* str, const char* format, va_list arg);
int strnicmp(const char* s1, const char* s2, size_t len);

C_LINKAGE_END

#endif
