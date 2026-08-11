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

#include "common/str.h"
#include "m4/core/cstring.h"

namespace M4 {

int32 cstrlen(const char *s) {
	if (!s)
		return 0;

	int32 size = -1;
	const char *str = s;
	do {
		++size;
	} while (*str++);
	return size;
}

void cstrcpy(char *dest, const char *src) {
	if (!src || !dest)
		return;

	do {
		*dest++ = *src;
	} while (*src++);
}

void cstrncpy(char *dest, const char *src, const int16 max_len) {
	if (!src || !dest)
		return;

	Common::strlcpy(dest, src, max_len);
}

char *cstrupr(char *src) {
	if (!src)
		return nullptr;

	char *mark = src;
	do {
		if (*mark >= 'a' && *mark <= 'z')
			*mark = (char)(*mark - 'a' + 'A');
	} while (*mark++);

	return src;
}

} // namespace M4
