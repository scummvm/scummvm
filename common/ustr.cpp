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

U32String &U32String::operator=(U32String &&str) {
	assign(static_cast<U32String &&>(str));
	return *this;
}

U32String &U32String::operator=(const String &str) {
	clear();
	decodeInternal(str.c_str(), str.size(), Common::kUtf8);
	return *this;
}

U32String &U32String::operator=(const value_type *str) {
	assign(str);
	return *this;
}

U32String &U32String::operator=(const char *str) {
	clear();
	decodeInternal(str, strlen(str), Common::kUtf8);
	return *this;
}

U32String &U32String::operator=(value_type c) {
	assign(c);
	return *this;
}

U32String &U32String::operator+=(const value_type *str) {
	assignAppend(str);
	return *this;
}

U32String &U32String::operator+=(const U32String &str) {
	assignAppend(str);
	return *this;
}

U32String &U32String::operator+=(value_type c) {
	assignAppend(c);
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

U32String U32String::formatInternal(const U32String *fmt, ...) {
	U32String output;

	va_list va;
	va_start(va, fmt);
	U32String::vformat(fmt->c_str(), fmt->c_str() + fmt->size(), output, va);
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
	uint uint_temp;
	char *string_temp;

	value_type ch;
	value_type *u32string_temp;

	value_type buffer[512];

	const value_type *start = fmt;

	while (fmt != fmtEnd) {
		if (*fmt == '%') {
			// Copy all characters since the last argument
			if (fmt != start)
				output.append(start, fmt);

			switch (ch = *++fmt) {
			case 'S':
				u32string_temp = va_arg(args, value_type *);

				output += u32string_temp;
				break;
			case 's':
				string_temp = va_arg(args, char *);

				output += Common::U32String(string_temp, kUtf8);
				break;
			case 'i':
			// fallthrough intended
			case 'd':
				int_temp = va_arg(args, int);
				ustr_helper_itoa(int_temp, buffer, 10);

				output += buffer;
				break;
			case 'u':
				uint_temp = va_arg(args, uint);
				ustr_helper_uitoa(uint_temp, buffer, 10);

				output += buffer;
				break;
			case 'c':
				//char is promoted to int when passed through '...'
				int_temp = va_arg(args, int);

				output += int_temp;
				break;
			case '%':
				output += '%';
				break;
			default:
				warning("Unexpected formatting type for U32String::Format.");
				break;
			}

			start = ++fmt;
		} else {
			// We attempt to copy as many characters as possible in one go.
			++fmt;
		}
	}

	// Append any remaining characters
	if (fmt != start)
		output.append(start, fmt);

	return output.size();
}

U32String::value_type* U32String::ustr_helper_itoa(int num, value_type* str, uint base) {
	if (num < 0) {
		str[0] = '-';
		ustr_helper_uitoa(-num, str + 1, base);
	} else {
		ustr_helper_uitoa(num, str, base);
	}

	return str;
}

U32String::value_type* U32String::ustr_helper_uitoa(uint num, value_type* str, uint base) {
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
		value_type temp = str[k];
		str[k] = str[j];
		str[j] = temp;
		k++;
		j--;
	}

	return str;
}

U32String toPrintable(const U32String &in, bool keepNewLines) {
	U32String res;

	const char *tr = "\x01\x01\x02\x03\x04\x05\x06" "a"
				  //"\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
					   "b" "t" "n" "v" "f" "r\x0e\x0f"
					"\x10\x11\x12\x13\x14\x15\x16\x17"
					"\x18\x19\x1a" "e\x1c\x1d\x1e\x1f";

	for (const u32char_type_t *p = in.c_str(); *p; p++) {
		if (*p == '\n') {
			if (keepNewLines)
				res += *p;
			else
				res += U32String("\\n");

			continue;
		}

		if (*p < 0x20 || *p == '\'' || *p == '\"' || *p == '\\') {
			res += '\\';

			if (*p < 0x20) {
				if (tr[*p] < 0x20)
					res += Common::String::format("x%02x", *p);
				else
					res += tr[*p];
			} else {
				res += *p;	// We will escape it
			}
		} else
			res += *p;
	}

	return res;
}

} // End of namespace Common
