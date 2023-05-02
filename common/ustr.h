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

#ifndef COMMON_USTR_H
#define COMMON_USTR_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/str-enc.h"
#include "common/str-base.h"

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
typedef char32_t u32char_type_t;

class U32String : public BaseString<u32char_type_t> {
public:
	typedef uint32 unsigned_type; /*!< Unsigned version of the underlying type. */
public:
	/** Construct a new empty string. */
	constexpr U32String() : BaseString<u32char_type_t>() {}

	/** Construct a new string from the given null-terminated C string. */
	explicit U32String(const value_type *str) : BaseString<u32char_type_t>(str) {}

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	U32String(const value_type *str, uint32 len) : BaseString<u32char_type_t>(str, len) {}

	explicit U32String(const uint32 *str) : BaseString<u32char_type_t>((const value_type *) str) {}
	U32String(const uint32 *str, uint32 len) : BaseString<u32char_type_t>((const value_type *) str, len) {}
	U32String(const uint32 *beginP, const uint32 *endP) : BaseString<u32char_type_t>((const value_type *) beginP, (const value_type *) endP) {}

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	U32String(const value_type *beginP, const value_type *endP) : BaseString<u32char_type_t>(beginP, endP) {}

	/** Construct a copy of the given string. */
	U32String(const U32String &str) : BaseString<u32char_type_t>(str) {}

	/** Construct a string by moving an existing string. */
	U32String(U32String &&str) : BaseString<u32char_type_t>(static_cast<BaseString<u32char_type_t> &&>(str)) {}

	/** Construct a new string from the given null-terminated C string that uses the given @p page encoding. */
	explicit U32String(const char *str, CodePage page = kUtf8);

	/** Construct a new string containing exactly @p len characters read from address @p str. */
	U32String(const char *str, uint32 len, CodePage page = kUtf8);

	/** Construct a new string containing the characters between @p beginP (including) and @p endP (excluding). */
	U32String(const char *beginP, const char *endP, CodePage page = kUtf8);

	/** Construct a copy of the given string. */
	U32String(const String &str, CodePage page = kUtf8);

	/** Construct a string consisting of the given character. */
	explicit U32String(value_type c);

	/** Assign a given string to this string. */
	U32String &operator=(const U32String &str);

	/** Move a given string to this string. */
	U32String &operator=(U32String &&str);

	/** @overload */
	U32String &operator=(const String &str);

	/** @overload */
	U32String &operator=(const value_type *str);

	/** @overload */
	U32String &operator=(const char *str);

	/** @overload */
	U32String &operator=(value_type c);

	/** Append the given string to this string. */
	U32String &operator+=(const U32String &str);

	/** @overload */
	U32String &operator+=(const value_type *str);

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

	/** Convert the string to the given @p page encoding and output in string @p outString,
		replacing invalid characters with @p errorChar. */
	StringEncodingResult encode(String &outString, CodePage page, char errorChar) const;

	/**
	 * Print formatted data into a U32String object.
	 *
	 * Similar to sprintf, except that it stores the result
	 * in a (variably sized) string instead of a fixed-size buffer.
	 */
	template<class... TParam>
	static U32String format(const U32String &fmt, TParam... param);

	/** @overload **/
	static U32String format(const char *fmt, ...);

	/**
	 * Print formatted data into a U32String object.
	 * The method takes in the output by reference and works with iterators.
	 */
	static int vformat(const value_type *fmt, const value_type *fmtEnd, U32String &output, va_list args);

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

	/** Transform Traditional Chinese string into Simplified. */
	U32String transcodeChineseT2S() const;

private:
	static U32String formatInternal(const U32String *fmt, ...);

	/**
	 * Helper function for vformat. Convert an int to a string.
	 * Minimal implementation, only for base 10.
	 */
	static value_type* ustr_helper_itoa(int num, value_type* str, uint base);

	/**
	 * Helper function for vformat. Convert an unsigned int to a string.
	 * Minimal implementation, only for base 10.
	 */
	static value_type* ustr_helper_uitoa(uint num, value_type* str, uint base);

	void decodeInternal(const char *str, uint32 len, CodePage page);
	void decodeOneByte(const char *str, uint32 len, CodePage page);
	void decodeWindows932(const char *src, uint32 len);
	void decodeWindows936(const char *src, uint32 len);
	void decodeWindows949(const char *src, uint32 len);
	void decodeWindows950(const char *src, uint32 len);
	void decodeJohab(const char *src, uint32 len);
	void decodeUTF8(const char *str, uint32 len);

	friend class String;
};

template<class... TParam>
inline U32String U32String::format(const U32String &fmt, TParam... param) {
	return formatInternal(&fmt, Common::forward<TParam>(param)...);
}

/** Concatenate strings @p x and @p y. */
U32String operator+(const U32String &x, const U32String &y);

/** Append the given @p y character to the given @p x string. */
U32String operator+(const U32String &x, U32String::value_type y);

/**
 * Converts string with all non-printable characters properly escaped
 * with use of C++ escape sequences.
 * Unlike the String version, this does not escape characters with
 * codepoints > 127.
 *
 * @param src The source string.
 * @param keepNewLines Whether keep newlines or convert them to '\n', default: true.
 * @return The converted string.
 */
U32String toPrintable(const U32String &src, bool keepNewLines = true);

/** @} */

} // End of namespace Common

#endif
