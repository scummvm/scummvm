/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"

#include "common/str.h"
#include "common/util.h"

#include <ctype.h>

namespace Common {

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
const String String::emptyString;
#else
const char *String::emptyString = "";
#endif

static int computeCapacity(int len) {
	// By default, for the capacity we use the nearest multiple of 32
	// that leaves at least 16 chars of extra space (in case the string
	// grows a bit).
	// Finally, we subtract 1 to compensate for the trailing zero byte.
	len += 16;
	return ((len + 32 - 1) & ~0x1F) - 1;
}

String::String(const char *str, int len, int capacity)
: _str(0), _len(0), _refCount(0) {

	if (str && *str && len != 0) {
		if (len > 0)
			_len = len;
		else
			_len = strlen(str);

		_capacity = computeCapacity(_len);
		if (_capacity < capacity)
			_capacity = capacity;

		_str = (char *)malloc(_capacity+1);
		memcpy(_str, str, _len);
		_str[_len] = 0;
	} else {
		_capacity = _len = 0;
		_str = 0;
	}
}

String::String(const String &str)
 : _str(str._str), _len(str._len), _refCount(0), _capacity(str._capacity) {

	if (_str != 0) {
		// If the string we are copying is non-empty, we increment its
		// refcount.
		str.incRefCount();
		_refCount = str._refCount;
	}
}

String::~String() {
	decRefCount();
}

void String::incRefCount() const {
	if (_refCount == 0) {
		_refCount = new int(2);
	} else {
		++(*_refCount);
	}
}

void String::decRefCount() {
	if (_refCount) {
		--(*_refCount);
	}
	if (!_refCount || *_refCount <= 0) {
		delete _refCount;
		_refCount = 0;
		free(_str);
		_str = 0;
	}
}

String& String::operator  =(const char *str) {
	int len = strlen(str);
	if (len > 0) {
		ensureCapacity(len, false);

		_len = len;
		memcpy(_str, str, _len + 1);
	} else if (_len > 0) {
		decRefCount();

		_refCount = 0;
		_capacity = 0;
		_len = 0;
		_str = 0;
	}
	return *this;
}

String &String::operator  =(const String &str) {
	str.incRefCount();
	decRefCount();

	_refCount = str._refCount;
	_capacity = str._capacity;
	_len = str._len;
	_str = str._str;

	return *this;
}

String& String::operator  =(char c) {
	ensureCapacity(1, false);
	_len = 1;
	_str[0] = c;
	_str[1] = 0;
	return *this;
}

String &String::operator +=(const char *str) {
	int len = strlen(str);
	if (len > 0) {
		ensureCapacity(_len + len, true);

		memcpy(_str + _len, str, len + 1);
		_len += len;
	}
	return *this;
}

String &String::operator +=(const String &str) {
	int len = str._len;
	if (len > 0) {
		ensureCapacity(_len + len, true);

		memcpy(_str + _len, str._str, len + 1);
		_len += len;
	}
	return *this;
}

String &String::operator += (char c) {
	ensureCapacity(_len + 1, true);

	_str[_len++] = c;
	_str[_len] = 0;

	return *this;
}

bool String::hasPrefix(const char *x) const {
	assert(x != 0);
	// Compare x with the start of _str.
	const char *y = c_str();
	while (*x && *x == *y) {
		++x;
		++y;
	}
	// It's a prefix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::hasSuffix(const char *x) const {
	assert(x != 0);
	// Compare x with the end of _str.
	const int x_len = strlen(x);
	if (x_len > _len)
		return false;
	const char *y = c_str() + _len - x_len;
	while (*x && *x == *y) {
		++x;
		++y;
	}
	// It's a suffix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

void String::deleteLastChar() {
	if (_len > 0) {
		ensureCapacity(_len - 1, true);
		_str[--_len] = 0;
	}
}

void String::deleteChar(int p) {
	if (p >= 0 && p < _len) {
		ensureCapacity(_len - 1, true);
		while (p++ < _len)
			_str[p-1] = _str[p];
		_len--;
	}
}

void String::clear() {
	if (_capacity) {
		decRefCount();

		_refCount = 0;
		_capacity = 0;
		_len = 0;
		_str = 0;
	}
}

void String::insertChar(char c, int p) {
	// FIXME: This should be an 'assert', not an 'if' !
	if (p >= 0 && p <= _len) {
		ensureCapacity(_len + 1, true);
		_len++;
		for (int i = _len; i > p; i--) {
			_str[i] = _str[i-1];
		}
		_str[p] = c;
	}
}

void String::toLowercase() {
	if (_str == 0 || _len == 0)
		return;

	ensureCapacity(_len, true);
	for (int i = 0; i < _len; ++i)
		_str[i] = tolower(_str[i]);
}

void String::toUppercase() {
	if (_str == 0 || _len == 0)
		return;

	ensureCapacity(_len, true);
	for (int i = 0; i < _len; ++i)
		_str[i] = toupper(_str[i]);
}

void String::ensureCapacity(int new_len, bool keep_old) {
	// If there is not enough space, or if we are not the only owner
	// of the current data, then we have to reallocate it.
	if (new_len <= _capacity && (_refCount == 0 || *_refCount == 1))
		return;

	int newCapacity = computeCapacity(new_len);

	// FIXME: We never shrink the capacity here. Is that really a good idea?
	if (newCapacity < _capacity)
		newCapacity = _capacity;

	char *newStr = (char *)malloc(newCapacity+1);

	if (keep_old && _str) {
		memcpy(newStr, _str, _len + 1);
	} else {
		_len = 0;
		newStr[0] = 0;
	}

	decRefCount();

	_refCount = 0;
	_capacity = newCapacity;
	_str = newStr;
}

#pragma mark -

bool String::operator ==(const String &x) const {
	return (0 == strcmp(c_str(), x.c_str()));
}

bool String::operator ==(const char *x) const {
	assert(x != 0);
	return (0 == strcmp(c_str(), x));
}

bool String::operator !=(const String &x) const {
	return (0 != strcmp(c_str(), x.c_str()));
}

bool String::operator !=(const char *x) const {
	assert(x != 0);
	return (0 != strcmp(c_str(), x));
}

bool String::operator < (const String &x) const {
	return strcmp(c_str(), x.c_str()) < 0;
}

bool String::operator <= (const String &x) const {
	return strcmp(c_str(), x.c_str()) <= 0;
}

bool String::operator > (const String &x) const {
	return (x < *this);
}

bool String::operator >= (const String &x) const {
	return (x <= *this);
}

#pragma mark -

String operator +(const String &x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator +(const char *x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator +(const String &x, const char *y) {
	String temp(x);
	temp += y;
	return temp;
}

#pragma mark -

bool operator == (const char* y, const String &x) {
	return (x == y);
}

bool operator != (const char* y, const String &x) {
	return x != y;
}

}	// End of namespace Common
