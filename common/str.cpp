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

#include "common/memorypool.h"

#include <stdarg.h>

#if !defined(__SYMBIAN32__)
#include <new>
#endif


namespace Common {

#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
const String String::emptyString;
#else
const char *String::emptyString = "";
#endif


MemoryPool *g_refCountPool = 0;	// FIXME: This is never freed right now

static uint32 computeCapacity(uint32 len) {
	// By default, for the capacity we use the next multiple of 32
	return ((len + 32 - 1) & ~0x1F);
}

String::String(const char *str) : _size(0), _str(_storage) {
	if (str == 0) {
		_storage[0] = 0;
		_size = 0;
	} else
		initWithCStr(str, strlen(str));
}

String::String(const char *str, uint32 len) : _size(0), _str(_storage) {
	initWithCStr(str, len);
}

String::String(const char *beginP, const char *endP) : _size(0), _str(_storage) {
	assert(endP >= beginP);
	initWithCStr(beginP, endP - beginP);
}

void String::initWithCStr(const char *str, uint32 len) {
	assert(str);

	// Init _storage member explicitly (ie. without calling its constructor)
	// for GCC 2.95.x compatibility (see also tracker item #1602879).
	_storage[0] = 0;

	_size = len;

	if (len >= _builtinCapacity) {
		// Not enough internal storage, so allocate more
		_extern._capacity = computeCapacity(len+1);
		_extern._refCount = 0;
		_str = new char[_extern._capacity];
		assert(_str != 0);
	}

	// Copy the string into the storage area
	memmove(_str, str, len);
	_str[len] = 0;
}

String::String(const String &str)
 : _size(str._size) {
	if (str.isStorageIntern()) {
		// String in internal storage: just copy it
		memcpy(_storage, str._storage, _builtinCapacity);
		_str = _storage;
	} else {
		// String in external storage: use refcount mechanism
		str.incRefCount();
		_extern._refCount = str._extern._refCount;
		_extern._capacity = str._extern._capacity;
		_str = str._str;
	}
	assert(_str != 0);
}

String::String(char c)
: _size(0), _str(_storage) {

	_storage[0] = c;
	_storage[1] = 0;

	// TODO/FIXME: There is no reason for the following check -- we *do*
	// allow strings to contain 0 bytes!
	_size = (c == 0) ? 0 : 1;
}

String::~String() {
	decRefCount(_extern._refCount);
}

void String::makeUnique() {
	ensureCapacity(_size, true);
}

/**
 * Ensure that enough storage is available to store at least new_size
 * characters plus a null byte. In addition, if we currently share
 * the storage with another string, unshare it, so that we can safely
 * write to the storage.
 */
void String::ensureCapacity(uint32 new_size, bool keep_old) {
	bool isShared;
	uint32 curCapacity, newCapacity;
	char *newStorage;
	int *oldRefCount = _extern._refCount;

	if (isStorageIntern()) {
		isShared = false;
		curCapacity = _builtinCapacity;
	} else {
		isShared = (oldRefCount && *oldRefCount > 1);
		curCapacity = _extern._capacity;
	}

	// Special case: If there is enough space, and we do not share
	// the storage, then there is nothing to do.
	if (!isShared && new_size < curCapacity)
		return;

	if (isShared && new_size < _builtinCapacity) {
		// We share the storage, but there is enough internal storage: Use that.
		newStorage = _storage;
		newCapacity = _builtinCapacity;
	} else {
		// We need to allocate storage on the heap!

		// Compute a suitable new capacity limit
		newCapacity = MAX(curCapacity * 2, computeCapacity(new_size+1));

		// Allocate new storage
		newStorage = new char[newCapacity];
		assert(newStorage);
	}

	// Copy old data if needed, elsewise reset the new storage.
	if (keep_old) {
		assert(_size < newCapacity);
		memcpy(newStorage, _str, _size + 1);
	} else {
		_size = 0;
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
		if (g_refCountPool == 0) {
			g_refCountPool = new MemoryPool(sizeof(int));
			assert(g_refCountPool);
		}

		_extern._refCount = (int *)g_refCountPool->allocChunk();
		*_extern._refCount = 2;
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
		if (oldRefCount) {
			assert(g_refCountPool);
			g_refCountPool->freeChunk(oldRefCount);
		}
		delete[] _str;

		// Even though _str points to a freed memory block now,
		// we do not change its value, because any code that calls
		// decRefCount will have to do this afterwards anyway.
	}
}

String& String::operator  =(const char *str) {
	uint32 len = strlen(str);
	ensureCapacity(len, false);
	_size = len;
	memmove(_str, str, len + 1);
	return *this;
}

String &String::operator  =(const String &str) {
	if (&str == this)
		return *this;

	if (str.isStorageIntern()) {
		decRefCount(_extern._refCount);
		_size = str._size;
		_str = _storage;
		memcpy(_str, str._str, _size + 1);
	} else {
		str.incRefCount();
		decRefCount(_extern._refCount);

		_extern._refCount = str._extern._refCount;
		_extern._capacity = str._extern._capacity;
		_size = str._size;
		_str = str._str;
	}

	return *this;
}

String& String::operator  =(char c) {
	decRefCount(_extern._refCount);
	_str = _storage;
	_size = 1;
	_str[0] = c;
	_str[1] = 0;
	return *this;
}

String &String::operator +=(const char *str) {
	if (_str <= str && str <= _str + _size)
		return operator+=(Common::String(str));

	int len = strlen(str);
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str, len + 1);
		_size += len;
	}
	return *this;
}

String &String::operator +=(const String &str) {
	if (&str == this)
		return operator+=(Common::String(str));

	int len = str._size;
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str._str, len + 1);
		_size += len;
	}
	return *this;
}

String &String::operator +=(char c) {
	ensureCapacity(_size + 1, true);

	_str[_size++] = c;
	_str[_size] = 0;

	return *this;
}

bool String::hasPrefix(const String &x) const {
	return hasPrefix(x.c_str());
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

bool String::hasSuffix(const String &x) const {
	return hasSuffix(x.c_str());
}

bool String::hasSuffix(const char *x) const {
	assert(x != 0);
	// Compare x with the end of _str.
	const uint32 x_size = strlen(x);
	if (x_size > _size)
		return false;
	const char *y = c_str() + _size - x_size;
	while (*x && *x == *y) {
		++x;
		++y;
	}
	// It's a suffix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::contains(const String &x) const {
	return strstr(c_str(), x.c_str()) != NULL;
}

bool String::contains(const char *x) const {
	assert(x != 0);
	return strstr(c_str(), x) != NULL;
}

bool String::contains(char x) const {
	return strchr(c_str(), x) != NULL;
}

bool String::matchString(const char *pat, bool ignoreCase, bool pathMode) const {
	return Common::matchString(c_str(), pat, ignoreCase, pathMode);
}

bool String::matchString(const String &pat, bool ignoreCase, bool pathMode) const {
	return Common::matchString(c_str(), pat.c_str(), ignoreCase, pathMode);
}

void String::deleteLastChar() {
	if (_size > 0)
		deleteChar(_size - 1);
}

void String::deleteChar(uint32 p) {
	assert(p < _size);

	makeUnique();
	while (p++ < _size)
		_str[p-1] = _str[p];
	_size--;
}

void String::clear() {
	decRefCount(_extern._refCount);

	_size = 0;
	_str = _storage;
	_storage[0] = 0;
}

void String::setChar(char c, uint32 p) {
	assert(p <= _size);

	makeUnique();
	_str[p] = c;
}

void String::insertChar(char c, uint32 p) {
	assert(p <= _size);

	ensureCapacity(_size + 1, true);
	_size++;
	for (uint32 i = _size; i > p; --i)
		_str[i] = _str[i-1];
	_str[p] = c;
}

void String::toLowercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i)
		_str[i] = tolower(_str[i]);
}

void String::toUppercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i)
		_str[i] = toupper(_str[i]);
}

void String::trim() {
	if (_size == 0)
		return;

	makeUnique();

	// Trim trailing whitespace
	while (_size >= 1 && isspace(_str[_size-1]))
		_size--;
	_str[_size] = 0;

	// Trim leading whitespace
	char *t = _str;
	while (isspace(*t))
		t++;

	if (t != _str) {
		_size -= t - _str;
		memmove(_str, t, _size + 1);
	}
}

uint String::hash() const {
	return hashit(c_str());
}

// static
String String::printf(const char *fmt, ...) {
	String output;
	assert(output.isStorageIntern());

	va_list va;
	va_start(va, fmt);
	int len = vsnprintf(output._str, _builtinCapacity, fmt, va);
	va_end(va);

	if (len == -1) {
		// MSVC doesn't return the size the full string would take up.
		// Try increasing the size of the string until it fits.

		// We assume MSVC failed to output the correct, null-terminated string
		// if the return value is either -1 or size.
		int size = _builtinCapacity;
		do {
			size *= 2;
			output.ensureCapacity(size-1, false);
			assert(!output.isStorageIntern());
			size = output._extern._capacity;

			va_start(va, fmt);
			len = vsnprintf(output._str, size, fmt, va);
			va_end(va);
		} while (len == -1 || len >= size);
	} else if (len < (int)_builtinCapacity) {
		// vsnprintf succeeded
		output._size = len;
	} else {
		// vsnprintf didn't have enough space, so grow buffer
		output.ensureCapacity(len, false);
		va_start(va, fmt);
		int len2 = vsnprintf(output._str, len+1, fmt, va);
		va_end(va);
		assert(len == len2);
		output._size = len2;
	}

	return output;
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

Common::String lastPathComponent(const Common::String &path, const char sep) {
	const char *str = path.c_str();
	const char *last = str + path.size();

	// Skip over trailing slashes
	while (last > str && *(last-1) == sep)
		--last;

	// Path consisted of only slashes -> return empty string
	if (last == str)
		return Common::String();

	// Now scan the whole component
	const char *first = last - 1;
	while (first >= str && *first != sep)
		--first;

	if (*first == sep)
		first++;

	return Common::String(first, last);
}

Common::String normalizePath(const Common::String &path, const char sep) {
	if (path.empty())
		return path;

	const char *cur = path.c_str();
	Common::String result;

	// If there is a leading slash, preserve that:
	if (*cur == sep) {
		result += sep;
		while (*cur == sep)
			++cur;
	}

	// Scan till the end of the String
	while (*cur != 0) {
		const char *start = cur;

		// Scan till the next path separator resp. the end of the string
		while (*cur != sep && *cur != 0)
			cur++;

		const Common::String component(start, cur);

		// Skip empty components and dot components, add all others
		if (!component.empty() && component != ".") {
			// Add a separator before the component, unless the result
			// string already ends with one (which happens only if the
			// path *starts* with a separator).
			if (!result.empty() && result.lastChar() != sep)
				result += sep;

			// Add the component
			result += component;
		}

		// Skip over separator chars
		while (*cur == sep)
			cur++;
	}

	return result;
}

bool matchString(const char *str, const char *pat, bool ignoreCase, bool pathMode) {
	assert(str);
	assert(pat);

	const char *p = 0;
	const char *q = 0;

	for (;;) {
		if (pathMode && *str == '/') {
			p = 0;
			q = 0;
			if (*pat == '?')
				return false;
		}

		switch (*pat) {
		case '*':
			// Record pattern / string position for backtracking
			p = ++pat;
			q = str;
			// If pattern ended with * -> match
			if (!*pat)
				return true;
			break;

		default:
			if ((!ignoreCase && *pat != *str) ||
				(ignoreCase && tolower(*pat) != tolower(*str))) {
				if (p) {
					// No match, oops -> try to backtrack
					pat = p;
					str = ++q;
					if (!*str)
						return !*pat;
					break;
				}
				else
					return false;
			}
			// fallthrough
		case '?':
			if (!*str)
				return !*pat;
			pat++;
			str++;
		}
	}
}

}	// End of namespace Common
