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

template<typename T> inline T ABS(T x)		{ return (x >= 0) ? x : -x; }
template<typename T> inline T MIN(T a, T b)	{ return (a < b) ? a : b; }
template<typename T> inline T MAX(T a, T b)	{ return (a > b) ? a : b; }
template<typename T> inline T CLIP(T v, T amin, T amax)
		{ if (v < amin) return amin; else if (v > amax) return amax; else return v; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

/**
 * Macro which determines the number of entries in a fixed size array.
 */
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

/**
 * Compute a pointer to one past the last element of an array.
 */
#define ARRAYEND(x) ((x) + ARRAYSIZE((x)))


/**
 * @def SCUMMVM_CURRENT_FUNCTION
 * This macro evaluates to the current function's name on compilers supporting this.
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

namespace Common {

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the length of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 * @param startOffset	shift the shown offsets by the starting offset (default: 0)
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16, int startOffset = 0);


/**
 * Parse a string for a boolean value.
 * The strings "true", "yes", and "1" are interpreted as true.
 * The strings "false", "no", and "0" are interpreted as false.
 * This function ignores case.
 *
 * @param[in] val			the string to parse
 * @param[out] valAsBool	the parsing result
 * @return 	true if the string parsed correctly, false if an error occurred.
 */
bool parseBool(const String &val, bool &valAsBool);


/**
 * Test whether the given character is alphanumeric (a-z, A-Z, 0-9).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is alphanumeric, false otherwise.
 */
bool isAlnum(int c);

/**
 * Test whether the given character is an alphabetic letter (a-z, A-Z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is alphabetic, false otherwise.
 */
bool isAlpha(int c);

/**
 * Test whether the given character is a decimal-digit (0-9).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a decimal-digit, false otherwise.
 */
bool isDigit(int c);

/**
 * Test whether the given character is a hwzadecimal-digit (0-9 or A-F).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a hexadecimal-digit, false otherwise.
 */
bool isXDigit(int c);

/**
 * Test whether the given character is a lower-case letter (a-z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a lower-case letter, false otherwise.
 */
bool isLower(int c);

/**
 * Test whether the given character is a white-space.
 * White-space characters are ' ', '\t', '\r', '\n', '\v', '\f'.
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a white-space, false otherwise.
 */
bool isSpace(int c);

/**
 * Test whether the given character is an upper-case letter (A-Z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is an upper-case letter, false otherwise.
 */
bool isUpper(int c);

/**
 * Test whether the given character is printable. This includes the space
 * character (' ').
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is printable, false otherwise.
 */
bool isPrint(int c);

/**
 * Test whether the given character is a punctuation character,
 * (i.e. not alphanumeric).
 *
 * @param c		the character to test
 * @return		true if the character is punctuation, false otherwise.
 */
bool isPunct(int c);

/**
 * Test whether the given character is a control character.
 *
 * @param c		the character to test
 * @return		true if the character is a control character, false otherwise.
 */
bool isCntrl(int c);

/**
 * Test whether the given character has a graphical representation.
 *
 * @param c		the character to test
 * @return		true if the character is a graphic, false otherwise.
 */
bool isGraph(int c);


/**
 * Represent bytes size of a file as a number with floating point and
 * largest suitable units. For example, 1474560 bytes as 1.4 MB.
 * 
 * @param bytes		size in bytes to be represented
 * @param unitsOut	(out-parameter) string with units
 * @note			use _() to translate units correctly
 * @return			string with a floating point number representing given size
 */
Common::String getHumanReadableBytes(uint64 bytes, Common::String &unitsOut);

} // End of namespace Common

#endif
