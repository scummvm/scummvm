/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/str.h"

#include <ctype.h>

namespace Common {

String::String(const char *str, int len) {
	_refCount = new int(1);
	if (str && len != 0) {
		if (len > 0)
			_capacity = _len = len;
		else
			_capacity = _len = strlen(str);
		_str = (char *)calloc(1, _capacity+1);
		memcpy(_str, str, _len);
		_str[_len] = 0;
	} else {
		_capacity = _len = 0;
		_str = 0;
	}
}

String::String(const ConstString &str) {
	printf("String::String(const ConstString &str)\n");
	_refCount = new int(1);
	if (str._str) {		
		_capacity = _len = strlen(str._str);
		_str = (char *)calloc(1, _capacity+1);
		memcpy(_str, str._str, _len+1);
	} else {
		_capacity = _len = 0;
		_str = 0;
	}
}

String::String(const String &str) : ConstString() {
	++(*str._refCount);

	_refCount = str._refCount;
	_capacity = str._capacity;
	_len = str._len;
	_str = str._str;
}

String::~String() {
	decRefCount();
}

void String::decRefCount() {
	--(*_refCount);
	if (*_refCount <= 0) {
		delete _refCount;
		if (_str)
			free(_str);
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

		_refCount = new int(1);
		_capacity = 0;
		_len = 0;
		_str = 0;
	}
	return *this;
}

String &String::operator  =(const String &str) {
	++(*str._refCount);

	decRefCount();

	_refCount = str._refCount;
	_capacity = str._capacity;
	_len = str._len;
	_str = str._str;

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

		_refCount = new int(1);
		_capacity = 0;
		_len = 0;
		_str = 0;
	}
}

void String::insertChar(char c, int p) {
	if (p >= 0 && p <= _len) {
		ensureCapacity(++_len, true);
		for (int i = _len; i > p; i--) {
			_str[i] = _str[i-1];
		}
		_str[p] = c;
	}
}

void String::toLowercase() {
	if (_str == 0 || _len == 0)
		return;

	for (int i = 0; i < _len; ++i)
		_str[i] = tolower(_str[i]);
}

void String::toUppercase() {
	if (_str == 0 || _len == 0)
		return;

	for (int i = 0; i < _len; ++i)
		_str[i] = toupper(_str[i]);
}

void String::ensureCapacity(int new_len, bool keep_old) {
	// If there is not enough space, or if we are not the only owner 
	// of the current data, then we have to reallocate it.
	if (new_len <= _capacity && *_refCount == 1)
		return;

	int		newCapacity = (new_len <= _capacity) ? _capacity : new_len + 32;
	char	*newStr = (char *)calloc(1, newCapacity+1);

	if (keep_old && _str)
		memcpy(newStr, _str, _len + 1);
	else
		_len = 0;

	decRefCount();

	_refCount = new int(1);
	_capacity = newCapacity;
	_str = newStr;
}

#pragma mark -

bool ConstString::operator ==(const ConstString &x) const {
	return (_len == x._len) && ((_len == 0) || (0 == strcmp(_str, x._str)));
}

bool ConstString::operator ==(const char *x) const {
	if (_str == 0)
		return (x == 0) || (*x == 0);
	if (x == 0)
		return (_len == 0);
	return (0 == strcmp(_str, x));
}

bool ConstString::operator !=(const ConstString &x) const {
	return (_len != x._len) || ((_len != 0) && (0 != strcmp(_str, x._str)));
}

bool ConstString::operator !=(const char *x) const {
	if (_str == 0)
		return (x != 0) && (*x != 0);
	if (x == 0)
		return (_len != 0);
	return (0 != strcmp(_str, x));
}

bool ConstString::operator < (const ConstString &x) const {
	if (!_len || !x._len)	// Any or both empty?
		return !_len && x._len;	// Less only if this string is empty and the other isn't
	return scumm_stricmp(_str, x._str) < 0;
}

bool ConstString::operator <= (const ConstString &x) const {
	if (!_len || !x._len)	// Any or both empty?
		return !_len;	// Less or equal unless the other string is empty and this one isn't
	return scumm_stricmp(_str, x._str) <= 0;
}

bool ConstString::operator > (const ConstString &x) const {
	return (x < *this);
}

bool ConstString::operator >= (const ConstString &x) const {
	return (x <= *this);
}

bool operator == (const char* y, const ConstString &x) {
	return (x == y);
}

bool operator != (const char* y, const ConstString &x) {
	return x != y;
}

}	// End of namespace Common
