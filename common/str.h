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

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/ustr.h"
#include "common/str-base.h"

#include <stdarg.h>

namespace Common {

/**
 * @defgroup common_str Strings
 * @ingroup common
 *
 * @brief API for working with strings.
 *
 * @{
 */

class U32String;

/**
 * Simple string class for ScummVM. Provides automatic storage management,
 * and overloads several operators in a 'natural' fashion, mimicking
 * the std::string class. Even provides simple iterators.
 *
 * This class tries to avoid allocating lots of small blocks on the heap,
 * since that is inefficient on several platforms supported by ScummVM.
 * Instead, small strings are stored 'inside' the string object (i.e. on
 * the stack, for stack allocated objects), and only for strings exceeding
 * a certain length do we allocate a buffer on the heap.
 *
 * The presence of \0 characters in the string will cause undefined
 * behavior in some operations.
 */
class String : public BaseString<char> {
public:
	/**
	 * Unsigned version of the underlying type. This can be used to cast
	 * individual string characters to bigger integer types without sign
	 * extension happening.
	 */
	typedef unsigned char unsigned_type;

	/** Construct a new empty string. */
	constexpr String() : BaseString<char>() {}

	/** Construct a new string from the given NULL-terminated C string. */
	String(const char *str) : BaseString<char>(str) {}

	/** Construct a new string containing exactly len characters read from address str. */
	String(const char *str, uint32 len) : BaseString<char>(str, len) {}

	/** Construct a new string containing the characters between beginP (including) and endP (excluding). */
	String(const char *beginP, const char *endP) : BaseString<char>(beginP, endP) {}

	/** Construct a copy of the given string. */
	String(const String &str) : BaseString<char>(str) {}

	/** Construct a string by moving an existing string. */
	String(String &&str) : BaseString<char>(static_cast<BaseString<char> &&>(str)) {}

	/** Construct a string consisting of the given character. */
	explicit constexpr String(value_type c) : BaseString<char>(c) {}

	/** Construct a new string from the given u32 string. */
	String(const U32String &str, CodePage page = kUtf8);

	String &operator=(const char *str);
	String &operator=(const String &str);
	String &operator=(String &&str);
	String &operator=(char c);
	String &operator+=(const char *str);
	String &operator+=(const String &str);
	String &operator+=(char c);

	bool equalsIgnoreCase(const String &x) const;
	int compareToIgnoreCase(const String &x) const; // stricmp clone

	bool equalsIgnoreCase(const char *x) const;
	int compareToIgnoreCase(const char *x) const;   // stricmp clone
	int compareDictionary(const String &x) const;
	int compareDictionary(const char *x) const;

	bool hasSuffix(const String &x) const;
	bool hasSuffix(const char *x) const;
	bool hasSuffixIgnoreCase(const String &x) const;
	bool hasSuffixIgnoreCase(const char *x) const;

	bool hasPrefix(const String &x) const;
	bool hasPrefix(const char *x) const;
	bool hasPrefixIgnoreCase(const String &x) const;
	bool hasPrefixIgnoreCase(const char *x) const;

	using BaseString<value_type>::contains;
	bool contains(const String &x) const;
	bool contains(const char *x) const;

	/**
	 * Simple DOS-style pattern matching function (understands * and ? like used in DOS).
	 * Taken from exult/files/listfiles.cc
	 *
	 * Token meaning:
	 *      "*": any character, any amount of times.
	 *      "?": any character, only once.
	 *      "#": any decimal digit, only once.
	 *      "\#": #, only once.
	 *
	 * Example strings/patterns:
	 *      String: monkey.s01   Pattern: monkey.s??    => true
	 *      String: monkey.s101  Pattern: monkey.s??    => false
	 *      String: monkey.s99   Pattern: monkey.s?1    => false
	 *      String: monkey.s101  Pattern: monkey.s*     => true
	 *      String: monkey.s99   Pattern: monkey.s*1    => false
	 *      String: monkey.s01   Pattern: monkey.s##    => true
	 *      String: monkey.s01   Pattern: monkey.###    => false
	 *
	 * @param pat Glob pattern.
	 * @param ignoreCase Whether to ignore the case when doing pattern match
	 * @param wildcardExclusions Characters which are excluded from wildcards and must be matched explicitly.
	 *
	 * @return true if str matches the pattern, false otherwise.
	 */
	bool matchString(const char *pat, bool ignoreCase = false, const char *wildcardExclusions = NULL) const;
	bool matchString(const String &pat, bool ignoreCase = false, const char *wildcardExclusions = NULL) const;

	/**
	 * Print formatted data into a String object. Similar to sprintf,
	 * except that it stores the result in (variably sized) String
	 * instead of a fixed size buffer.
	 */
	static String format(MSVC_PRINTF const char *fmt, ...) GCC_PRINTF(1, 2);

	/**
	 * Print formatted data into a String object. Similar to vsprintf,
	 * except that it stores the result in (variably sized) String
	 * instead of a fixed size buffer.
	 */
	static String vformat(const char *fmt, va_list args);

	/** Return a substring of this string */
	String substr(size_t pos = 0, size_t len = npos) const;

	/** Calls func on each line of the string, and returns a joined string */
	String forEachLine(String(*func)(const String, va_list args), ...) const;

	/** Python-like method **/
	U32String decode(CodePage page = kUtf8) const;

protected:
	StringEncodingResult encodeUTF8(const U32String &src, char errorChar);
	StringEncodingResult encodeWindows932(const U32String &src, char errorChar);
	StringEncodingResult encodeWindows936(const U32String &src, char errorChar);
	StringEncodingResult encodeWindows949(const U32String &src, char errorChar);
	StringEncodingResult encodeWindows950(const U32String &src, bool translit, char errorChar);
	StringEncodingResult encodeJohab(const U32String &src, char errorChar);
	StringEncodingResult encodeOneByte(const U32String &src, CodePage page, bool translit, char errorChar);
	StringEncodingResult encodeInternal(const U32String &src, CodePage page, char errorChar);
	StringEncodingResult translitChar(U32String::value_type point, char errorChar);

	friend class U32String;
};

// Append two strings to form a new (temp) string
String operator+(const String &x, const String &y);

String operator+(const char *x, const String &y);
String operator+(const String &x, const char *y);

String operator+(const String &x, char y);
String operator+(char x, const String &y);

// Some useful additional comparison operators for Strings
bool operator==(const char *x, const String &y);
bool operator!=(const char *x, const String &y);

// Utility functions to remove leading and trailing whitespaces
extern char *ltrim(char *t);
extern char *rtrim(char *t);
extern char *trim(char *t);


/**
 * Returns the last component of a given path.
 *
 * Examples:
 *          /foo/bar.txt    would return 'bar.txt'
 *          /foo/bar/       would return 'bar'
 *          /foo/./bar//    would return 'bar'
 *
 * @param path the path of which we want to know the last component
 * @param sep character used to separate path components
 * @return The last component of the path.
 */
String lastPathComponent(const String &path, const char sep);

/**
 * Returns the first components of a given path (complementary to lastPathComponent)
 *
 * Examples:
 *          /foo/bar.txt    would return '/foo/'
 *          /foo/bar/       would return '/foo/'
 *          /foo/./bar//    would return '/foo/./'
 *
 * @param path the path of which we want to know the last component
 * @param sep character used to separate path components
 * @return The all the components of the path except the last one.
 */
String firstPathComponents(const String &path, const char sep);

/**
 * Normalize a given path to a canonical form. In particular:
 * - trailing separators are removed:  /foo/bar/ -> /foo/bar
 * - double separators (= empty components) are removed:   /foo//bar -> /foo/bar
 * - dot components are removed:  /foo/./bar -> /foo/bar
 *
 * @todo remove double dot components:  /foo/baz/../bar -> /foo/bar
 *
 * @param path  the path to normalize
 * @param sep   the separator token (usually '/' on Unix-style systems, or '\\' on Windows based stuff)
 * @return      the normalized path
 */
String normalizePath(const String &path, const char sep);


/**
 * Simple DOS-style pattern matching function (understands * and ? like used in DOS).
 * Taken from exult/files/listfiles.cc
 *
 * Token meaning:
 *      "*": any character, any amount of times.
 *      "?": any character, only once.
 *      "#": any decimal digit, only once.
 *
 * Example strings/patterns:
 *      String: monkey.s01   Pattern: monkey.s??    => true
 *      String: monkey.s101  Pattern: monkey.s??    => false
 *      String: monkey.s99   Pattern: monkey.s?1    => false
 *      String: monkey.s101  Pattern: monkey.s*     => true
 *      String: monkey.s99   Pattern: monkey.s*1    => false
 *      String: monkey.s01   Pattern: monkey.s##    => true
 *      String: monkey.s01   Pattern: monkey.###    => false
 *
 * @param str Text to be matched against the given pattern.
 * @param pat Glob pattern.
 * @param ignoreCase Whether to ignore the case when doing pattern match
 * @param wildcardExclusions Characters which are excluded from wildcards and must be matched explicitly.
 *
 * @return true if str matches the pattern, false otherwise.
 */
bool matchString(const char *str, const char *pat, bool ignoreCase = false, const char *wildcardExclusions = NULL);

/**
 * Function which replaces substring with the other. It happens in place.
 * If there is no substring found, original string is not changed.
 *
 * @param source String to search and replace substring in.
 * @param what Substring to replace.
 * @param with String to replace with.
 */
void replace(Common::String &source, const Common::String &what, const Common::String &with);

/**
 * Take a 32 bit value and turn it into a four character string, where each of
 * the four bytes is turned into one character. Most significant byte is printed
 * first.
 *
 * @param tag tag value to convert
 * @param nonPrintable indicate if non-printable characters need to be printed as octals
 */
String tag2string(uint32 tag, bool nonPrintable = false);

/**
 * Copy up to size - 1 characters from src to dst and also zero terminate the
 * result. Note that src must be a zero terminated string.
 *
 * @note This is modeled after strcpy_s from C11 but simplified by using warning
 * instead of erroring out
 *
 * @param dst The destination buffer.
 * @param size The size of the destination buffer.
 * @param src The source string.
 */
void strcpy_s(char *dst, size_t size, const char *src);

/**
 * Copy up to N - 1 characters from src to dst and also zero terminate the
 * result. Note that src must be a zero terminated string.
 *
 * @note This is modeled after strcpy_s from C11 but simplified by using warning
 * instead of erroring out
 *
 * @param dst The destination buffer as a reference to a constant size array.
 * @param src The source string.
 */
template<typename T, size_t N>
FORCEINLINE void strcpy_s(T (&dst)[N], const char *src) {
	STATIC_ASSERT(sizeof(T) == sizeof(char), T_is_not_compatible_with_char);
	strcpy_s((char *)dst, N, src);
}

/**
 * Append the string src to the string dst. Note that both src and dst must be
 * zero terminated. The result will be zero terminated. At most
 * "size - strlen(dst) - 1" bytes will be appended.
 *
 * @note This is modeled after strcpy_s from C11 but simplified by using warning
 * instead of erroring out
 *
 * @param dst The string the source string should be appended to.
 * @param size The (total) size of the destination buffer.
 * @param src The source string.
 */
void strcat_s(char *dst, size_t size, const char *src);

/**
 * Append the string src to the string dst. Note that both src and dst must be
 * zero terminated. The result will be zero terminated. At most
 * "N - strlen(dst) - 1" bytes will be appended.
 *
 * @note This is modeled after strcat_s from C11 but simplified by using warning
 * instead of erroring out
 *
 * @param dst The string the source string should be appended to as a reference to a constant size array.
 * @param src The source string.
 */
template<typename T, size_t N>
FORCEINLINE void strcat_s(T (&dst)[N], const char *src) {
	STATIC_ASSERT(sizeof(T) == sizeof(char), T_is_not_compatible_with_char);
	strcat_s((char *)dst, N, src);
}

/**
 * A sprintf shim which warns when the buffer overruns and null terminates in this case
 *
 * @param dst Where the resulting string will be stored.
 * @param size The (total) size of the destination buffer.
 * @param format The format string.
 */
int vsprintf_s(char *dst, size_t size, const char *format, va_list ap) GCC_PRINTF(3, 0);

/**
 * A sprintf shim which warns when the buffer overruns and null terminates in this case
 * The size of the buffer is automatically determined.
 *
 * @param dst Where the resulting string will be stored.
 * @param format The format string.
 */
template<typename T, size_t N>
FORCEINLINE GCC_PRINTF(2, 0) int vsprintf_s(T (&dst)[N], const char *format, va_list ap) {
	STATIC_ASSERT(sizeof(T) == sizeof(char), T_is_not_compatible_with_char);
	return vsprintf_s((char *)dst, N, format, ap);
}

/**
 * A sprintf shim which warns when the buffer overruns and null terminates in this case
 *
 * @param dst Where the resulting string will be stored.
 * @param size The (total) size of the destination buffer.
 * @param format The format string.
 */
int sprintf_s(char *dst, size_t size, MSVC_PRINTF const char *format, ...) GCC_PRINTF(3, 4);

/**
 * A sprintf shim which warns when the buffer overruns and null terminates in this case
 * The size of the buffer is automatically determined.
 *
 * @param dst Where the resulting string will be stored.
 * @param format The format string.
 */
template<typename T, size_t N>
inline GCC_PRINTF(2, 3) int sprintf_s(T (&dst)[N], MSVC_PRINTF const char *format, ...) {
	STATIC_ASSERT(sizeof(T) == sizeof(char), T_is_not_compatible_with_char);
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vsprintf_s((char *)dst, N, format, ap);
	va_end(ap);
	return ret;
}

/**
 * Copy up to size - 1 characters from src to dst and also zero terminate the
 * result. Note that src must be a zero terminated string.
 *
 * In case size is zero this function just returns the length of the source
 * string.
 *
 * @note This is modeled after OpenBSD's strlcpy. See the manpage here:
 *       https://man.openbsd.org/strlcpy
 *
 * @param dst The destination buffer.
 * @param src The source string.
 * @param size The size of the destination buffer.
 * @return The length of the (non-truncated) result, i.e. strlen(src).
 */
size_t strlcpy(char *dst, const char *src, size_t size);

/**
 * Append the string src to the string dst. Note that both src and dst must be
 * zero terminated. The result will be zero terminated. At most
 * "size - strlen(dst) - 1" bytes will be appended.
 *
 * In case the dst string does not contain a zero within the first "size" bytes
 * the dst string will not be changed and size + strlen(src) is returned.
 *
 * @note This is modeled after OpenBSD's strlcat. See the manpage here:
 *       https://man.openbsd.org/strlcat
 *
 * @param dst The string the source string should be appended to.
 * @param src The source string.
 * @param size The (total) size of the destination buffer.
 * @return The length of the (non-truncated) result. That is
 *         strlen(dst) + strlen(src). In case strlen(dst) > size
 *         size + strlen(src) is returned.
 */
size_t strlcat(char *dst, const char *src, size_t size);

/**
 * Determine the length of a string up to a maximum of `maxSize` characters.
 * This should be used instead of `strlen` when reading the length of a C string
 * from potentially unsafe or corrupt sources, like game assets.
 *
 * @param src The source string.
 * @param maxSize The maximum size of the string.
 * @return The length of the string.
 */
size_t strnlen(const char *src, size_t maxSize);

/**
 * Convenience wrapper for tag2string which "returns" a C string.
 * Note: It is *NOT* safe to do anything with the return value other than directly
 * copying or printing it.
 */
#define tag2str(x)	Common::tag2string(x).c_str()

/**
 * Convenience wrapper for tag2string with non-printable characters which "returns" a C string.
 * Note: It is *NOT* safe to do anything with the return value other than directly
 * copying or printing it.
 */
#define tag2strP(x)	Common::tag2string(x, true).c_str()

/**
 * Converts string with all non-printable characters properly escaped
 * with use of C++ escape sequences
 *
 * @param src The source string.
 * @param keepNewLines Whether keep newlines or convert them to '\n', default: true.
 * @return The converted string.
 */
String toPrintable(const String &src, bool keepNewLines = true);

/**
 * Converts string with special URL characters to URL encoded (percent encoded) strings
 */
String percentEncodeString(const String &src);

/** @} */

} // End of namespace Common

extern int scumm_stricmp(const char *s1, const char *s2);
extern int scumm_strnicmp(const char *s1, const char *s2, uint n);
extern char *scumm_strdup(const char *in);

extern int scumm_compareDictionary(const char *s1, const char *s2);
extern const char *scumm_skipArticle(const char *s1);

extern const char *scumm_strcasestr(const char *s, const char *find);

#endif
