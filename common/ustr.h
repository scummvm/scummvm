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

#ifndef COMMON_USTR_H
#define COMMON_USTR_H

#include "common/scummsys.h"

namespace Common {

class String;

/**
 * Very simple string class for UTF-32 strings in ScummVM. The main intention
 * behind this class is to feature a simple way of displaying UTF-32 strings
 * through the Graphics::Font API.
 *
 * Please note that operations like equals, deleteCharacter, toUppercase, etc.
 * are only very simplified convenience operations. They might not fully work
 * as you would expect for a proper UTF-32 string class.
 *
 * The presence of \0 characters in the string will cause undefined
 * behavior in some operations.
 */
class U32String {
public:
	static const uint32 npos = 0xFFFFFFFF;

	typedef uint32 value_type;
	typedef uint32 unsigned_type;
private:
	/**
	 * The size of the internal storage. Increasing this means less heap
	 * allocations are needed, at the cost of more stack memory usage,
	 * and of course lots of wasted memory.
	 */
	static const uint32 _builtinCapacity = 32;

	/**
	 * Length of the string.
	 */
	uint32 _size;

	/**
	 * Pointer to the actual string storage. Either points to _storage,
	 * or to a block allocated on the heap via malloc.
	 */
	value_type  *_str;


	union {
		/**
		 * Internal string storage.
		 */
		value_type _storage[_builtinCapacity];
		/**
		 * External string storage data -- the refcounter, and the
		 * capacity of the string _str points to.
		 */
		struct {
			mutable int *_refCount;
			uint32       _capacity;
		} _extern;
	};

	inline bool isStorageIntern() const {
		return _str == _storage;
	}

public:
	/** Construct a new empty string. */
	U32String() : _size(0), _str(_storage) { _storage[0] = 0; }

	/** Construct a new string from the given NULL-terminated C string. */
	explicit U32String(const value_type *str);

	/** Construct a new string containing exactly len characters read from address str. */
	U32String(const value_type *str, uint32 len);

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	U32String(const value_type *beginP, const value_type *endP);

	/** Construct a copy of the given string. */
	U32String(const U32String &str);

	/** Construct a new string from the given NULL-terminated C string. */
	explicit U32String(const char *str);

	/** Construct a new string containing exactly len characters read from address str. */
	U32String(const char *str, uint32 len);

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	U32String(const char *beginP, const char *endP);

	/** Construct a copy of the given string. */
	U32String(const String &str);

	~U32String();

	U32String &operator=(const U32String &str);
	U32String &operator=(const String &str);
	U32String &operator=(const value_type *str);
	U32String &operator=(const char *str);
	U32String &operator+=(const U32String &str);
	U32String &operator+=(value_type c);
	bool operator==(const U32String &x) const;
	bool operator==(const String &x) const;
	bool operator==(const value_type *x) const;
	bool operator==(const char *x) const;
	bool operator!=(const U32String &x) const;
	bool operator!=(const String &x) const;
	bool operator!=(const value_type *x) const;
	bool operator!=(const char *x) const;

	/**
	 * Compares whether two U32String are the same based on memory comparison.
	 * This does *not* do comparison based on canonical equivalence.
	 */
	bool equals(const U32String &x) const;

	/**
	 * Compares whether two U32String are the same based on memory comparison.
	 * This does *not* do comparison based on canonical equivalence.
	 */
	bool equals(const String &x) const;

	bool contains(value_type x) const;

	inline const value_type *c_str() const { return _str; }
	inline uint32 size() const             { return _size; }

	inline bool empty() const { return (_size == 0); }

	value_type operator[](int idx) const {
		assert(_str && idx >= 0 && idx < (int)_size);
		return _str[idx];
	}

	/**
	 * Removes the value at position p from the string.
	 * Using this on decomposed characters will not remove the whole
	 * character!
	 */
	void deleteChar(uint32 p);

	/** Clears the string, making it empty. */
	void clear();

	/**
	 * Convert all characters in the string to lowercase.
	 *
	 * Be aware that this only affects the case of ASCII characters. All
	 * other characters will not be touched at all.
	 */
	void toLowercase();

	/**
	 * Convert all characters in the string to uppercase.
	 *
	 * Be aware that this only affects the case of ASCII characters. All
	 * other characters will not be touched at all.
	 */
	void toUppercase();

	uint32 find(const U32String &str, uint32 pos = 0) const;

	typedef value_type *        iterator;
	typedef const value_type *  const_iterator;

	iterator begin() {
		// Since the user could potentially
		// change the string via the returned
		// iterator we have to assure we are
		// pointing to a unique storage.
		makeUnique();

		return _str;
	}

	iterator end() {
		return begin() + size();
	}

	const_iterator begin() const {
		return _str;
	}

	const_iterator end() const {
		return begin() + size();
	}

private:
	void makeUnique();
	void ensureCapacity(uint32 new_size, bool keep_old);
	void incRefCount() const;
	void decRefCount(int *oldRefCount);
	void initWithCStr(const value_type *str, uint32 len);
	void initWithCStr(const char *str, uint32 len);
};

U32String convertUtf8ToUtf32(const String &str);
String convertUtf32ToUtf8(const U32String &str);

enum CodePage {
	kUtf8,
	kWindows1250,
	kWindows1251,
	kWindows1252,
	kWindows1253,
	kWindows1254,
	kWindows1255,
	kWindows1257
};

U32String convertToU32String(const char *str, CodePage page = kUtf8);
String convertFromU32String(const U32String &str, CodePage page = kUtf8);

} // End of namespace Common

#endif
