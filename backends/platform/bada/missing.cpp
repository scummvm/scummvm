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

#include "portdefs.h"

C_LINKAGE_BEGIN

int printf(const char* format, ...) {
  return 0;
}

int sprintf(char* str, const char* format, ...) {
  return 0;
}

int stricmp(const char*, const char*) {
  return 0;
}

int vsnprintf(char* buf,
              size_t count,
              const char* format,
              va_list arg) {
  return 0;
}

int snprintf(char* str, size_t size, const char* format, ...) {
  return 0;
}

int sscanf(const char* buffer, const char* format, ...) {
  return 0;
}

// engines/scumm/smush/smush_font.cpp
char* strdup(const char* s1) {
  return 0;
}

// engines/agi/text.cpp
int vsprintf(char* str, const char* format, va_list arg) {
  return 0;
}

// engines/agos/script_pn.cpp
int strnicmp(const char* s1, const char* s2, size_t len) {
  return 0;
}

C_LINKAGE_END
