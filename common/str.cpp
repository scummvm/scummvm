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
 * $URL$
 * $Id$
 */

#include "common/str.h"
#include "common/hash-str.h"
#include "common/util.h"

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

String::String(const char *str) : _len(0), _str(_storage) {
	if (str == 0) {
		_storage[0] = 0;
		_len = 0;
	} else
		initWithCStr(str, strlen(str));
}

String::String(const char *str, uint32 len) : _len(0), _str(_storage) {
	initWithCStr(str, len);
}

String::String(const char *beginP, const char *endP) : _len(0), _str(_storage) {
	assert(endP >= beginP);
	initWithCStr(beginP, endP - beginP);
}

void String::initWithCStr(const char *str, uint32 len) {
	assert(str);

	// Init _storage member explicitly (ie. without calling its constructor)
	// for GCC 2.95.x compatibility (see also tracker item #1602879).
	_storage[0] = 0;

	_len = len;

	if (len >= _builtinCapacity) {
		// Not enough internal storage, so allocate more
		_extern._capacity = computeCapacity(len);
		_extern._refCount = 0;
		_str = (char *)malloc(_extern._capacity+1);
		assert(_str != 0);
	}

	// Copy the string into the storage area
	memmove(_str, str, len);
	_str[len] = 0;
}

String::String(const String &str)
 : _len(str._len), _str(str.isStorageIntern() ? _storage : str._str) {
	if (str.isStorageIntern()) {
		// String in internal storage: just copy it
		memcpy(_storage, str._storage, _builtinCapacity);
	} else {
		// String in external storage: use refcount mechanism
		str.incRefCount();
		_extern._refCount = str._extern._refCount;
		_extern._capacity = str._extern._capacity;
	}
	assert(_str != 0);
}

String::String(char c)
: _len(0), _str(_storage) {

	_storage[0] = c;
	_storage[1] = 0;

	// TODO/FIXME: There is no reason for the following check -- we *do*
	// allow strings to contain 0 bytes!
	_len = (c == 0) ? 0 : 1;
}

String::~String() {
	decRefCount(_extern._refCount);
}

void String::makeUnique() {
	ensureCapacity(_len, true);
}

/**
 * Ensure that enough storage is available to store at least new_len
 * characters plus a null byte. In addition, if we currently share
 * the storage with another string, unshare it, so that we can safely
 * write to the storage.
 */
void String::ensureCapacity(uint32 new_len, bool keep_old) {
	bool isShared;
	uint32 curCapacity, newCapacity;
	char *newStorage;
	int *oldRefCount = _extern._refCount;

	if (isStorageIntern()) {
		isShared = false;
		curCapacity = _builtinCapacity - 1;
	} else {
		isShared = (oldRefCount && *oldRefCount > 1);
		curCapacity = _extern._capacity;
	}

	// Special case: If there is enough space, and we do not share
	// the storage, then there is nothing to do.
	if (!isShared && new_len <= curCapacity)
		return;

	if (isShared && new_len <= _builtinCapacity - 1) {
		// We share the storage, but there is enough internal storage: Use that.
		newStorage = _storage;
		newCapacity = _builtinCapacity - 1;
	} else {
		// We need to allocate storage on the heap!

		// Compute a suitable new capacity limit
		newCapacity = computeCapacity(new_len);

		// Allocate new storage
		newStorage = (char *)malloc(newCapacity+1);
		assert(newStorage);
	}

	// Copy old data if needed, elsewise reset the new storage.
	if (keep_old) {
		assert(_len <= newCapacity);
		memcpy(newStorage, _str, _len + 1);
	} else {
		_len = 0;
		newStorage[0] = 0;
	}

	// Release hold on the old storage ...
	decRefCount(oldRefCount);

	// ... in favor of the new storage
	_str = newStorage;

	if (!isStorageIntern()) {
		// Set the ref count & capacity if we use an external storage.
		// It is important to do this *after* copying any old content,
		// else we would override data that has not yet been copied!
		_extern._refCount = 0;
		_extern._capacity = newCapacity;
	}
}

void String::incRefCount() const {
	assert(!isStorageIntern());
	if (_extern._refCount == 0) {
		_extern._refCount = new int(2);
	} else {
		++(*_extern._refCount);
	}
}

void String::decRefCount(int *oldRefCount) {
	if (isStorageIntern())
		return;

	if (oldRefCount) {
		--(*oldRefCount);
	}
	if (!oldRefCount || *oldRefCount <= 0) {
		// The ref count reached zero, so we free the string storage
		// and the ref count storage.
		delete oldRefCount;
		free(_str);

		// Even though _str points to a freed memory block now,
		// we do not change its value, because any code that calls
		// decRefCount will have to do this afterwards anyway.
	}
}

String& String::operator  =(const char *str) {
	uint32 len = strlen(str);
	ensureCapacity(len, false);
	_len = len;
	memmove(_str, str, len + 1);
	return *this;
}

String &String::operator  =(const String &str) {
	if (&str == this)
		return *this;

	if (str.isStorageIntern()) {
		decRefCount(_extern._refCount);
		_len = str._len;
		_str = _storage;
		memcpy(_str, str._str, _len + 1);
	} else {
		str.incRefCount();
		decRefCount(_extern._refCount);

		_extern._refCount = str._extern._refCount;
		_extern._capacity = str._extern._capacity;
		_len = str._len;
		_str = str._str;
	}

	return *this;
}

String& String::operator  =(char c) {
	decRefCount(_extern._refCount);
	_str = _storage;
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

String &String::operator +=(char c) {
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
	const uint32 x_len = strlen(x);
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

bool String::contains(const char *x) const {
	assert(x != 0);
	return strstr(c_str(), x) != NULL;
}

bool String::contains(char x) const {
	return strchr(c_str(), x) != NULL;
}

void String::deleteLastChar() {
	deleteChar(_len - 1);
}

void String::deleteChar(uint32 p) {
	assert(p < _len);

	makeUnique();
	while (p++ < _len)
		_str[p-1] = _str[p];
	_len--;
}

void String::clear() {
	decRefCount(_extern._refCount);

	_len = 0;
	_str = _storage;
	_storage[0] = 0;
}

void String::setChar(char c, uint32 p) {
	assert(p <= _len);

	makeUnique();
	_str[p] = c;
}

void String::insertChar(char c, uint32 p) {
	assert(p <= _len);

	ensureCapacity(_len + 1, true);
	_len++;
	for (uint32 i = _len; i > p; --i)
		_str[i] = _str[i-1];
	_str[p] = c;
}

void String::toLowercase() {
	makeUnique();
	for (uint32 i = 0; i < _len; ++i)
		_str[i] = tolower(_str[i]);
}

void String::toUppercase() {
	makeUnique();
	for (uint32 i = 0; i < _len; ++i)
		_str[i] = toupper(_str[i]);
}

void String::trim() {
	if (_len == 0)
		return;

	makeUnique();

	// Trim trailing whitespace
	while (_len >= 1 && isspace(_str[_len-1]))
		_len--;
	_str[_len] = 0;

	// Trim leading whitespace
	char *t = _str;
	while (isspace(*t))
		t++;

	if (t != _str) {
		_len -= t - _str;
		memmove(_str, t, _len + 1);
	}
}

uint String::hash() const {
	return hashit(c_str());
}

#pragma mark -

bool String::operator ==(const String &x) const {
	return equals(x);
}

bool String::operator ==(const char *x) const {
	assert(x != 0);
	return equals(x);
}

bool String::operator !=(const String &x) const {
	return !equals(x);
}

bool String::operator !=(const char *x) const {
	assert(x != 0);
	return !equals(x);
}

bool String::operator < (const String &x) const {
	return compareTo(x) < 0;
}

bool String::operator <= (const String &x) const {
	return compareTo(x) <= 0;
}

bool String::operator > (const String &x) const {
	return compareTo(x) > 0;
}

bool String::operator >= (const String &x) const {
	return compareTo(x) >= 0;
}

#pragma mark -

bool operator == (const char* y, const String &x) {
	return (x == y);
}

bool operator != (const char* y, const String &x) {
	return x != y;
}

#pragma mark -

bool String::equals(const String &x) const {
	return (0 == compareTo(x));
}

bool String::equals(const char *x) const {
	assert(x != 0);
	return (0 == compareTo(x));
}

bool String::equalsIgnoreCase(const String &x) const {
	return (0 == compareToIgnoreCase(x));
}

bool String::equalsIgnoreCase(const char *x) const {
	assert(x != 0);
	return (0 == compareToIgnoreCase(x));
}

int String::compareTo(const String &x) const {
	return compareTo(x.c_str());
}

int String::compareTo(const char *x) const {
	assert(x != 0);
	return strcmp(c_str(), x);
}

int String::compareToIgnoreCase(const String &x) const {
	return compareToIgnoreCase(x.c_str());
}

int String::compareToIgnoreCase(const char *x) const {
	assert(x != 0);
	return scumm_stricmp(c_str(), x);
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

String operator +(char x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator +(const String &x, char y) {
	String temp(x);
	temp += y;
	return temp;
}

char *ltrim(char *t) {
	while (isspace(*t))
		t++;
	return t;
}

char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isspace(t[l]))
		t[l--] = 0;
	return t;
}

char *trim(char *t) {
	return rtrim(ltrim(t));
}

}	// End of namespace Common
