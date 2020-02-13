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

#include "ultima/shared/std/misc.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Std {

void memset(void *data, byte val, size_t count) {
	::memset(data, val, count);
}

int memcmp(const void *ptr1, const void *ptr2, size_t num) {
	return ::memcmp(ptr1, ptr2, num);
}

void *memcpy(void *destination, const void *source, size_t num) {
	return ::memcpy(destination, source, num);
}

void strncpy(char *buffer, const char *src, size_t bufSize) {
	::strncpy(buffer, src, bufSize);
}

size_t strlen(const char *str) {
	return ::strlen(str);
}

int toUpper(int ch) {
	return toupper(ch);
}

int toLower(int ch) {
	return tolower(ch);
}

int strcmp(const char *leftStr, const char *rightStr) {
	return ::strcmp(leftStr, rightStr);
}

int strncmp(const char *str1, const char *str2, size_t n) {
	return ::strncmp(str1, str2, n);
}

long int strtol(const char *str, char **endptr, int base) {
	return ::strtol(str, endptr, base);
}

int atoi(const char *str) {
	return ::atoi(str);
}

double atof(const char *str) {
	return ::atof(str);
}

double sqrt(double val) {
	return ::sqrt(val);
}

int labs(int v) {
	return ABS(v);
}

const char *strstr(const char *str, const char *substr) {
	return ::strstr(str, substr);
}

double pow(double x, double y) {
	return ::pow(x, y);
}

void *malloc(size_t size) {
	return ::malloc(size);
}

void *calloc(size_t num, size_t size) {
	return ::calloc(num, size);
}

void free(void *&p) {
	::free(p);
	p = 0;
}

} // End of namespace Std
} // End of namespace Ultima
