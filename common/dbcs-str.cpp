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

#include "common/dbcs-str.h"
#include "common/str.h"
#include "common/memorypool.h"
#include "common/util.h"

namespace Common {

void DBCSString::decodeDBCS(const char *str, uint32 len) {
	for (uint i = 0; i < len; ) {
		if ((str[i] & 0x80) && i + 1 < len) {
			operator+=((str[i] << 8) | (str[i+1] & 0xff));
			i += 2;
		} else if (str[i] & 0x80) {
			operator+=(str[i] << 8);
			i++;
		} else {
			operator+=(str[i]);
			i++;
		}
	}
}

DBCSString::DBCSString(const char *str) : BaseString<uint16>() {
	if (str == nullptr) {
		_storage[0] = 0;
		_size = 0;
	} else {
		decodeDBCS(str, strlen(str));
	}
}

DBCSString::DBCSString(const char *str, uint32 len) : BaseString<uint16>() {
	decodeDBCS(str, len);
}

DBCSString::DBCSString(const char *beginP, const char *endP) : BaseString<uint16>() {
	assert(endP >= beginP);
	decodeDBCS(beginP, endP - beginP);
}

DBCSString::DBCSString(const String &str) : BaseString<uint16>() {
	decodeDBCS(str.c_str(), str.size());
}

DBCSString::DBCSString(uint16 c) : BaseString<uint16>() {
	_storage[0] = c;
	_storage[1] = 0;

	_size = (c == 0) ? 0 : 1;
}

DBCSString &DBCSString::operator=(const DBCSString &str) {
	assign(str);
	return *this;
}

DBCSString &DBCSString::operator=(DBCSString &&str) {
	assign(static_cast<DBCSString &&>(str));
	return *this;
}

DBCSString &DBCSString::operator=(const String &str) {
	clear();
	decodeDBCS(str.c_str(), str.size());
	return *this;
}

DBCSString &DBCSString::operator=(const value_type *str) {
	return DBCSString::operator=(DBCSString(str));
}

DBCSString &DBCSString::operator=(const char *str) {
	clear();
	decodeDBCS(str, strlen(str));
	return *this;
}

DBCSString &DBCSString::operator+=(const DBCSString &str) {
	if (&str == this) {
		return operator+=(DBCSString(str));
	}

	int len = str._size;
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str._str, (len + 1) * sizeof(value_type));
		_size += len;
	}
	return *this;
}

DBCSString &DBCSString::operator+=(value_type c) {
	ensureCapacity(_size + 1, true);

	_str[_size++] = c;
	_str[_size] = 0;

	return *this;
}

bool DBCSString::operator==(const String &x) const {
	return equalsC(x.c_str());
}

bool DBCSString::operator==(const char *x) const {
	return equalsC(x);
}

bool DBCSString::operator!=(const String &x) const {
	return !equalsC(x.c_str());
}

bool DBCSString::operator!=(const char *x) const {
	return !equalsC(x);
}

DBCSString operator+(const DBCSString &x, const DBCSString &y) {
	DBCSString temp(x);
	temp += y;
	return temp;
}

DBCSString operator+(const DBCSString &x, const DBCSString::value_type y) {
	DBCSString temp(x);
	temp += y;
	return temp;
}

DBCSString DBCSString::substr(size_t pos, size_t len) const {
	if (pos >= _size)
		return DBCSString();
	else if (len == npos)
		return DBCSString(_str + pos);
	else
		return DBCSString(_str + pos, MIN((size_t)_size - pos, len));
}

void DBCSString::insertString(const char *s, uint32 p) {
	insertString(DBCSString(s), p);
}

void DBCSString::insertString(const String &s, uint32 p) {
	insertString(DBCSString(s), p);
}

void DBCSString::replace(uint32 pos, uint32 count, const DBCSString &str) {
	replace(pos, count, str, 0, str._size);
}

void DBCSString::replace(iterator begin_, iterator end_, const DBCSString &str) {
	replace(begin_ - _str, end_ - begin_, str._str, 0, str._size);
}

void DBCSString::replace(uint32 posOri, uint32 countOri, const DBCSString &str,
					 uint32 posDest, uint32 countDest) {
	replace(posOri, countOri, str._str, posDest, countDest);
}

void DBCSString::replace(uint32 posOri, uint32 countOri, const uint16 *str,
					 uint32 posDest, uint32 countDest) {

	// Prepare string for the replaced text.
	if (countOri < countDest) {
		uint32 offset = countDest - countOri; ///< Offset to copy the characters
		uint32 newSize = _size + offset;

		ensureCapacity(newSize, true);

		_size = newSize;

		// Push the old characters to the end of the string
		for (uint32 i = _size; i >= posOri + countDest; i--)
			_str[i] = _str[i - offset];

	} else if (countOri > countDest) {
		uint32 offset = countOri - countDest; ///< Number of positions that we have to pull back

		makeUnique();

		// Pull the remainder string back
		for (uint32 i = posOri + countDest; i + offset <= _size; i++)
			_str[i] = _str[i + offset];

		_size -= offset;
	} else {
		makeUnique();
	}

	// Copy the replaced part of the string
	for (uint32 i = 0; i < countDest; i++)
		_str[posOri + i] = str[posDest + i];

}

String DBCSString::convertToString() const {
	Common::String r;
	for (uint i = 0; i < size(); i++) {
		if (_str[i] >= 0x100) {
			r += _str[i] >> 8;
			r += _str[i] & 0xff;
		} else
			r += _str[i];
			
	}
	return r;
}

} // End of namespace Common
