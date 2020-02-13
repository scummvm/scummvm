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

#include "glk/archetype/string.h"
#include "common/algorithm.h"

namespace Glk {
namespace Archetype {

int String::indexOf(char c) const {
	const char *p = strchr(c_str(), c);
	return p ? p - c_str() : -1;
}

int String::indexOf(const String &substr) const {
	const char *c = strstr(c_str(), substr.c_str());
	return c ? c - c_str() : -1;
}

int String::indexOf(const String &substr, int start) const {
	const char *c = strstr(c_str() + start, substr.c_str());
	return c ? c - c_str() : -1;
}

int String::lastIndexOf(char c) const {
	for (int i = (int)size() - 1; i >= 0; --i) {
		if (operator[](i) == c)
			return i;
	}

	return -1;
}

void String::trim() {
	while (!empty() && (lastChar() == ' ' || lastChar() == '\t' || lastChar() == '\n'
			|| lastChar() == '\r'))
		deleteLastChar();

	while (hasPrefix(" ") || hasPrefix("\t") || hasPrefix("\n"))
		deleteChar(0);
}

String operator+(const String &x, const String &y) {
	Common::String tmp = Common::operator+(x, y);
	return String(tmp.c_str());
}

String operator+(const char *x, const String &y) {
	Common::String tmp = Common::operator+(x, y);
	return String(tmp.c_str());
}

String operator+(const String &x, const char *y) {
	Common::String tmp = Common::operator+(x, y);
	return String(tmp.c_str());
}

String operator+(const String &x, char y) {
	Common::String tmp = Common::operator+(x, y);
	return String(tmp.c_str());
}

String operator+(char x, const String &y) {
	Common::String tmp = Common::operator+(x, y);
	return String(tmp.c_str());
}

bool operator==(const char *x, const String &y) {
	return Common::operator==(x, y);
}

bool operator!=(const char *x, const String &y) {
	return Common::operator!=(x, y);
}

String String::format(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	Common::String tmp = Common::String::vformat(fmt, va);
	va_end(va);
	return String(tmp);
}

String String::vformat(const char *fmt, va_list args) {
	Common::String tmp = Common::String::vformat(fmt, args);
	return String(tmp);
}

int String::val(int *code) {
	const char *srcP = c_str();
	int result = 0, sign = 0, idx = 1;

	if (*srcP == '-') {
		sign = -1;
		++srcP;
		++idx;
	}

	for (; *srcP; ++srcP, ++idx) {
		if (*srcP < '0' || *srcP > '9') {
			// Invalid character
			if (code)
				*code = idx;
			return result;
		}

		result = (result * 10) + (*srcP - '0');
	}

	if (code)
		*code = 0;

	return sign * result;
}

String String::left(size_t count) const {
	return String(c_str(), c_str() + MIN(count, (size_t)size()));
}

String String::right(size_t count) const {
	size_t len = size();
	return String(c_str() + len - MIN(count, len), c_str() + len);
}

String String::mid(size_t start, size_t count) const {
	int max = (int)size() - start;
	return String(c_str() + start, c_str() + start + MIN((int)count, max));
}

String String::mid(size_t start) const {
	return String(c_str() + start);
}

void String::del(size_t start, size_t count) {
	if (start)
		(*this) = left(start) + mid(start + count - 1);
	else
		(*this) = mid(count);
}

} // End of namespace Archetype
} // End of namespace Glk
