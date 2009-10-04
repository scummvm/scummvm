/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include "common/sys.h"
#include "common/array.h"

namespace Common {

/**
 * Simple string class for ScummVM. Provides automatic storage managment,
 * and overloads several operators in a 'natural' fashion, mimicking
 * the std::string class. Even provides simple iterators.
 *
 * This class tries to avoid allocating lots of small blocks on the heap,
 * since that is inefficient on several platforms supported by ScummVM.
 * Instead, small strings are stored 'inside' the string object (i.e. on
 * the stack, for stack allocated objects), and only for strings exceeding
 * a certain length do we allocate a buffer on the heap.
 */
class String {
protected:
	/**
	 * The size of the internal storage. Increasing this means less heap
	 * allocations are needed, at the cost of more stack memory usage,
	 * and of course lots of wasted memory. Empirically, 90% or more of
	 * all String instances are less than 32 chars long. If a platform
	 * is very short on stack space, it would be possible to lower this.
	 * A value of 24 still seems acceptable, though considerably worse,
	 * while 16 seems to be the lowest you want to go... Anything lower
	 * than 8 makes no sense, since that's the size of member _extern
	 * (on 32 bit machines; 12 bytes on systems with 64bit pointers).
	 */
	static const uint32 _builtinCapacity = 32 - sizeof(uint32) - sizeof(char*);

	/**
	 * Length of the string. Stored to avoid having to call strlen
	 * a lot. Yes, we limit ourselves to strings shorter than 4GB --
	 * on purpose :-).
	 */
	uint32		_size;

	/**
	 * Pointer to the actual string storage. Either points to _storage,
	 * or to a block allocated on the heap via malloc.
	 */
	char		*_str;


	union {
		/**
		 * Internal string storage.
		 */
		char _storage[_builtinCapacity];
		/**
		 * External string storage data -- the refcounter, and the
		 * capacity of the string _str points to.
		 */
		struct {
			mutable int *_refCount;
			uint32		_capacity;
		} _extern;
	};

	inline bool isStorageIntern() const {
		return _str == _storage;
	}

public:
#if !(defined(PALMOS_ARM) || defined(PALMOS_DEBUG) || defined(__GP32__))
	static const String emptyString;
#else
	static const char *emptyString;
#endif

	/** Construct a new empty string. */
	String() : _size(0), _str(_storage) { _storage[0] = 0; }

	/** Construct a new string from the given NULL-terminated C string. */
	String(const char *str);

	/** Construct a new string containing exactly len characters read from address str. */
	String(const char *str, uint32 len);

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	String(const char *beginP, const char *endP);

	/** Construct a copy of the given string. */
	String(const String &str);

	/** Construct a string consisting of the given character. */
	explicit String(char c);

	~String();

	String &operator  =(const char *str);
	String &operator  =(const String &str);
	String &operator  =(char c);
	String &operator +=(const char *str);
	String &operator +=(const String &str);
	String &operator +=(char c);

	bool operator ==(const String &x) const;
	bool operator ==(const char *x) const;
	bool operator !=(const String &x) const;
	bool operator !=(const char *x) const;

	bool operator <(const String &x) const;
	bool operator <=(const String &x) const;
	bool operator >(const String &x) const;
	bool operator >=(const String &x) const;

	bool equals(const String &x) const;
	bool equalsIgnoreCase(const String &x) const;
	int compareTo(const String &x) const;	// strcmp clone
	int compareToIgnoreCase(const String &x) const;	// stricmp clone

	bool equals(const char *x) const;
	bool equalsIgnoreCase(const char *x) const;
	int compareTo(const char *x) const;	// strcmp clone
	int compareToIgnoreCase(const char *x) const;	// stricmp clone

	bool hasSuffix(const String &x) const;
	bool hasSuffix(const char *x) const;

	bool hasPrefix(const String &x) const;
	bool hasPrefix(const char *x) const;

	bool contains(const String &x) const;
	bool contains(const char *x) const;
	bool contains(char x) const;

	/**
	 * Simple DOS-style pattern matching function (understands * and ? like used in DOS).
	 * Taken from exult/files/listfiles.cc
	 *
	 * Token meaning:
	 *		"*": any character, any amount of times.
	 *		"?": any character, only once.
	 *
	 * Example strings/patterns:
	 *		String: monkey.s01	 Pattern: monkey.s??	=> true
	 *		String: monkey.s101	 Pattern: monkey.s??	=> false
	 *		String: monkey.s99	 Pattern: monkey.s?1	=> false
	 *		String: monkey.s101	 Pattern: monkey.s*		=> true
	 *		String: monkey.s99	 Pattern: monkey.s*1	=> false
	 *
	 * @param str Text to be matched against the given pattern.
	 * @param pat Glob pattern.
	 * @param pathMode Whether to use path mode, i.e., whether slashes must be matched explicitly.
	 *
	 * @return true if str matches the pattern, false otherwise.
	 */
	bool matchString(const char *pat, bool pathMode = false) const;
	bool matchString(const String &pat, bool pathMode = false) const;


	inline const char *c_str() const		{ return _str; }
	inline uint size() const				{ return _size; }

	inline bool empty() const	{ return (_size == 0); }
	char lastChar() const	{ return (_size > 0) ? _str[_size-1] : 0; }

	char operator[](int idx) const {
		assert(_str && idx >= 0 && idx < (int)_size);
		return _str[idx];
	}

	/** Remove the last character from the string. */
	void deleteLastChar();

	/** Remove the character at position p from the string. */
	void deleteChar(uint32 p);

	/** Set character c at position p, replacing the previous character there. */
	void setChar(char c, uint32 p);

	/** Insert character c before position p. */
	void insertChar(char c, uint32 p);

	/** Clears the string, making it empty. */
	void clear();

	/** Convert all characters in the string to lowercase. */
	void toLowercase();

	/** Convert all characters in the string to uppercase. */
	void toUppercase();

	/**
	 * Removes trailing and leading whitespaces. Uses isspace() to decide
	 * what is whitespace and what not.
	 */
	void trim();

	uint hash() const;

public:
	typedef char *        iterator;
	typedef const char *  const_iterator;

	iterator		begin() {
		return _str;
	}

	iterator		end() {
		return begin() + size();
	}

	const_iterator	begin() const {
		return _str;
	}

	const_iterator	end() const {
		return begin() + size();
	}

protected:
	void makeUnique();
	void ensureCapacity(uint32 new_size, bool keep_old);
	void incRefCount() const;
	void decRefCount(int *oldRefCount);
	void initWithCStr(const char *str, uint32 len);
};

// Append two strings to form a new (temp) string
String operator +(const String &x, const String &y);

String operator +(const char *x, const String &y);
String operator +(const String &x, const char *y);

String operator +(const String &x, char y);
String operator +(char x, const String &y);

// Some useful additional comparison operators for Strings
bool operator == (const char *x, const String &y);
bool operator != (const char *x, const String &y);

// Utility functions to remove leading and trailing whitespaces
extern char *ltrim(char *t);
extern char *rtrim(char *t);
extern char *trim(char *t);


/**
 * Returns the last component of a given path.
 *
 * Examples:
 *			/foo/bar.txt would return 'bar.txt'
 *			/foo/bar/    would return 'bar'
 *			/foo/./bar//    would return 'bar'
 *
 * @param path the path of which we want to know the last component
 * @param sep character used to separate path components
 * @return The last component of the path.
 */
Common::String lastPathComponent(const Common::String &path, const char sep);

/**
 * Normalize a gien path to a canonical form. In particular:
 * - trailing separators are removed:  /foo/bar/ -> /foo/bar
 * - double separators (= empty components) are removed:   /foo//bar -> /foo/bar
 * - dot components are removed:  /foo/./bar -> /foo/bar
 *
 * @todo remove double dot components:  /foo/baz/../bar -> /foo/bar
 *
 * @param path	the path to normalize
 * @param sep	the separator token (usually '/' on Unix-style systems, or '\\' on Windows based stuff)
 * @return	the normalized path
 */
Common::String normalizePath(const Common::String &path, const char sep);


/**
 * Simple DOS-style pattern matching function (understands * and ? like used in DOS).
 * Taken from exult/files/listfiles.cc
 *
 * Token meaning:
 *		"*": any character, any amount of times.
 *		"?": any character, only once.
 *
 * Example strings/patterns:
 *		String: monkey.s01	 Pattern: monkey.s??	=> true
 *		String: monkey.s101	 Pattern: monkey.s??	=> false
 *		String: monkey.s99	 Pattern: monkey.s?1	=> false
 *		String: monkey.s101	 Pattern: monkey.s*		=> true
 *		String: monkey.s99	 Pattern: monkey.s*1	=> false
 *
 * @param str Text to be matched against the given pattern.
 * @param pat Glob pattern.
 * @param pathMode Whether to use path mode, i.e., whether slashes must be matched explicitly.
 *
 * @return true if str matches the pattern, false otherwise.
 */
bool matchString(const char *str, const char *pat, bool pathMode = false);


typedef Array<String> StringList;

}	// End of namespace Common

#endif
