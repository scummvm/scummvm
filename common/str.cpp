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

#include "common/hash-str.h"
#include "common/list.h"
#include "common/memorypool.h"
#include "common/str.h"
#include "common/util.h"
#include "common/mutex.h"

namespace Common {

MemoryPool *g_refCountPool = nullptr; // FIXME: This is never freed right now
MutexRef g_refCountPoolMutex = nullptr;

void lockMemoryPoolMutex() {
	// The Mutex class can only be used once g_system is set and initialized,
	// but we may use the String class earlier than that (it is for example
	// used in the OSystem_POSIX constructor). However in those early stages
	// we can hope we don't have multiple threads either.
	if (!g_system || !g_system->backendInitialized())
		return;
	if (!g_refCountPoolMutex)
		g_refCountPoolMutex = g_system->createMutex();
	g_system->lockMutex(g_refCountPoolMutex);
}

void unlockMemoryPoolMutex() {
	if (g_refCountPoolMutex)
		g_system->unlockMutex(g_refCountPoolMutex);
}

void String::releaseMemoryPoolMutex() {
	if (g_refCountPoolMutex){
		g_system->deleteMutex(g_refCountPoolMutex);
		g_refCountPoolMutex = nullptr;
	}
}

static uint32 computeCapacity(uint32 len) {
	// By default, for the capacity we use the next multiple of 32
	return ((len + 32 - 1) & ~0x1F);
}

String::String(const char *str) : _size(0), _str(_storage) {
	if (str == nullptr) {
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
		_extern._capacity = computeCapacity(len + 1);
		_extern._refCount = nullptr;
		_str = new char[_extern._capacity];
		assert(_str != nullptr);
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
	assert(_str != nullptr);
}

String::String(char c)
	: _size(0), _str(_storage) {

	_storage[0] = c;
	_storage[1] = 0;

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

	// We need to allocate storage on the heap!

	// Compute a suitable new capacity limit
	// If the current capacity is sufficient we use the same capacity
	if (new_size < curCapacity)
		newCapacity = curCapacity;
	else
		newCapacity = MAX(curCapacity * 2, computeCapacity(new_size+1));

	// Allocate new storage
	newStorage = new char[newCapacity];
	assert(newStorage);


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
		_extern._refCount = nullptr;
		_extern._capacity = newCapacity;
	}
}

void String::incRefCount() const {
	assert(!isStorageIntern());
	if (_extern._refCount == nullptr) {
		lockMemoryPoolMutex();
		if (g_refCountPool == nullptr) {
			g_refCountPool = new MemoryPool(sizeof(int));
			assert(g_refCountPool);
		}

		_extern._refCount = (int *)g_refCountPool->allocChunk();
		unlockMemoryPoolMutex();
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
			lockMemoryPoolMutex();
			assert(g_refCountPool);
			g_refCountPool->freeChunk(oldRefCount);
			unlockMemoryPoolMutex();
		}
		delete[] _str;

		// Even though _str points to a freed memory block now,
		// we do not change its value, because any code that calls
		// decRefCount will have to do this afterwards anyway.
	}
}

String &String::operator=(const char *str) {
	uint32 len = strlen(str);
	ensureCapacity(len, false);
	_size = len;
	memmove(_str, str, len + 1);
	return *this;
}

String &String::operator=(const String &str) {
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

String &String::operator=(char c) {
	decRefCount(_extern._refCount);
	_str = _storage;

	_str[0] = c;
	_str[1] = 0;

	_size = (c == 0) ? 0 : 1;
	return *this;
}

String &String::operator+=(const char *str) {
	if (_str <= str && str <= _str + _size)
		return operator+=(String(str));

	int len = strlen(str);
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str, len + 1);
		_size += len;
	}
	return *this;
}

String &String::operator+=(const String &str) {
	if (&str == this)
		return operator+=(String(str));

	int len = str._size;
	if (len > 0) {
		ensureCapacity(_size + len, true);

		memcpy(_str + _size, str._str, len + 1);
		_size += len;
	}
	return *this;
}

String &String::operator+=(char c) {
	ensureCapacity(_size + 1, true);

	_str[_size++] = c;
	_str[_size] = 0;

	return *this;
}

bool String::hasPrefix(const String &x) const {
	return hasPrefix(x.c_str());
}

bool String::hasPrefix(const char *x) const {
	assert(x != nullptr);
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

bool String::hasPrefixIgnoreCase(const String &x) const {
	return hasPrefixIgnoreCase(x.c_str());
}

bool String::hasPrefixIgnoreCase(const char *x) const {
	assert(x != nullptr);
	// Compare x with the start of _str.
	const char *y = c_str();
	while (*x && tolower(*x) == tolower(*y)) {
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
	assert(x != nullptr);
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

bool String::hasSuffixIgnoreCase(const String &x) const {
	return hasSuffixIgnoreCase(x.c_str());
}

bool String::hasSuffixIgnoreCase(const char *x) const {
	assert(x != nullptr);
	// Compare x with the end of _str.
	const uint32 x_size = strlen(x);
	if (x_size > _size)
		return false;
	const char *y = c_str() + _size - x_size;
	while (*x && tolower(*x) == tolower(*y)) {
		++x;
		++y;
	}
	// It's a suffix, if and only if all letters in x are 'used up' before
	// _str ends.
	return *x == 0;
}

bool String::contains(const String &x) const {
	return strstr(c_str(), x.c_str()) != nullptr;
}

bool String::contains(const char *x) const {
	assert(x != nullptr);
	return strstr(c_str(), x) != nullptr;
}

bool String::contains(char x) const {
	return strchr(c_str(), x) != nullptr;
}

uint64 String::asUint64() const {
	uint64 result = 0;
	for (uint32 i = 0; i < _size; ++i) {
		if (_str[i] < '0' || _str[i] > '9') break;
		result = result * 10L + (_str[i] - '0');
	}
	return result;
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
		_str[p - 1] = _str[p];
	_size--;
}

void String::erase(uint32 p, uint32 len) {
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

void String::clear() {
	decRefCount(_extern._refCount);

	_size = 0;
	_str = _storage;
	_storage[0] = 0;
}

void String::setChar(char c, uint32 p) {
	assert(p < _size);

	makeUnique();
	_str[p] = c;
}

void String::insertChar(char c, uint32 p) {
	assert(p <= _size);

	ensureCapacity(_size + 1, true);
	_size++;
	for (uint32 i = _size; i > p; --i)
		_str[i] = _str[i - 1];
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
	while (_size >= 1 && isSpace(_str[_size - 1]))
		--_size;
	_str[_size] = 0;

	// Trim leading whitespace
	char *t = _str;
	while (isSpace(*t))
		t++;

	if (t != _str) {
		_size -= t - _str;
		memmove(_str, t, _size + 1);
	}
}

void String::wordWrap(const uint32 maxLength) {
	if (_size < maxLength) {
		return;
	}

	makeUnique();

	enum { kNoSpace = 0xFFFFFFFF };

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

uint String::hash() const {
	return hashit(c_str());
}

void String::replace(uint32 pos, uint32 count, const String &str) {
	replace(pos, count, str, 0, str._size);
}

void String::replace(uint32 pos, uint32 count, const char *str) {
	replace(pos, count, str, 0, strlen(str));
}

void String::replace(iterator begin_, iterator end_, const String &str) {
	replace(begin_ - _str, end_ - begin_, str._str, 0, str._size);
}

void String::replace(iterator begin_, iterator end_, const char *str) {
	replace(begin_ - _str, end_ - begin_, str, 0, strlen(str));
}

void String::replace(uint32 posOri, uint32 countOri, const String &str,
					 uint32 posDest, uint32 countDest) {
	replace(posOri, countOri, str._str, posDest, countDest);
}

void String::replace(uint32 posOri, uint32 countOri, const char *str,
					 uint32 posDest, uint32 countDest) {

	ensureCapacity(_size + countDest - countOri, true);

	// Prepare string for the replaced text.
	if (countOri < countDest) {
		uint32 offset = countDest - countOri; ///< Offset to copy the characters
		uint32 newSize = _size + offset;
		_size = newSize;

		// Push the old characters to the end of the string
		for (uint32 i = _size; i >= posOri + countDest; i--)
			_str[i] = _str[i - offset];

	} else if (countOri > countDest){
		uint32 offset = countOri - countDest; ///< Number of positions that we have to pull back

		// Pull the remainder string back
		for (uint32 i = posOri + countDest; i < _size; i++)
			_str[i] = _str[i + offset];

		_size -= offset;
	}

	// Copy the replaced part of the string
	for (uint32 i = 0; i < countDest; i++)
		_str[posOri + i] = str[posDest + i];

}

// static
String String::format(const char *fmt, ...) {
	String output;

	va_list va;
	va_start(va, fmt);
	output = String::vformat(fmt, va);
	va_end(va);

	return output;
}

// static
String String::vformat(const char *fmt, va_list args) {
	String output;
	assert(output.isStorageIntern());

	va_list va;
	scumm_va_copy(va, args);
	int len = vsnprintf(output._str, _builtinCapacity, fmt, va);
	va_end(va);

	if (len == -1 || len == _builtinCapacity - 1) {
		// MSVC and IRIX don't return the size the full string would take up.
		// MSVC returns -1, IRIX returns the number of characters actually written,
		// which is at the most the size of the buffer minus one, as the string is
		// truncated to fit.

		// We assume MSVC failed to output the correct, null-terminated string
		// if the return value is either -1 or size.
		// For IRIX, because we lack a better mechanism, we assume failure
		// if the return value equals size - 1.
		// The downside to this is that whenever we try to format a string where the
		// size is 1 below the built-in capacity, the size is needlessly increased.

		// Try increasing the size of the string until it fits.
		int size = _builtinCapacity;
		do {
			size *= 2;
			output.ensureCapacity(size - 1, false);
			assert(!output.isStorageIntern());
			size = output._extern._capacity;

			scumm_va_copy(va, args);
			len = vsnprintf(output._str, size, fmt, va);
			va_end(va);
		} while (len == -1 || len >= size - 1);
		output._size = len;
	} else if (len < (int)_builtinCapacity) {
		// vsnprintf succeeded
		output._size = len;
	} else {
		// vsnprintf didn't have enough space, so grow buffer
		output.ensureCapacity(len, false);
		scumm_va_copy(va, args);
		int len2 = vsnprintf(output._str, len + 1, fmt, va);
		va_end(va);
		assert(len == len2);
		output._size = len2;
	}

	return output;
}


#pragma mark -

bool String::operator==(const String &x) const {
	return equals(x);
}

bool String::operator==(const char *x) const {
	assert(x != nullptr);
	return equals(x);
}

bool String::operator!=(const String &x) const {
	return !equals(x);
}

bool String::operator !=(const char *x) const {
	assert(x != nullptr);
	return !equals(x);
}

bool String::operator<(const String &x) const {
	return compareTo(x) < 0;
}

bool String::operator<=(const String &x) const {
	return compareTo(x) <= 0;
}

bool String::operator>(const String &x) const {
	return compareTo(x) > 0;
}

bool String::operator>=(const String &x) const {
	return compareTo(x) >= 0;
}

#pragma mark -

bool operator==(const char* y, const String &x) {
	return (x == y);
}

bool operator!=(const char* y, const String &x) {
	return x != y;
}

#pragma mark -

bool String::equals(const String &x) const {
	return (0 == compareTo(x));
}

bool String::equals(const char *x) const {
	assert(x != nullptr);
	return (0 == compareTo(x));
}

bool String::equalsIgnoreCase(const String &x) const {
	return (0 == compareToIgnoreCase(x));
}

bool String::equalsIgnoreCase(const char *x) const {
	assert(x != nullptr);
	return (0 == compareToIgnoreCase(x));
}

int String::compareTo(const String &x) const {
	return compareTo(x.c_str());
}

int String::compareTo(const char *x) const {
	assert(x != nullptr);
	return strcmp(c_str(), x);
}

int String::compareToIgnoreCase(const String &x) const {
	return compareToIgnoreCase(x.c_str());
}

int String::compareToIgnoreCase(const char *x) const {
	assert(x != nullptr);
	return scumm_stricmp(c_str(), x);
}

#pragma mark -

String operator+(const String &x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const char *x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const String &x, const char *y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(char x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const String &x, char y) {
	String temp(x);
	temp += y;
	return temp;
}

char *ltrim(char *t) {
	while (isSpace(*t))
		t++;
	return t;
}

char *rtrim(char *t) {
	int l = strlen(t) - 1;
	while (l >= 0 && isSpace(t[l]))
		t[l--] = 0;
	return t;
}

char *trim(char *t) {
	return rtrim(ltrim(t));
}

String lastPathComponent(const String &path, const char sep) {
	const char *str = path.c_str();
	const char *last = str + path.size();

	// Skip over trailing slashes
	while (last > str && *(last - 1) == sep)
		--last;

	// Path consisted of only slashes -> return empty string
	if (last == str)
		return String();

	// Now scan the whole component
	const char *first = last - 1;
	while (first > str && *first != sep)
		--first;

	if (*first == sep)
		first++;

	return String(first, last);
}

String normalizePath(const String &path, const char sep) {
	if (path.empty())
		return path;

	const char *cur = path.c_str();
	String result;

	// If there is a leading slash, preserve that:
	if (*cur == sep) {
		result += sep;
		// Skip over multiple leading slashes, so "//" equals "/"
		while (*cur == sep)
			++cur;
	}

	// Scan for path components till the end of the String
	List<String> comps;
	while (*cur != 0) {
		const char *start = cur;

		// Scan till the next path separator resp. the end of the string
		while (*cur != sep && *cur != 0)
			cur++;

		const String component(start, cur);

		if (component.empty() || component == ".") {
			// Skip empty components and dot components
		} else if (!comps.empty() && component == ".." && comps.back() != "..") {
			// If stack is non-empty and top is not "..", remove top
			comps.pop_back();
		} else {
			// Add the component to the stack
			comps.push_back(component);
		}

		// Skip over separator chars
		while (*cur == sep)
			cur++;
	}

	// Finally, assemble all components back into a path
	while (!comps.empty()) {
		result += comps.front();
		comps.pop_front();
		if (!comps.empty())
			result += sep;
	}

	return result;
}

bool matchString(const char *str, const char *pat, bool ignoreCase, bool pathMode) {
	assert(str);
	assert(pat);

	const char *p = nullptr;
	const char *q = nullptr;
	bool escaped = false;

	for (;;) {
		if (pathMode && *str == '/') {
			p = nullptr;
			q = nullptr;
			if (*pat == '?')
				return false;
		}

		const char curPat = *pat;
		switch (*pat) {
		case '*':
			if (*str) {
				// Record pattern / string position for backtracking
				p = ++pat;
				q = str;
			} else {
				// If we've reached the end of str, we can't backtrack further
				// NB: We can't simply check if pat also ended here, because
				// the pattern might end with any number of *s.
				++pat;
				p = nullptr;
				q = nullptr;
			}
			// If pattern ended with * -> match
			if (!*pat)
				return true;
			break;

		case '\\':
			if (!escaped) {
				pat++;
				break;
			}
			// fallthrough

		case '#':
			// treat # as a wildcard for digits unless escaped
			if (!escaped) {
				if (!isDigit(*str))
					return false;
				pat++;
				str++;
				break;
			}
			// fallthrough

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

		escaped = !escaped && (curPat == '\\');
	}
}

void replace(Common::String &source, const Common::String &what, const Common::String &with) {
	const char *cstr = source.c_str();
	const char *position = strstr(cstr, what.c_str());
	if (position) {
		uint32 index = position - cstr;
		source.replace(index, what.size(), with);
	}
}

String tag2string(uint32 tag) {
	char str[5];
	str[0] = (char)(tag >> 24);
	str[1] = (char)(tag >> 16);
	str[2] = (char)(tag >> 8);
	str[3] = (char)tag;
	str[4] = '\0';
	// Replace non-printable chars by dot
	for (int i = 0; i < 4; ++i) {
		if (!Common::isPrint(str[i]))
			str[i] = '.';
	}
	return String(str);
}

size_t strlcpy(char *dst, const char *src, size_t size) {
	// Our backup of the source's start, we need this
	// to calculate the source's length.
	const char * const srcStart = src;

	// In case a non-empty size was specified we
	// copy over (size - 1) bytes at max.
	if (size != 0) {
		// Copy over (size - 1) bytes at max.
		while (--size != 0) {
			if ((*dst++ = *src) == 0)
				break;
			++src;
		}

		// In case the source string was longer than the
		// destination, we need to add a terminating
		// zero.
		if (size == 0)
			*dst = 0;
	}

	// Move to the terminating zero of the source
	// string, we need this to determine the length
	// of the source string.
	while (*src)
		++src;

	// Return the source string's length.
	return src - srcStart;
}

size_t strlcat(char *dst, const char *src, size_t size) {
	// In case the destination buffer does not contain
	// space for at least 1 character, we will just
	// return the source string's length.
	if (size == 0)
		return strlen(src);

	// Our backup of the source's start, we need this
	// to calculate the source's length.
	const char * const srcStart = src;

	// Our backup of the destination's start, we need
	// this to calculate the destination's length.
	const char * const dstStart = dst;

	// Search the end of the destination, but do not
	// move past the terminating zero.
	while (size-- != 0 && *dst != 0)
		++dst;

	// Calculate the destination's length;
	const size_t dstLength = dst - dstStart;

	// In case we reached the end of the destination
	// buffer before we had a chance to append any
	// characters we will just return the destination
	// length plus the source string's length.
	if (size == 0)
		return dstLength + strlen(srcStart);

	// Copy over all of the source that fits
	// the destination buffer. We also need
	// to take the terminating zero we will
	// add into consideration.
	while (size-- != 0 && *src != 0)
		*dst++ = *src++;
	*dst = 0;

	// Move to the terminating zero of the source
	// string, we need this to determine the length
	// of the source string.
	while (*src)
		++src;

	// Return the total length of the result string
	return dstLength + (src - srcStart);
}

size_t strnlen(const char *src, size_t maxSize) {
	size_t counter = 0;
	while (counter != maxSize && *src++)
		++counter;
	return counter;
}

} // End of namespace Common

// Portable implementation of stricmp / strcasecmp / strcmpi.
// TODO: Rename this to Common::strcasecmp
int scumm_stricmp(const char *s1, const char *s2) {
	byte l1, l2;
	do {
		// Don't use ++ inside tolower, in case the macro uses its
		// arguments more than once.
		l1 = (byte)*s1++;
		l1 = tolower(l1);
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

// Portable implementation of strnicmp / strncasecmp / strncmpi.
// TODO: Rename this to Common::strncasecmp
int scumm_strnicmp(const char *s1, const char *s2, uint n) {
	byte l1, l2;
	do {
		if (n-- == 0)
			return 0; // no difference found so far -> signal equality

		// Don't use ++ inside tolower, in case the macro uses its
		// arguments more than once.
		l1 = (byte)*s1++;
		l1 = tolower(l1);
		l2 = (byte)*s2++;
		l2 = tolower(l2);
	} while (l1 == l2 && l1 != 0);
	return l1 - l2;
}

//  Portable implementation of strdup.
char *scumm_strdup(const char *in) {
	const size_t len = strlen(in) + 1;
	char *out = new char[len];
	if (out) {
		strcpy(out, in);
	}
	return out;
}
