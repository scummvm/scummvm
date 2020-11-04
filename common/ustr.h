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
 * @ingroup common_str
 *
 * @brief API for working with UTF-32 strings.
 *
 * @{
 */
 
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
#ifdef USE_CXX11
typedef char32_t u32char_type_t;
#else
typedef uint32 u32char_type_t;
#endif

class U32String : public BaseString<u32char_type_t> {
public:
	typedef uint32 unsigned_type;
public:
	/** Construct a new empty string. */
	U32String() : BaseString<u32char_type_t>() {}

	/** Construct a new string from the given NULL-terminated C string. */
	explicit U32String(const value_type *str) : BaseString<u32char_type_t>(str) {}

	/** Construct a new string containing exactly len characters read from address str. */
	U32String(const value_type *str, uint32 len) : BaseString<u32char_type_t>(str, len) {}

#ifdef USE_CXX11
	explicit U32String(const uint32 *str) : BaseString<u32char_type_t>((const value_type *) str) {}
	U32String(const uint32 *str, uint32 len) : BaseString<u32char_type_t>((const value_type *) str, len) {}
	U32String(const uint32 *beginP, const uint32 *endP) : BaseString<u32char_type_t>((const value_type *) beginP, (const value_type *) endP) {}
#endif

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	U32String(const value_type *beginP, const value_type *endP) : BaseString<u32char_type_t>(beginP, endP) {}

	/** Construct a copy of the given string. */
	U32String(const U32String &str) : BaseString<u32char_type_t>(str) {}

	/** Construct a new string from the given NULL-terminated C string. */
	explicit U32String(const char *str);

	/** Construct a new string containing exactly len characters read from address str. */
	U32String(const char *str, uint32 len);

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	U32String(const char *beginP, const char *endP);

	/** Construct a copy of the given string. */
	U32String(const String &str);

	U32String &operator=(const U32String &str);
	U32String &operator=(const String &str);
	U32String &operator=(const value_type *str);
	U32String &operator=(const char *str);
	U32String &operator+=(const U32String &str);
	U32String &operator+=(value_type c);
	using BaseString<value_type>::operator==;
	using BaseString<value_type>::operator!=;
	bool operator==(const String &x) const;
	bool operator==(const char *x) const;
	bool operator!=(const String &x) const;
	bool operator!=(const char *x) const;

	/** Python-like method **/
	String encode(CodePage page = kUtf8) const;

	/**
	 * Print formatted data into a U32String object.
	 */
	static U32String format(U32String fmt, ...);
	static U32String format(const char *fmt, ...);

	/**
	 * Print formatted data into a U32String object. It takes in the
	 * output by reference and works with iterators.
	 */
	static int vformat(const value_type *fmt, const value_type *fmtEnd, U32String &output, va_list args);

	/**
	 * Helper function for vformat, convert an int to string
	 * minimal implementation, only for base 10
	*/
	static char* itoa(int num, char* str, int base);

	using BaseString<value_type>::insertString;
	void insertString(const char *s, uint32 p);
	void insertString(const String &s, uint32 p);

	/** Return a substring of this string */
	U32String substr(size_t pos = 0, size_t len = npos) const;

	const uint32 *u32_str() const {
		return (const uint32 *) _str;
	}

private:
	void encodeUTF8(String &dst) const;
	void encodeOneByte(String &dst, CodePage page) const;
};

U32String operator+(const U32String &x, const U32String &y);
U32String operator+(const U32String &x, U32String::value_type y);

/** @} */

} // End of namespace Common

#endif
