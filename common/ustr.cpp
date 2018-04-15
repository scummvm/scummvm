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
#include "common/memorypool.h"
#include "common/util.h"

namespace Common {

extern MemoryPool *g_refCountPool;

static uint32 computeCapacity(uint32 len) {
	// By default, for the capacity we use the next multiple of 32
	return ((len + 32 - 1) & ~0x1F);
}

U32String::U32String(const value_type *str) : _size(0), _str(_storage) {
	if (str == 0) {
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
	assert(_str != 0);
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

bool U32String::equals(const U32String &x) const {
	if (this == &x || _str == x._str) {
		return true;
	}

	if (x.size() != _size) {
		return false;
	}

	return !memcmp(_str, x._str, _size * sizeof(value_type));
}

bool U32String::contains(value_type x) const {
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] == x) {
			return true;
		}
	}

	return false;
}

void U32String::deleteChar(uint32 p) {
	assert(p < _size);

	makeUnique();
	while (p++ < _size)
		_str[p - 1] = _str[p];
	_size--;
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
		_extern._refCount = 0;
		_extern._capacity = newCapacity;
	}
}

void U32String::incRefCount() const {
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
		delete[] _str;

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
		_extern._refCount = 0;
		_str = new value_type[_extern._capacity];
		assert(_str != 0);
	}

	// Copy the string into the storage area
	memmove(_str, str, len * sizeof(value_type));
	_str[len] = 0;
}

} // End of namespace Common
