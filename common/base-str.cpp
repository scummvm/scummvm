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

#include "common/base-str.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/memorypool.h"
#include "common/util.h"
#include "common/mutex.h"

namespace Common {

#define TEMPLATE template<class T>
#define BASESTRING BaseString<T>

MemoryPool *g_refCountPool = nullptr; // FIXME: This is never freed right now
#ifndef SCUMMVM_UTIL
Mutex *g_refCountPoolMutex = nullptr;

void lockMemoryPoolMutex() {
	// The Mutex class can only be used once g_system is set and initialized,
	// but we may use the String class earlier than that (it is for example
	// used in the OSystem_POSIX constructor). However in those early stages
	// we can hope we don't have multiple threads either.
	if (!g_system || !g_system->backendInitialized())
		return;
	if (!g_refCountPoolMutex)
		g_refCountPoolMutex = new Mutex();
	g_refCountPoolMutex->lock();
}

void unlockMemoryPoolMutex() {
	if (g_refCountPoolMutex)
		g_refCountPoolMutex->unlock();
}

TEMPLATE void BASESTRING::releaseMemoryPoolMutex() {
	if (g_refCountPoolMutex){
		delete g_refCountPoolMutex;
		g_refCountPoolMutex = nullptr;
	}
}

#endif

static uint32 computeCapacity(uint32 len) {
	// By default, for the capacity we use the next multiple of 32
	return ((len + 32 - 1) & ~0x1F);
}

TEMPLATE
BASESTRING::BaseString(const BASESTRING &str)
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

TEMPLATE BASESTRING::BaseString(const value_type *str) : _size(0), _str(_storage) {
	if (str == nullptr) {
		_storage[0] = 0;
		_size = 0;
	} else {
		uint32 len = 0;
		const value_type *s = str;
		while (*s++) {
			++len;
		}
		initWithValueTypeStr(str, len);
	}
}

TEMPLATE BASESTRING::BaseString(const value_type *str, uint32 len) : _size(0), _str(_storage) {
	initWithValueTypeStr(str, len);
}

TEMPLATE BASESTRING::BaseString(const value_type *beginP, const value_type *endP) : _size(0), _str(_storage) {
	assert(endP >= beginP);
	initWithValueTypeStr(beginP, endP - beginP);
}

TEMPLATE BASESTRING::~BaseString() {
	decRefCount(_extern._refCount);
}

TEMPLATE void BASESTRING::makeUnique() {
	ensureCapacity(_size, true);
}

TEMPLATE void BASESTRING::ensureCapacity(uint32 new_size, bool keep_old) {
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

TEMPLATE
void BASESTRING::incRefCount() const {
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

TEMPLATE
void BASESTRING::decRefCount(int *oldRefCount) {
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

TEMPLATE void BASESTRING::initWithValueTypeStr(const value_type *str, uint32 len) {
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

TEMPLATE bool BASESTRING::equals(const BaseString &x) const {
	if (this == &x || _str == x._str) {
		return true;
	}

	if (x.size() != _size) {
		return false;
	}

	return !memcmp(_str, x._str, _size * sizeof(value_type));
}

TEMPLATE bool BASESTRING::equals(const value_type *ptr) const {
	if (_str == ptr) {
		return true;
	}

	uint i = 0;

	for (; i < _size && *ptr; i++, ptr++) {
		if (_str[i] != *ptr)
			return false;
	}

	if (i == _size && *ptr == 0) {
		return true;
	}

	return false;
}

TEMPLATE bool BASESTRING::equalsC(const char *ptr) const {
	uint i = 0;

	for (; i < _size && *ptr; i++, ptr++) {
		if (_str[i] != (T)*ptr)
			return false;
	}

	if (i == _size && *ptr == 0) {
		return true;
	}

	return false;
}

TEMPLATE bool BASESTRING::operator==(const BaseString &x) const {
	return equals(x);
}

TEMPLATE bool BASESTRING::operator==(const value_type *x) const {
	return equals(x);
}

TEMPLATE bool BASESTRING::operator!=(const BaseString &x) const {
	return !equals(x);
}

TEMPLATE bool BASESTRING::operator!=(const value_type *x) const {
	return !equals(x);
}

TEMPLATE int BASESTRING::compareTo(const BaseString &x) const {
	for (uint32 i = 0, n = x.size(); i < _size && i < n; ++i) {
		uint32 sc = _str[i];
		uint32 xc = x[i];
		if (sc < xc)
			return -1;
		else if (sc > xc)
			return +1;
	}
	if (_size < x.size())
		return -1;
	if (_size == x.size())
		return 0;
	return +1;
}

TEMPLATE int BASESTRING::compareTo(const value_type *ptr) const {
	uint32 i = 0;
	for (; i < _size && *ptr; ++i, ptr++) {
		uint32 sc = _str[i];
		uint32 xc = *ptr;
		if (sc < xc)
			return -1;
		else if (sc > xc)
			return +1;
	}
	if (i == _size && *ptr == 0)
		return 0;
	if (*ptr == 0)
		return +1;
	return -1;
}

TEMPLATE int BASESTRING::compareToC(const char *ptr) const {
	uint32 i = 0;
	for (; i < _size && *ptr; ++i, ptr++) {
		uint32 sc = _str[i];
		uint32 xc = *ptr;
		if (sc < xc)
			return -1;
		else if (sc > xc)
			return +1;
	}
	if (i == _size && *ptr == 0)
		return 0;
	if (*ptr == 0)
		return +1;
	return -1;
}

TEMPLATE bool BASESTRING::operator<(const BaseString &x) const {
	return compareTo(x) < 0;
}

TEMPLATE bool BASESTRING::operator<=(const BaseString &x) const {
	return compareTo(x) <= 0;
}

TEMPLATE bool BASESTRING::operator>(const BaseString &x) const {
	return compareTo(x) > 0;
}

TEMPLATE bool BASESTRING::operator>=(const BaseString &x) const {
	return compareTo(x) >= 0;
}

TEMPLATE bool BASESTRING::operator<(const value_type *x) const {
	return compareTo(x) < 0;
}

TEMPLATE bool BASESTRING::operator<=(const value_type *x) const {
	return compareTo(x) <= 0;
}

TEMPLATE bool BASESTRING::operator>(const value_type *x) const {
	return compareTo(x) > 0;
}

TEMPLATE bool BASESTRING::operator>=(const value_type *x) const {
	return compareTo(x) >= 0;
}

TEMPLATE bool BASESTRING::contains(value_type x) const {
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] == x) {
			return true;
		}
	}

	return false;
}

TEMPLATE bool BASESTRING::contains(const BaseString &otherString) const {
	if (empty() || otherString.empty() || _size < otherString.size()) {
		return false;
	}

	uint32 sizeMatch = 0;
	typename BASESTRING::const_iterator itr = otherString.begin();

	for (typename BASESTRING::const_iterator itr2 = begin(); itr != otherString.end() && itr2 != end(); itr2++) {
		if (*itr == *itr2) {
			itr++;
			sizeMatch++;
			if (sizeMatch == otherString.size())
				return true;
		} else {
			sizeMatch = 0;
			itr = otherString.begin();
		}
	}

	return false;
}

TEMPLATE void BASESTRING::insertChar(value_type c, uint32 p) {
	assert(p <= _size);

	ensureCapacity(_size + 1, true);
	_size++;
	for (uint32 i = _size; i > p; --i)
		_str[i] = _str[i - 1];
	_str[p] = c;
}

TEMPLATE void BASESTRING::deleteChar(uint32 p) {
	assert(p < _size);

	makeUnique();
	while (p++ < _size)
		_str[p - 1] = _str[p];
	_size--;
}

TEMPLATE void BASESTRING::deleteLastChar() {
	if (_size > 0)
		deleteChar(_size - 1);
}

TEMPLATE void BASESTRING::erase(uint32 p, uint32 len) {
	if (len == 0)
		return;

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

TEMPLATE typename BASESTRING::iterator BASESTRING::erase(iterator it) {
	this->deleteChar(it - _str);
	return it;
}

TEMPLATE void BASESTRING::clear() {
	decRefCount(_extern._refCount);

	_size = 0;
	_str = _storage;
	_storage[0] = 0;
}

	
TEMPLATE void BASESTRING::setChar(value_type c, uint32 p) {
	assert(p < _size);

	makeUnique();
	_str[p] = c;
}

TEMPLATE void BASESTRING::assign(const BaseString &str) {
	if (&str == this)
		return;

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
}

TEMPLATE void BASESTRING::assign(value_type c) {
	decRefCount(_extern._refCount);
	_str = _storage;

	_str[0] = c;
	_str[1] = 0;

	_size = (c == 0) ? 0 : 1;
}

TEMPLATE void BASESTRING::insertString(const value_type *s, uint32 p) {
	while (*s != '\0') {
		BaseString::insertChar(*s++, p++);
	}
}

TEMPLATE void BASESTRING::insertString(const BaseString &s, uint32 p) {
	for (uint i = 0; i < s._size; i++) {
		BaseString::insertChar(s[i], p+i);
	}
}

TEMPLATE uint32 BASESTRING::find(value_type x, uint32 pos) const {
	for (uint32 i = pos; i < _size; ++i) {
		if (_str[i] == x) {
			return i;
		}
	}

	return npos;
}

TEMPLATE uint32 BASESTRING::find(const BaseString &str, uint32 pos) const {
	if (pos >= _size) {
		return npos;
	}

	const value_type *strP = str.c_str();

	for (const_iterator cur = begin() + pos; cur != end(); ++cur) {
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

TEMPLATE size_t BASESTRING::find(const value_type *strP, uint32 pos) const {
	if (pos >= _size) {
		return npos;
	}

	for (const_iterator cur = begin() + pos; cur != end(); ++cur) {
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

TEMPLATE uint64 BASESTRING::asUint64() const {
	uint64 result = 0;
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] < '0' || _str[i] > '9') break;
		result = result * 10L + (_str[i] - '0');
	}
	return result;
}

TEMPLATE uint64 BASESTRING::asUint64Ext() const {
	uint64 result = 0;
	uint64 base = 10;
	uint32 skip = 0;
	
	if (_size >= 3 && _str[0] == '0' && _str[1] == 'x') {
		base = 16;
		skip = 2;
	} else if (_size >= 2 && _str[0] == '0') {
		base = 8;
		skip = 1;
	} else {
		base = 10;
		skip = 0;
	}
	for (uint32 i = skip; i < _size; ++i) {
		char digit = _str[i];
		uint64 digitval = 17; // sentinel
		if (digit >= '0' && digit <= '9')
			digitval = digit - '0';
		else if (digit >= 'a' && digit <= 'f')
			digitval = digit - 'a' + 10;
		else if (digit >= 'A' && digit <= 'F')
			digitval = digit - 'A' + 10;
		if (digitval > base)
			break;
		result = result * base + digitval;
	}
	return result;
}

#ifndef SCUMMVM_UTIL

TEMPLATE void BASESTRING::wordWrap(const uint32 maxLength) {
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

#endif

TEMPLATE void BASESTRING::toLowercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] > 0 && _str[i] < 128) {
			_str[i] = tolower(_str[i]);
		}
	}
}

TEMPLATE void BASESTRING::toUppercase() {
	makeUnique();
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] > 0 && _str[i] < 128) {
			_str[i] = toupper(_str[i]);
		}
	}
}

#ifndef SCUMMVM_UTIL

TEMPLATE void BASESTRING::trim() {
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
		memmove(_str, t, (_size + 1) * sizeof(_str[0]));
	}
}
#endif

TEMPLATE void BASESTRING::assignAppend(value_type c) {
	if (c == 0)
		return;
	ensureCapacity(_size + 1, true);

	_str[_size++] = c;
	_str[_size] = 0;
}

TEMPLATE void BASESTRING::assignAppend(const BaseString &str) {
	if (&str == this) {
		assignAppend(BaseString(str));
		return;
	}

	int len = str._size;
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str._str, (len + 1) * sizeof(value_type));
		_size += len;
	}
}

TEMPLATE bool BASESTRING::pointerInOwnBuffer(const value_type *str) const {
	//compared pointers must be in the same array or UB
	//cast to intptr however is IB
	//which includes comparision of the values
	uintptr ownBuffStart = (uintptr)_str;
	uintptr ownBuffEnd = (uintptr)(_str + _size);
	uintptr candidateAddr = (uintptr)str;
	return ownBuffStart <= candidateAddr && candidateAddr <= ownBuffEnd;
}

TEMPLATE void BASESTRING::assignAppend(const value_type *str) {
	if (pointerInOwnBuffer(str)) {
		assignAppend(BaseString(str));
		return;
	}

	uint32 len;
	for (len = 0; str[len]; len++);
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str, (len + 1) * sizeof(value_type));
		_size += len;
	}
}

TEMPLATE void BASESTRING::assign(const value_type *str) {
	uint32 len;
	for (len = 0; str[len]; len++);
	ensureCapacity(len, false);
	_size = len;
	memmove(_str, str, (len + 1) * sizeof(value_type));
}

TEMPLATE uint BASESTRING::getUnsignedValue(uint pos) const {
	const int shift = (sizeof(uint) - sizeof(value_type)) * 8;
	return ((uint)_str[pos]) << shift >> shift;
}

// Hash function for strings, taken from CPython.
TEMPLATE uint BASESTRING::hash() const {
	uint hashResult = getUnsignedValue(0) << 7;
	for (uint i = 0; i < _size; i++) {
		hashResult = (1000003 * hashResult) ^ getUnsignedValue(i);
	}
	return hashResult ^ _size;
}

template class BaseString<char>;
template class BaseString<u32char_type_t>;

}
