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
 */

#ifndef COMMON_USTR_H
#define COMMON_USTR_H

#include "common/scummsys.h"

namespace Common {

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
	U32String() : _size(0), _str(_storage) { _storage[0] = 0; }

	explicit U32String(const value_type *str);

	U32String(const value_type *str, uint32 len);

	U32String(const value_type *beginP, const value_type *endP);

	U32String(const U32String &str);

	~U32String();

	U32String &operator=(const U32String &str);
	U32String &operator+=(const U32String &str);
	U32String &operator+=(value_type c);

	bool operator==(const U32String &x) const { return equals(x); }

	bool equals(const U32String &x) const;

	bool contains(value_type x) const;

	inline const value_type *c_str() const { return _str; }
	inline uint32 size() const             { return _size; }

	inline bool empty() const { return (_size == 0); }

	value_type operator[](int idx) const {
		assert(_str && idx >= 0 && idx < (int)_size);
		return _str[idx];
	}

	/** Remove the character at position p from the string. */
	void deleteChar(uint32 p);

	/** Clears the string, making it empty. */
	void clear();

	/** Convert all characters in the string to lowercase. */
	void toLowercase();

	/** Convert all characters in the string to uppercase. */
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
};

} // End of namespace Common

#endif
