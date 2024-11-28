/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_STR_BASE_H
#define COMMON_STR_BASE_H

#include "common/scummsys.h"
#include "common/str-enc.h"

#include <stdarg.h>

namespace Common {
template<class T>
class BaseString {
public:
	static void releaseMemoryPoolMutex();

	static const uint32 npos = 0xFFFFFFFF;
	typedef T          value_type;
	typedef T *        iterator;
	typedef const T *  const_iterator;

protected:
	/**
	 * The size of the internal storage. Increasing this means less heap
	 * allocations are needed, at the cost of more stack memory usage,
	 * and of course lots of wasted memory.
	 */
	static const uint32 _builtinCapacity = 32 - (sizeof(uint32) + sizeof(char *)) / sizeof(value_type);

	/**
	 * Length of the string. Stored to avoid having to call strlen
	 * a lot. Yes, we limit ourselves to strings shorter than 4GB --
	 * on purpose :-).
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
	constexpr BaseString() : _size(0), _str(_storage), _storage{0} {}

	/** Construct a string consisting of the given character. */
	explicit constexpr BaseString(value_type c) : _size((c == 0) ? 0 : 1), _str(_storage), _storage{c, 0} {}

	/** Construct a copy of the given string. */
	BaseString(const BaseString &str);

	/** Construct a string by moving an existing string. */
	BaseString(BaseString &&str);

	/** Construct a new string from the given NULL-terminated C string. */
	explicit BaseString(const value_type *str);

	/** Construct a new string containing exactly len characters read from address str. */
	BaseString(const value_type *str, uint32 len);

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	BaseString(const value_type *beginP, const value_type *endP);

	bool operator==(const BaseString &x) const;
	bool operator==(const value_type *x) const;
	bool operator!=(const BaseString &x) const;
	bool operator!=(const value_type *x) const;

	bool operator<(const BaseString &x) const;
	bool operator<(const value_type *x) const;
	bool operator<=(const BaseString &x) const;
	bool operator<=(const value_type *x) const;
	bool operator>(const BaseString &x) const;
	bool operator>(const value_type *x) const;
	bool operator>=(const BaseString &x) const;
	bool operator>=(const value_type *x) const;

	/**
	 * Compares whether two BaseString are the same based on memory comparison.
	 * This does *not* do comparison based on canonical equivalence.
	 */
	bool equals(const BaseString &x) const;
	bool equals(const value_type *x) const;
	bool equalsC(const char *x) const;
	int compareTo(const BaseString &x) const;           // strcmp clone
	int compareTo(const value_type *x) const;             // strcmp clone
	int compareToC(const char *x) const;             // strcmp clone

	/** Set character c at position p, replacing the previous character there. */
	void setChar(value_type c, uint32 p);

	/**
	 * Removes the value at position p from the string.
	 * Using this on decomposed characters will not remove the whole
	 * character!
	 */
	void deleteChar(uint32 p);

	/** Remove the last character from the string. */
	inline void deleteLastChar() { chop(1); }

	/** Remove all characters from position p to the p + len. If len = String::npos, removes all characters to the end */
	void erase(uint32 p, uint32 len = npos);

	/** Erases the character at the given iterator location */
	iterator erase(iterator it);

	/** Removes characters from the end of the string */
	void chop(uint32 len = 1);

	/** Clears the string, making it empty. */
	void clear();

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

	inline const value_type *c_str() const { return _str; }
	inline uint size() const         { return _size; }

	inline bool empty() const { return (_size == 0); }
	value_type firstChar() const    { return (_size > 0) ? _str[0] : 0; }
	value_type lastChar() const     { return (_size > 0) ? _str[_size - 1] : 0; }

	value_type operator[](int idx) const {
		assert(_str);
		assert(idx >= 0);
		assert(idx < (int)_size);
		return _str[idx];
	}

	/**
	 * Checks if a given string is present in the internal string or not.
	 */
	bool contains(const BaseString &otherString) const;
	bool contains(value_type x) const;

	/** Insert character c before position p. */
	void insertChar(value_type c, uint32 p);
	void insertString(const value_type *s, uint32 p);
	void insertString(const BaseString &s, uint32 p);

	/** Finds the index of a character in the string */
	uint32 find(value_type x, uint32 pos = 0) const;
	/** Does a find for the passed string */
	size_t find(const value_type *s, uint32 pos = 0) const;
	uint32 find(const BaseString &str, uint32 pos = 0) const;

	/** Does a reverse find for the passed string */
	size_t rfind(const value_type *s) const;
	size_t rfind(const BaseString &s) const {
		return rfind(s.c_str());
	}

	/** Does a reverse find for a passed character */
	size_t rfind(value_type c, size_t pos = npos) const;

	/** Find first character in the string matching the passed character */
	size_t findFirstOf(value_type c, size_t pos = 0) const;

	/** Find first character in the string that's any character of the passed string */
	size_t findFirstOf(const value_type *chars, size_t pos = 0) const;
	size_t findFirstOf(const BaseString &chars, size_t pos = 0) const {
		return findFirstOf(chars.c_str(), pos);
	}

	/** Find the last character in the string that's the specified character */
	size_t findLastOf(value_type c, size_t pos = npos) const;

	/** Find the last character in the string that's in any of the passed characters */
	size_t findLastOf(const value_type *chars, size_t pos = npos) const;
	size_t findLastOf(const BaseString &chars, size_t pos = npos) const {
		return findLastOf(chars.c_str(), pos);
	}

	/** Find first character in the string that's not the specified character */
	size_t findFirstNotOf(value_type c, size_t pos = 0) const;

	/** Find first character in the string that's not any character of the passed string */
	size_t findFirstNotOf(const value_type *chars, size_t pos = 0) const;
	size_t findFirstNotOf(const BaseString &chars, size_t pos = 0) const {
		return findFirstNotOf(chars.c_str(), pos);
	}

	/** Find the last character in the string that's not the specified character */
	size_t findLastNotOf(value_type c) const;

	/** Find the last character in the string that's not in any of the passed characters */
	size_t findLastNotOf(const value_type *chars) const;
	size_t findLastNotOf(const BaseString &chars) const {
		return findLastNotOf(chars.c_str());
	}

	/**@{
	 * Functions to replace some amount of chars with chars from some other string.
	 *
	 * @note The implementation follows that of the STL's std::string:
	 *       http://www.cplusplus.com/reference/string/string/replace/
	 *
	 * @param pos Starting position for the replace in the original string.
	 * @param count Number of chars to replace from the original string.
	 * @param str Source of the new chars.
	 * @param posOri Same as pos
	 * @param countOri Same as count
	 * @param posDest Initial position to read str from.
	 * @param countDest Number of chars to read from str. npos by default.
	 */
	// Replace 'count' bytes, starting from 'pos' with str.
	void replace(uint32 pos, uint32 count, const BaseString &str);
	// The same as above, but accepts a C-like array of characters.
	void replace(uint32 pos, uint32 count, const value_type *str);
	// Replace the characters in [begin, end) with str._str.
	void replace(iterator begin, iterator end, const BaseString &str);
	// Replace the characters in [begin, end) with str.
	void replace(iterator begin, iterator end, const value_type *str);
	// Replace _str[posOri, posOri + countOri) with
	// str._str[posDest, posDest + countDest)
	void replace(uint32 posOri, uint32 countOri, const BaseString &str,
					uint32 posDest, uint32 countDest);
	// Replace _str[posOri, posOri + countOri) with
	// str[posDest, posDest + countDest)
	void replace(uint32 posOri, uint32 countOri, const value_type *str,
					uint32 posDest, uint32 countDest);
	/**@}*/

	/**
	 * Replace all from characters in object by to character
	 * @param from the character to look for
	 * @param to The replacement character
	 */
	void replace(value_type from, value_type to);

	/** Appends a string containing the characters between beginP (including) and endP (excluding). */
	void append(const value_type *begin, const value_type *end);

	/**
	 * Wraps the text in the string to the given line maximum. Lines will be
	 * broken at any whitespace character. New lines are assumed to be
	 * represented using '\n'.
	 *
	 * This is a very basic line wrap which does not perform tab stop
	 * calculation, consecutive whitespace collapsing, auto-hyphenation, or line
	 * balancing.
	 */
	void wordWrap(const uint32 maxLength);

	/** Return uint64 corrensponding to String's contents. */
	uint64 asUint64() const;

	/** Return uint64 corrensponding to String's contents. This variant recognizes 0 (oct) and 0x (hex) prefixes. */
	uint64 asUint64Ext() const;

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

	/**
	 * Removes trailing and leading whitespaces. Uses isspace() to decide
	 * what is whitespace and what not.
	 */
	void trim();

	uint hash() const;

protected:
	~BaseString();

	void makeUnique() {
		ensureCapacity(_size, true);
	}

	void ensureCapacity(uint32 new_size, bool keep_old);
	void incRefCount() const;
	void decRefCount(int *oldRefCount);
	void initWithValueTypeStr(const value_type *str, uint32 len);

	void assignInsert(const value_type *str, uint32 p);
	void assignInsert(value_type c, uint32 p);
	void assignInsert(const BaseString &str, uint32 p);
	void assignAppend(const value_type *str);
	void assignAppend(value_type c);
	void assignAppend(const BaseString &str);
	void assign(const BaseString &str);
	void assign(BaseString &&str);
	void assign(value_type c);
	void assign(const value_type *str);

	bool pointerInOwnBuffer(const value_type *str) const;

	uint getUnsignedValue(uint pos) const;

	void toCase(int (*caseChangeFunc)(int));

	static uint32 cStrLen(const value_type *str);
	static const value_type *cMemChr(const value_type *ptr, value_type c, size_t count);
	static       value_type *cMemChr(value_type *ptr,       value_type c, size_t count);
	static int cMemCmp(const value_type* ptr1, const value_type* ptr2, size_t count);
};
}
#endif
