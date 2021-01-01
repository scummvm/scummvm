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
#include "common/str-enc.h"
#include "common/base-str.h"

namespace Common {

/**
 * @defgroup common_ustr UTF-32 strings
 * @ingroup common
 *
 * @brief API for working with UTF-32 strings.
 *
 * @{
 */
 
class String;

/**
 * A simple string class for UTF-32 strings in ScummVM. The main intention
 * behind this class is to feature a simple way of displaying UTF-32 strings
 * through the Graphics::Font API.
 *
 * Note that operations like equals, deleteCharacter, toUppercase, etc.
 * are only simplified convenience operations. They might not fully work
 * as you would expect for a proper UTF-32 string class.
 *
 * The presence of \0 characters in the string will cause undefined
 * behavior in some operations.
 */
#ifdef USE_CXX11
typedef char32_t u32char_type_t;
#else
typedef uint32 u32char_type_t;
#endif

class U32String : public BaseString<u32char_type_t> {
public:
	typedef uint32 unsigned_type; /*!< Unsigned version of the underlying type. */
public:
	/** Construct a new empty string. */
	U32String() : BaseString<u32char_type_t>() {}

	/** Construct a new string from the given null-terminated C string. */
	explicit U32String(const value_type *str) : BaseString<u32char_type_t>(str) {}

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	U32String(const value_type *str, uint32 len) : BaseString<u32char_type_t>(str, len) {}

#ifdef USE_CXX11
	explicit U32String(const uint32 *str) : BaseString<u32char_type_t>((const value_type *) str) {}
	U32String(const uint32 *str, uint32 len) : BaseString<u32char_type_t>((const value_type *) str, len) {}
	U32String(const uint32 *beginP, const uint32 *endP) : BaseString<u32char_type_t>((const value_type *) beginP, (const value_type *) endP) {}
#endif

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	U32String(const value_type *beginP, const value_type *endP) : BaseString<u32char_type_t>(beginP, endP) {}

	/** Construct a copy of the given string. */
	U32String(const U32String &str) : BaseString<u32char_type_t>(str) {}

	/** Construct a new string from the given null-terminated C string that uses the given @p page encoding. */
	explicit U32String(const char *str, CodePage page = kUtf8);

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	U32String(const char *str, uint32 len, CodePage page = kUtf8);

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	U32String(const char *beginP, const char *endP, CodePage page = kUtf8);

	/** Construct a copy of the given string. */
	U32String(const String &str, CodePage page = kUtf8);

	/** Assign a given string to this string. */
	U32String &operator=(const U32String &str);

	/** @overload */
	U32String &operator=(const String &str);

	/** @overload */
	U32String &operator=(const value_type *str);

	/** @overload */
	U32String &operator=(const char *str);

	/** Append the given string to this string. */
	U32String &operator+=(const U32String &str);

	/** @overload */
	U32String &operator+=(value_type c);

	using BaseString<value_type>::operator==;
	using BaseString<value_type>::operator!=;

	/** Check whether this string is identical to string @p x. */
	bool operator==(const String &x) const;

	/** @overload */
	bool operator==(const char *x) const;

	/** Check whether this string is different than string @p x. */
	bool operator!=(const String &x) const;

	/** @overload */
	bool operator!=(const char *x) const;

	/** Convert the string to the given @p page encoding and return the result as a new String. */
	String encode(CodePage page = kUtf8) const;

	/**
	 * Print formatted data into a U32String object.
	 *
	 * Similar to sprintf, except that it stores the result
	 * in a (variably sized) string instead of a fixed-size buffer.
	 */
	static U32String format(U32String fmt, ...);

	/** @overload **/
	static U32String format(const char *fmt, ...);

	/**
	 * Print formatted data into a U32String object.
	 * The method takes in the output by reference and works with iterators.
	 */
	static int vformat(const value_type *fmt, const value_type *fmtEnd, U32String &output, va_list args);

	/**
	 * Helper function for vformat. Convert an int to string.
	 * Minimal implementation, only for base 10.
	 */
	static char* itoa(int num, char* str, int base);

	using BaseString<value_type>::insertString;
	void insertString(const char *s, uint32 p, CodePage page = kUtf8);   /*!< Insert string @p s into this string at position @p p. */
	void insertString(const String &s, uint32 p, CodePage page = kUtf8); /*!< @overload */

	/** Return a substring of this string */
	U32String substr(size_t pos = 0, size_t len = npos) const;

	const uint32 *u32_str() const {   /*!< Return the string as a UTF-32 pointer. */
		return (const uint32 *) _str;
	}

	/** Decode a big endian UTF-16 string into a U32String. */
	static Common::U32String decodeUTF16BE(const uint16 *start, uint len);

	/** Decode a little endian UTF-16 string into a U32String. */
	static Common::U32String decodeUTF16LE(const uint16 *start, uint len);

	/** Decode a native UTF-16 string into a U32String. */
	static Common::U32String decodeUTF16Native(const uint16 *start, uint len);

	/** Transform a U32String into UTF-16 representation (big endian). The result must be freed. */
	uint16 *encodeUTF16BE(uint *len = nullptr) const;

	/** Transform a U32String into UTF-16 representation (native endian). The result must be freed. */
	uint16 *encodeUTF16LE(uint *len = nullptr) const;

	/** Transform a U32String into UTF-16 representation (native encoding). The result must be freed. */
	uint16 *encodeUTF16Native(uint *len = nullptr) const;

private:
	void decodeInternal(const char *str, uint32 len, CodePage page);
	void decodeOneByte(const char *str, uint32 len, CodePage page);
    	void decodeWindows932(const char *src, uint32 len);
	void decodeWindows949(const char *src, uint32 len);
    	void decodeWindows950(const char *src, uint32 len);
	void decodeUTF8(const char *str, uint32 len);
		
	friend class String;
};

/** Concatenate strings @p x and @p y. */
U32String operator+(const U32String &x, const U32String &y);

/** Append the given @p y character to the given @p x string. */
U32String operator+(const U32String &x, U32String::value_type y);

/** @} */

} // End of namespace Common

#endif
