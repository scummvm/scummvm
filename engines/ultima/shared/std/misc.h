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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_STD_MISC_H
#define ULTIMA8_STD_MISC_H

#include "common/scummsys.h"

namespace Ultima {
namespace Std {

enum Precision { hex = 16, dec = 10 };

extern void memset(void *data, byte val, size_t count);
extern int memcmp(const void *ptr1, const void *ptr2, size_t num);
extern void *memcpy(void *destination, const void *source, size_t num);
extern void strncpy(char *buffer, const char *src, size_t bufSize);
extern size_t strlen(const char *str);
extern int toUpper(int ch);
extern int toLower(int ch);
extern int strcmp(const char *leftStr, const char *rightStr);
extern int strncmp(const char *str1, const char *str2, size_t n);
extern long int strtol(const char *str, char **endptr, int base);
extern int atoi(const char *str);
extern double atof(const char *str);
extern const char *strstr(const char *str, const char *substr);
extern double pow(double x, double y);
extern double sqrt(double val);
extern int labs(int v);

extern void *malloc(size_t size);
extern void *calloc(size_t num, size_t size);
extern void free(void *&p);

} // End of namespace Std
} // End of namespace Ultima

#endif
