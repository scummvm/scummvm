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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"

/**
 * @defgroup common_util Util
 * @ingroup common
 *
 * @brief Various utility functions.
 *
 * @{
 */

/**
 * Check whether a given pointer is aligned correctly.
 * Note that 'alignment' must be a power of two!
 */
#define IS_ALIGNED(value, alignment) \
          ((((size_t)value) & ((alignment) - 1)) == 0)

#ifdef ABS
#undef ABS
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

/** Template method to return the absolute value of @p x. */
template<typename T> inline T ABS(T x)		{ return (x >= 0) ? x : -x; }

/** Template method to return the smallest of its parameters. */
template<typename T> inline T MIN(T a, T b)	{ return (a < b) ? a : b; }

/** Template method to return the largest of its parameters. */
template<typename T> inline T MAX(T a, T b)	{ return (a > b) ? a : b; }

/** Template method to clip the value @p v so that it remains between @p amin and @p amax. */
template<typename T> inline T CLIP(T v, T amin, T amax)
	{
#if !defined(RELEASE_BUILD)
		// Debug builds use this assert to pinpoint
		// any problematic cases, where amin and amax
		// are incorrectly ordered
		// and thus CLIP() would return an invalid result.
		assert(amin <= amax);
#endif
		if (v < amin) return amin;
		else if (v > amax) return amax;
		return v;
	}

/**
 * Template method to swap the values of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

/**
 * Determine the number of entries in a fixed size array.
 */
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

/**
 * Compute a pointer to one past the last element of an array.
 */
#define ARRAYEND(x) ((x) + ARRAYSIZE((x)))

/**
 * Clear an array using the default or provided value.
 */
template<typename T, size_t N> inline void ARRAYCLEAR(T (&array) [N], const T &value = T()) {
	T * ptr = array;
	size_t n = N;
	while(n--)
		*ptr++ = value;
}

/**
 * Evaluate the name of the current function on compilers supporting this.
 */
#if defined(__GNUC__)
# define SCUMMVM_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#  define SCUMMVM_CURRENT_FUNCTION	__func__
#elif defined(_MSC_VER)
#  define SCUMMVM_CURRENT_FUNCTION __FUNCTION__
#else
#  define SCUMMVM_CURRENT_FUNCTION "<unknown>"
#endif

/** @} */

namespace Common {

/**
 * @addtogroup common_util
 * @{
 */

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 *
 * @param data          The data to be dumped.
 * @param len           Length of that data.
 * @param bytesPerLine  Number of bytes to print per line (default: 16).
 * @param startOffset   Shift the shown offsets by the starting offset (default: 0).
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16, int startOffset = 0);


/**
 * Parse a string for a boolean value.
 *
 * The strings "true", "yes", and "1" are interpreted as true.
 * The strings "false", "no", and "0" are interpreted as false.
 * This function ignores case.
 *
 * @param[in]  val        The string to parse.
 * @param[out] valAsBool  Parsing result.
 *
 * @return True if the string has been parsed correctly, false if an error occurred.
 */
bool parseBool(const String &val, bool &valAsBool);


/**
 * Test whether the given character is alphanumeric (a-z, A-Z, 0-9).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is alphanumeric, false otherwise.
 */
bool isAlnum(int c);

/**
 * Test whether the given character is an alphabetic letter (a-z, A-Z).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is alphabetic, false otherwise.
 */
bool isAlpha(int c);

/**
 * Test whether the given character is a decimal digit (0-9).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a decimal digit, false otherwise.
 */
bool isDigit(int c);

/**
 * Test whether the given character is a hexadecimal digit (0-9 or A-F).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a hexadecimal digit, false otherwise.
 */
bool isXDigit(int c);

/**
 * Test whether the given character is a lowercase letter (a-z).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a lowercase letter, false otherwise.
 */
bool isLower(int c);

/**
 * Test whether the given character is a whitespace.
 *
 * The following characters are considered a whitespace:
 * @code
 * ' ', '\t', '\r', '\n', '\v', '\f'
 * @endcode
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a whitespace, false otherwise.
 */
bool isSpace(int c);

/**
 * Test whether the given character is an uppercase letter (A-Z).
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is an uppercase letter, false otherwise.
 */
bool isUpper(int c);

/**
 * Test whether the given character is printable. This includes the space
 * character (' ').
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		The character to test.
 *
 * @return True if the character is printable, false otherwise.
 */
bool isPrint(int c);

/**
 * Test whether the given character is a punctuation character,
 * (i.e. not alphanumeric).
 *
 * @param c		The character to test.
 *
 * @return True if the character is punctuation, false otherwise.
 */
bool isPunct(int c);

/**
 * Test whether the given character is a control character.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a control character, false otherwise.
 */
bool isCntrl(int c);

/**
 * Test whether the given character has a graphical representation.
 *
 * @param c		The character to test.
 *
 * @return True if the character is a graphic, false otherwise.
 */
bool isGraph(int c);


/**
 * Represent the size of a file in bytes as a number with floating point and
 * largest suitable units. For example, 1474560 bytes as 1.4 MB.
 * 
 * @param[in]  bytes     Size in bytes to be represented.
 * @param[out] unitsOut  String with units.
 *
 * @note Use @c _() to translate units correctly.
 *
 * @return String with a floating point number representing the given size.
 */
Common::String getHumanReadableBytes(uint64 bytes, Common::String &unitsOut);

/** @} */

} // End of namespace Common

#endif
