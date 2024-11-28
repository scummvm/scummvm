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

#ifndef COMMON_TOKENIZER_H
#define COMMON_TOKENIZER_H

#include "common/scummsys.h"
#include "common/str-array.h"

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
template<class T>
class BaseStringTokenizer {
public:
	/**
	 * Creates a BaseStringTokenizer.
	 * @param str The string to be tokenized.
	 * @param delimiters String containing all the delimiter characters (i.e. the characters to be ignored).
	 * @note Uses space, horizontal tab, carriage return, newline, form feed and vertical tab as delimiters by default.
	 */
	BaseStringTokenizer(const T &str, const String &delimiters = " \t\r\n\f\v");
	void reset();       ///< Resets the tokenizer to its initial state, i.e points boten token iterators to the beginning
	bool empty() const; ///< Returns true if there are no more tokens left in the string, false otherwise
	T nextToken(); ///< Returns the next token from the string (Or an empty string if there are no more tokens)
	Array<T> split(); ///< Returns an Array with all tokens. Beware of the memory usage

	T delimitersAtTokenBegin() const; ///< Returns a String with all delimiters between the current and previous token
	T delimitersAtTokenEnd() const;   ///< Returns a String with all delimiters between the current and next token

private:
	const T           _str;        ///< The unicode string to be tokenized
	const String      _delimiters; ///< String containing all the delimiter characters
	typename T::const_iterator _tokenBegin; ///< Latest found token's begin iterator (Valid after a call to nextToken())
	typename T::const_iterator _tokenEnd;   ///< Latest found token's end iterator (Valid after a call to nextToken())
};

typedef BaseStringTokenizer<String> StringTokenizer;
typedef BaseStringTokenizer<U32String> U32StringTokenizer;

/** @} */

} // End of namespace Common

#endif
