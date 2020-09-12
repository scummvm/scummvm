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
#include "unicode-bidi.h"

namespace Common {

extern MemoryPool *g_refCountPool;

static uint32 computeCapacity(uint32 len) {
	// By default, for the capacity we use the next multiple of 32
	return ((len + 32 - 1) & ~0x1F);
}

U32String::U32String(const value_type *str) : _size(0), _str(_storage) {
	if (str == nullptr) {
		_storage[0] = 0;
		_size = 0;
	} else {
		uint32 len = 0;
		const value_type *s = str;
		while (*s++) {
			++len;
		}
		initWithCStr(str, len);
	}
}

U32String::U32String(const value_type *str, uint32 len) : _size(0), _str(_storage) {
	initWithCStr(str, len);
}

U32String::U32String(const value_type *beginP, const value_type *endP) : _size(0), _str(_storage) {
	assert(endP >= beginP);
	initWithCStr(beginP, endP - beginP);
}

U32String::U32String(const U32String &str)
    : _size(str._size) {
	if (str.isStorageIntern()) {
		// String in internal storage: just copy it
		memcpy(_storage, str._storage, _builtinCapacity * sizeof(value_type));
		_str = _storage;
	} else {
		// String in external storage: use refcount mechanism
		str.incRefCount();
		_extern._refCount = str._extern._refCount;
		_extern._capacity = str._extern._capacity;
		_str = str._str;
	}
	assert(_str != nullptr);
}

U32String::U32String(const char *str) : _size(0), _str(_storage) {
	if (str == nullptr) {
		_storage[0] = 0;
		_size = 0;
	} else {
		initWithCStr(str, strlen(str));
	}
}

U32String::U32String(const char *str, uint32 len) : _size(0), _str(_storage) {
	initWithCStr(str, len);
}

U32String::U32String(const char *beginP, const char *endP) : _size(0), _str(_storage) {
	assert(endP >= beginP);
	initWithCStr(beginP, endP - beginP);
}

U32String::U32String(const String &str) : _size(0), _str(_storage) {
	initWithCStr(str.c_str(), str.size());
}

U32String::U32String(const UnicodeBiDiText &txt) : _size(0), _str(_storage) {
	initWithCStr(txt.visual.c_str(), txt.visual.size());
}

U32String::~U32String() {
	decRefCount(_extern._refCount);
}

U32String &U32String::operator=(const U32String &str) {
	if (&str == this)
		return *this;

	if (str.isStorageIntern()) {
		decRefCount(_extern._refCount);
		_size = str._size;
		_str = _storage;
		memcpy(_str, str._str, (_size + 1) * sizeof(value_type));
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

U32String &U32String::operator=(const String &str) {
	clear();
	initWithCStr(str.c_str(), str.size());
	return *this;
}

U32String &U32String::operator=(const value_type *str) {
	return U32String::operator=(U32String(str));
}

U32String &U32String::operator=(const char *str) {
	clear();
	initWithCStr(str, strlen(str));
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

bool U32String::operator==(const U32String &x) const {
	return equals(x);
}

bool U32String::operator==(const String &x) const {
	return equals(x);
}

bool U32String::operator==(const value_type *x) const {
	return equals(U32String(x));
}

bool U32String::operator==(const char *x) const {
	return equals(x);
}

bool U32String::operator!=(const U32String &x) const {
	return !equals(x);
}

bool U32String::operator!=(const String &x) const {
	return !equals(x);
}

bool U32String::operator!=(const value_type *x) const {
	return !equals(U32String(x));
}

bool U32String::operator!=(const char *x) const {
	return !equals(x);
}

bool U32String::operator<(const U32String &x) const {
	for (uint32 i = 0, n = x.size(); i < _size && i < n; ++i) {
		uint32 sc = _str[i];
		uint32 xc = x[i];
		if (sc < xc)
			return true;
		else if (sc > xc)
			return false;
	}
	return (_size < x.size());
}

bool U32String::operator<=(const U32String &x) const {
	return !operator>(x);
}

bool U32String::operator>(const U32String &x) const {
	for (uint i = 0, n = x.size(); i < _size && i < n; ++i) {
		uint32 sc = _str[i];
		uint32 xc = x[i];
		if (sc > xc)
			return true;
		else if (sc < xc)
			return false;
	}
	return (_size > x.size());
}

bool U32String::operator>=(const U32String &x) const {
	return !operator<(x);
}

bool U32String::equals(const U32String &x) const {
	if (this == &x || _str == x._str) {
		return true;
	}

	if (x.size() != _size) {
		return false;
	}

	return !memcmp(_str, x._str, _size * sizeof(value_type));
}

bool U32String::equals(const String &x) const {
	if (x.size() != _size)
		return false;

	for (uint32 idx = 0; idx < _size; ++idx)
		if (_str[idx] != static_cast<value_type>(x[idx]))
			return false;

	return true;
}

bool U32String::contains(value_type x) const {
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] == x) {
			return true;
		}
	}

	return false;
}

bool U32String::contains(const U32String &otherString) const {
	if (empty() || otherString.empty() || _size < otherString.size()) {
		return false;
	}

	uint32 size = 0;
	U32String::const_iterator itr = otherString.begin();

	for (U32String::const_iterator itr2 = begin(); itr != otherString.end() && itr2 != end(); itr2++) {
		if (*itr == *itr2) {
			itr++;
			size++;
			if (size == otherString.size())
				return true;
		} else {
			size = 0;
			itr = otherString.begin();
		}
	}

	return false;
}

void U32String::insertChar(value_type c, uint32 p) {
	assert(p <= _size);

	ensureCapacity(_size + 1, true);
	_size++;
	for (uint32 i = _size; i > p; --i)
		_str[i] = _str[i - 1];
	_str[p] = c;
}

void U32String::insertString(String s, uint32 p) {
	for (String::iterator i = s.begin(); i != s.end(); i++) {
		U32String::insertChar(*i, p++);
	}
}

void U32String::insertString(value_type *s, uint32 p) {
	while (*s != '\0') {
		U32String::insertChar(*s++, p++);
	}
}

void U32String::deleteChar(uint32 p) {
	assert(p < _size);

	makeUnique();
	while (p++ < _size)
		_str[p - 1] = _str[p];
	_size--;
}

void U32String::deleteLastChar() {
	if (_size > 0)
		deleteChar(_size - 1);
}

void U32String::erase(uint32 p, uint32 len) {
	assert(p < _size);

	makeUnique();
	// If len == npos or p + len is over the end, remove all the way to the end
	if (len == npos || p + len >= _size) {
		// Delete char at p as well. So _size = (p - 1) + 1
		_size = p;
		// Null terminate
		_str[_size] = 0;
		return;
	}

	for ( ; p + len <= _size; p++) {
		_str[p] = _str[p + len];
	}
	_size -= len;
}

void U32String::clear() {
	decRefCount(_extern._refCount);

	_size = 0;
	_str = _storage;
	_storage[0] = 0;
}

void U32String::toLowercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] < 128) {
			_str[i] = tolower(_str[i]);
		}
	}
}

void U32String::toUppercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] < 128) {
			_str[i] = toupper(_str[i]);
		}
	}
}

uint32 U32String::find(value_type x, uint32 pos) const {
	for (uint32 i = pos; i < _size; ++i) {
		if (_str[i] == x) {
			return i;
		}
	}

	return npos;
}

uint32 U32String::find(const U32String &str, uint32 pos) const {
	if (pos >= _size) {
		return npos;
	}

	const value_type *strP = str.c_str();

	for (const_iterator cur = begin() + pos; *cur; ++cur) {
		uint i = 0;
		while (true) {
			if (!strP[i]) {
				return cur - begin();
			}

			if (cur[i] != strP[i]) {
				break;
			}

			++i;
		}
	}

	return npos;
}

void U32String::makeUnique() {
	ensureCapacity(_size, true);
}

void U32String::ensureCapacity(uint32 new_size, bool keep_old) {
	bool isShared;
	uint32 curCapacity, newCapacity;
	value_type *newStorage;
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
		// If the current capacity is sufficient we use the same capacity
		if (new_size < curCapacity)
			newCapacity = curCapacity;
		else
			newCapacity = MAX(curCapacity * 2, computeCapacity(new_size + 1));

		// Allocate new storage
		newStorage = new value_type[newCapacity];
		assert(newStorage);
	}

	// Copy old data if needed, elsewise reset the new storage.
	if (keep_old) {
		assert(_size < newCapacity);
		memcpy(newStorage, _str, (_size + 1) * sizeof(value_type));
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
		_extern._refCount = nullptr;
		_extern._capacity = newCapacity;
	}
}

void U32String::incRefCount() const {
	assert(!isStorageIntern());
	if (_extern._refCount == nullptr) {
		if (g_refCountPool == nullptr) {
			g_refCountPool = new MemoryPool(sizeof(int));
			assert(g_refCountPool);
		}

		_extern._refCount = (int *)g_refCountPool->allocChunk();
		*_extern._refCount = 2;
	} else {
		++(*_extern._refCount);
	}
}

void U32String::decRefCount(int *oldRefCount) {
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
		// Coverity thinks that we always free memory, as it assumes
		// (correctly) that there are cases when oldRefCount == 0
		// Thus, DO NOT COMPILE, trick it and shut tons of false positives
#ifndef __COVERITY__
		delete[] _str;
#endif

		// Even though _str points to a freed memory block now,
		// we do not change its value, because any code that calls
		// decRefCount will have to do this afterwards anyway.
	}
}

void U32String::initWithCStr(const value_type *str, uint32 len) {
	assert(str);

	_storage[0] = 0;

	_size = len;

	if (len >= _builtinCapacity) {
		// Not enough internal storage, so allocate more
		_extern._capacity = computeCapacity(len + 1);
		_extern._refCount = nullptr;
		_str = new value_type[_extern._capacity];
		assert(_str != nullptr);
	}

	// Copy the string into the storage area
	memmove(_str, str, len * sizeof(value_type));
	_str[len] = 0;
}

void U32String::initWithCStr(const char *str, uint32 len) {
	assert(str);

	_storage[0] = 0;

	_size = len;

	if (len >= _builtinCapacity) {
		// Not enough internal storage, so allocate more
		_extern._capacity = computeCapacity(len + 1);
		_extern._refCount = nullptr;
		_str = new value_type[_extern._capacity];
		assert(_str != nullptr);
	}

	// Copy the string into the storage area
	for (size_t idx = 0; idx < len; ++idx, ++str)
		_str[idx] = (byte)(*str);

	_str[len] = 0;
}


U32String operator+(const U32String &x, const U32String &y) {
	U32String temp(x);
	temp += y;
	return temp;
}

void U32String::wordWrap(const uint32 maxLength) {
	if (_size < maxLength) {
		return;
	}

	makeUnique();

	const uint32 kNoSpace = 0xFFFFFFFF;

	uint32 i = 0;
	while (i < _size) {
		uint32 lastSpace = kNoSpace;
		uint32 x = 0;
		while (i < _size && x <= maxLength) {
			const char c = _str[i];
			if (c == '\n') {
				lastSpace = kNoSpace;
				x = 0;
			} else {
				if (Common::isSpace(c)) {
					lastSpace = i;
				}
				++x;
			}
			++i;
		}

		if (x > maxLength) {
			if (lastSpace == kNoSpace) {
				insertChar('\n', i - 1);
			} else {
				setChar('\n', lastSpace);
				i = lastSpace + 1;
			}
		}
	}
}

uint64 U32String::asUint64() const {
	uint64 result = 0;
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] < '0' || _str[i] > '9') break;
		result = result * 10L + (_str[i] - '0');
	}
	return result;
}

void U32String::trim() {
	if (_size == 0)
		return;

	makeUnique();

	// Trim trailing whitespace
	while (_size >= 1 && isSpace(_str[_size - 1]))
		--_size;
	_str[_size] = 0;

	// Trim leading whitespace
	value_type *t = _str;
	while (isSpace(*t))
		t++;

	if (t != _str) {
		_size -= t - _str;
		memmove(_str, t, _size + 1);
	}
}

U32String U32String::format(U32String fmt, ...) {
	U32String output;

	va_list va;
	va_start(va, fmt);
	U32String::vformat(fmt.begin(), fmt.end(), output, va);
	va_end(va);

	return output;
}

U32String U32String::format(const char *fmt, ...) {
	U32String output;

	Common::U32String fmtU32(fmt);
	va_list va;
	va_start(va, fmt);
	U32String::vformat(fmtU32.begin(), fmtU32.end(), output, va);
	va_end(va);

	return output;
}

int U32String::vformat(U32String::const_iterator fmt, const U32String::const_iterator inputItrEnd, U32String &output, va_list args) {
	int int_temp;
	char *string_temp;

	value_type ch;
	value_type *u32string_temp;
	int length = 0;
	int len = 0;
	int pos = 0;
	int tempPos = 0;

	char buffer[512];

	while (fmt != inputItrEnd) {
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
				len = strlen(string_temp);
				length += len;

				output.insertString(string_temp, pos);
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
