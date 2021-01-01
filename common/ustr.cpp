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

#include "common/ustr.h"
#include "common/str.h"
#include "common/memorypool.h"
#include "common/util.h"

namespace Common {

U32String::U32String(const char *str, Common::CodePage page) : BaseString<u32char_type_t>() {
	if (str == nullptr) {
		_storage[0] = 0;
		_size = 0;
	} else {
		decodeInternal(str, strlen(str), page);
	}
}

U32String::U32String(const char *str, uint32 len, Common::CodePage page) : BaseString<u32char_type_t>() {
	decodeInternal(str, len, page);
}

U32String::U32String(const char *beginP, const char *endP, Common::CodePage page) : BaseString<u32char_type_t>() {
	assert(endP >= beginP);
	decodeInternal(beginP, endP - beginP, page);
}

U32String::U32String(const String &str, Common::CodePage page) : BaseString<u32char_type_t>() {
	decodeInternal(str.c_str(), str.size(), page);
}

U32String &U32String::operator=(const U32String &str) {
	assign(str);
	return *this;
}

U32String &U32String::operator=(const String &str) {
	clear();
	decodeInternal(str.c_str(), str.size(), Common::kUtf8);
	return *this;
}

U32String &U32String::operator=(const value_type *str) {
	return U32String::operator=(U32String(str));
}

U32String &U32String::operator=(const char *str) {
	clear();
	decodeInternal(str, strlen(str), Common::kUtf8);
	return *this;
}

U32String &U32String::operator+=(const U32String &str) {
	if (&str == this) {
		return operator+=(U32String(str));
	}

	int len = str._size;
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str._str, (len + 1) * sizeof(value_type));
		_size += len;
	}
	return *this;
}

U32String &U32String::operator+=(value_type c) {
	ensureCapacity(_size + 1, true);

	_str[_size++] = c;
	_str[_size] = 0;

	return *this;
}

bool U32String::operator==(const String &x) const {
	return equalsC(x.c_str());
}

bool U32String::operator==(const char *x) const {
	return equalsC(x);
}

bool U32String::operator!=(const String &x) const {
	return !equalsC(x.c_str());
}

bool U32String::operator!=(const char *x) const {
	return !equalsC(x);
}

U32String operator+(const U32String &x, const U32String &y) {
	U32String temp(x);
	temp += y;
	return temp;
}

U32String operator+(const U32String &x, const U32String::value_type y) {
	U32String temp(x);
	temp += y;
	return temp;
}

U32String U32String::substr(size_t pos, size_t len) const {
	if (pos >= _size)
		return U32String();
	else if (len == npos)
		return U32String(_str + pos);
	else
		return U32String(_str + pos, MIN((size_t)_size - pos, len));
}

void U32String::insertString(const char *s, uint32 p, CodePage page) {
	insertString(U32String(s, page), p);
}

void U32String::insertString(const String &s, uint32 p, CodePage page) {
	insertString(U32String(s, page), p);
}

U32String U32String::format(U32String fmt, ...) {
	U32String output;

	va_list va;
	va_start(va, fmt);
	U32String::vformat(fmt.c_str(), fmt.c_str() + fmt.size(), output, va);
	va_end(va);

	return output;
}

U32String U32String::format(const char *fmt, ...) {
	U32String output;

	Common::U32String fmtU32(fmt);
	va_list va;
	va_start(va, fmt);
	U32String::vformat(fmtU32.c_str(), fmtU32.c_str() + fmtU32.size(),
			   output, va);
	va_end(va);

	return output;
}

int U32String::vformat(const value_type *fmt, const value_type *fmtEnd, U32String &output, va_list args) {
	int int_temp;
	char *string_temp;

	value_type ch;
	value_type *u32string_temp;
	int length = 0;
	int len = 0;
	int pos = 0;
	int tempPos = 0;

	char buffer[512];

	while (fmt != fmtEnd) {
		ch = *fmt++;
		if (ch == '%') {
			switch (ch = *fmt++) {
			case 'S':
				u32string_temp = va_arg(args, value_type *);

				tempPos = output.size();
				output.insertString(u32string_temp, pos);
				len = output.size() - tempPos;
				length += len;

				pos += len - 1;
				break;
			case 's':
				string_temp = va_arg(args, char *);
				tempPos = output.size();
				output.insertString(string_temp, pos);
				len = output.size() - tempPos;
				length += len;
				pos += len - 1;
				break;
			case 'i':
			// fallthrough intended
			case 'd':
				int_temp = va_arg(args, int);
				itoa(int_temp, buffer, 10);
				len = strlen(buffer);
				length += len;

				output.insertString(buffer, pos);
				pos += len - 1;
				break;
			case 'u':
				int_temp = va_arg(args, uint);
				itoa(int_temp, buffer, 10);
				len = strlen(buffer);
				length += len;

				output.insertString(buffer, pos);
				pos += len - 1;
				break;
			case 'c':
				//char is promoted to int when passed through '...'
				int_temp = va_arg(args, int);
				output.insertChar(int_temp, pos);
				++length;
				break;
			default:
				warning("Unexpected formatting type for U32String::Format.");
				break;
			}
		} else {
			output += *(fmt - 1);
		}
		pos++;
	}
	return length;
}

char* U32String::itoa(int num, char* str, int base) {
	int i = 0;

	if (num) {
		// go digit by digit
		while (num != 0) {
			int rem = num % base;
			str[i++] = rem + '0';
			num /= base;
		}
	} else {
		str[i++] = '0';
	}

	// append string terminator
	str[i] = '\0';
	int k = 0;
	int j = i - 1;

	// reverse the string
	while (k < j) {
		char temp = str[k];
		str[k] = str[j];
		str[j] = temp;
		k++;
		j--;
	}

	return str;
}

} // End of namespace Common
