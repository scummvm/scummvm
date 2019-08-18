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
	initWithCStr(str.c_str(), str.size());
	return *this;
}

U32String &U32String::operator=(const value_type *str) {
	return U32String::operator=(U32String(str));
}

U32String &U32String::operator=(const char *str) {
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

	for (size_t idx = 0; idx < _size; ++idx)
		if (_str[idx] != (value_type)x[idx])
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

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. This version is unsafe! There are no checks for end of buffer
//    near i++ operations.
// 2. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 3. Original version returns a result code. This version does NOT
//    insert 'FFFD' on errors & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
U32String convertUtf8ToUtf32(const String &str) {
	// The String class, and therefore the Font class as well, assume one
	// character is one byte, but in this case it's actually an UTF-8
	// string with up to 4 bytes per character. To work around this,
	// convert it to an U32String before drawing it, because our Font class
	// can handle that.
	Common::U32String u32str;
	uint i = 0;
	while (i < str.size()) {
		uint32 chr = 0;
		if ((str[i] & 0xF8) == 0xF0) {
			chr |= (str[i++] & 0x07) << 18;
			chr |= (str[i++] & 0x3F) << 12;
			chr |= (str[i++] & 0x3F) << 6;
			chr |= (str[i++] & 0x3F);
		} else if ((str[i] & 0xF0) == 0xE0) {
			chr |= (str[i++] & 0x0F) << 12;
			chr |= (str[i++] & 0x3F) << 6;
			chr |= (str[i++] & 0x3F);
		} else if ((str[i] & 0xE0) == 0xC0) {
			chr |= (str[i++] & 0x1F) << 6;
			chr |= (str[i++] & 0x3F);
		} else {
			chr = (str[i++] & 0x7F);
		}
		u32str += chr;
	}
	return u32str;
}

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version is more effective.
//    This version features buffer = (char)(...) + buffer; pattern that causes
//    unnecessary copying and reallocations, original code works with raw bytes
// 2. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 3. Original version returns a result code. This version inserts '0xFFFD' if
//    character does not fit in 4 bytes & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
String convertUtf32ToUtf8(const U32String &u32str) {
	static const uint8 firstByteMark[5] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };

	Common::String str;
	uint i = 0;
	while (i < u32str.size()) {
		unsigned short bytesToWrite = 0;
		const uint32 byteMask = 0xBF;
		const uint32 byteMark = 0x80;

		uint32 ch = u32str[i++];
		if (ch < (uint32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (uint32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (uint32)0x10000) {
			bytesToWrite = 3;
		} else if (ch <= 0x0010FFFF) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = 0x0000FFFD;
		}
		
		Common::String buffer;

		switch (bytesToWrite) {
		case 4:
			buffer = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 3:
			buffer = (char)((ch | byteMark) & byteMask) + buffer;
			ch >>= 6;
			// fallthrough
		case 2:
			buffer = (char)((ch | byteMark) & byteMask) + buffer;
			ch >>= 6;
			// fallthrough
		case 1:
			buffer = (char)(ch | firstByteMark[bytesToWrite]) + buffer;
		}

		str += buffer;
	}
	return str;
}

static const uint32 g_windows1250ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0083, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x0088, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x0098, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
										 0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7,
										 0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
										 0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
										 0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
										 0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7,
										 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
										 0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7,
										 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
										 0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7,
										 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
										 0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7,
										 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9};

static const uint32 g_windows1251ConversionTable[] = {0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
										 0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x0098, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
										 0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
										 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
										 0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
										 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
										 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
										 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
										 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
										 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
										 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
										 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
										 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
										 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F};

static const uint32 g_windows1252ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
										 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
										 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
										 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
										 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
										 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
										 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
										 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
										 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
										 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
										 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
										 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
										 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF};

static const uint32 g_windows1253ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x0088, 0x2030, 0x008A, 0x2039, 0x008C, 0x008D, 0x008E, 0x008F,
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x0098, 0x2122, 0x009A, 0x203A, 0x009C, 0x009D, 0x009E, 0x009F,
										 0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
										 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
										 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7,
										 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
										 0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
										 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
										 0x03A0, 0x03A1, 0x00D2, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7,
										 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
										 0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7,
										 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
										 0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7,
										 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x00FF};

static const uint32 g_windows1254ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 
										 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x008E, 0x008F, 
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 
										 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x009E, 0x0178, 
										 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 
										 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
										 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
										 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
										 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 
										 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
										 0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
										 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
										 0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
										 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
										 0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
										 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF};

static const uint32 g_windows1255ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x02C6, 0x2030, 0x008A, 0x2039, 0x008C, 0x008D, 0x008E, 0x008F,
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x02DC, 0x2122, 0x009A, 0x203A, 0x009C, 0x009D, 0x009E, 0x009F,
										 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7,
										 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
										 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
										 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
										 0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7,
										 0x05B8, 0x05B9, 0x05BA, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
										 0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3,
										 0x05F4, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
										 0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
										 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
										 0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
										 0x05E8, 0x05E9, 0x05EA, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x00FF};

static const uint32 g_windows1257ConversionTable[] = {0x20AC, 0x0081, 0x201A, 0x0083, 0x201E, 0x2026, 0x2020, 0x2021,
										 0x0088, 0x2030, 0x008A, 0x2039, 0x008C, 0x00A8, 0x02C7, 0x00B8,
										 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
										 0x0098, 0x2122, 0x009A, 0x203A, 0x009C, 0x00AF, 0x02DB, 0x009F,
										 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
										 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
										 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
										 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
										 0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
										 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
										 0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
										 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
										 0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
										 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
										 0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
										 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9};

U32String convertToU32String(const char *str, CodePage page) {
	const String string(str);
	if (page == kUtf8) {
		return convertUtf8ToUtf32(string);
	}

	U32String unicodeString;
	for (uint i = 0; i < string.size(); ++i) {
		if ((byte)string[i] <= 0x7F) {
			unicodeString += string[i];
			continue;
		}

		byte index = string[i] - 0x80;

		switch (page) {
		case kWindows1250:
			unicodeString += g_windows1250ConversionTable[index];
			break;
		case kWindows1251:
			unicodeString += g_windows1251ConversionTable[index];
			break;
		case kWindows1252:
			unicodeString += g_windows1252ConversionTable[index];
			break;
		case kWindows1253:
			unicodeString += g_windows1253ConversionTable[index];
			break;
		case kWindows1254:
			unicodeString += g_windows1254ConversionTable[index];
			break;
		case kWindows1255:
			unicodeString += g_windows1255ConversionTable[index];
			break;
		case kWindows1257:
			unicodeString += g_windows1257ConversionTable[index];
			break;
		default:
			break;
		}
	}
	return unicodeString;
}

String convertFromU32String(const U32String &string, CodePage page) {
	if (page == kUtf8) {
		return convertUtf32ToUtf8(string);
	}

	const uint32 *conversionTable = NULL;
	switch (page) {
	case kWindows1250:
		conversionTable = g_windows1250ConversionTable;
		break;
	case kWindows1251:
		conversionTable = g_windows1251ConversionTable;
		break;
	case kWindows1252:
		conversionTable = g_windows1252ConversionTable;
		break;
	case kWindows1253:
		conversionTable = g_windows1253ConversionTable;
		break;
	case kWindows1254:
		conversionTable = g_windows1254ConversionTable;
		break;
	case kWindows1255:
		conversionTable = g_windows1255ConversionTable;
		break;
	case kWindows1257:
		conversionTable = g_windows1257ConversionTable;
		break;
	default:
		break;
	}

	String charsetString;
	for (uint i = 0; i < string.size(); ++i) {
		if (string[i] <= 0x7F) {
			charsetString += string[i];
			continue;
		}
		
		if (!conversionTable) {
			continue;
		}

		for (uint j = 0; j < 128; ++j) {
			if (conversionTable[j] == string[i]) {
				charsetString += (char)(j + 0x80);
				break;
			}
		}
	}	
	return charsetString;
}

} // End of namespace Common
