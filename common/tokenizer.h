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

#ifndef COMMON_TOKENIZER_H
#define COMMON_TOKENIZER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/ustr.h"

namespace Common {

/**
 * @defgroup common_tokenizer String tokenizer
 * @ingroup common
 *
 * @brief String tokenizer for creating tokens out of parts of a string.
 *
 * @{
 */

/**
 * A simple non-optimized string tokenizer.
 *
 * Example of use:
 * StringTokenizer("Now, this is a test!", " ,!") gives tokens "Now", "this", "is", "a" and "test" using nextToken().
 */
class StringTokenizer {
public:
	/**
	 * Creates a StringTokenizer.
	 * @param str The string to be tokenized.
	 * @param delimiters String containing all the delimiter characters (i.e. the characters to be ignored).
	 * @note Uses space, horizontal tab, carriage return, newline, form feed and vertical tab as delimiters by default.
	 */
	StringTokenizer(const String &str, const String &delimiters = " \t\r\n\f\v");
	void reset();       ///< Resets the tokenizer to its initial state
	bool empty() const; ///< Returns true if there are no more tokens left in the string, false otherwise
	String nextToken(); ///< Returns the next token from the string (Or an empty string if there are no more tokens)

private:
	const String _str;        ///< The string to be tokenized
	const String _delimiters; ///< String containing all the delimiter characters
	uint         _tokenBegin; ///< Latest found token's begin (Valid after a call to nextToken(), zero otherwise)
	uint         _tokenEnd;   ///< Latest found token's end (Valid after a call to nextToken(), zero otherwise)
};

/**
 * A simple non-optimized unicode-string tokenizer.
 *
 * Example of use:
 * U32StringTokenizer("Now, this is a test!", " ,!") gives tokens "Now", "this", "is", "a" and "test" using nextToken().
 * Using non-ascii chars will also work, and is recommended to use this over StringTokenizer if string contains unicode chars.
 */
class U32StringTokenizer {
public:
	/**
	 * Creates a UnicodeStringTokenizer.
	 * @param str The unicode string to be tokenized.
	 * @param delimiters String containing all the delimiter characters (i.e. the characters to be ignored).
	 * @note Uses space, horizontal tab, carriage return, newline, form feed and vertical tab as delimiters by default.
	 */
	U32StringTokenizer(const U32String &str, const String &delimiters = " \t\r\n\f\v");
	void reset();       ///< Resets the tokenizer to its initial state, i.e points boten token iterators to the beginning
	bool empty() const; ///< Returns true if there are no more tokens left in the string, false otherwise
	U32String nextToken(); ///< Returns the next token from the string (Or an empty string if there are no more tokens)

private:
	const U32String _str;        ///< The unicode string to be tokenized
	const String    _delimiters; ///< String containing all the delimiter characters
	U32String::const_iterator            _tokenBegin; ///< Latest found token's begin iterator (Valid after a call to nextToken())
	U32String::const_iterator            _tokenEnd;   ///< Latest found token's end iterator (Valid after a call to nextToken())
};

/** @} */

} // End of namespace Common

#endif
