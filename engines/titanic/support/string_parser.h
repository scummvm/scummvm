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

#ifndef TITANIC_STRING_PARSER_H
#define TITANIC_STRING_PARSER_H

#include "titanic/support/string.h"

namespace Titanic {

class CStringParser : public CString {
private:
	uint _index;
private:
	/**
	 * Gets the character at the current index
	 */
	char currentChar() const;

	/**
	 * Gets the next character, and increments the parsing index
	 */
	char getNextChar();

	/**
	 * Skips over any spaces
	 */
	void skipSpaces();
public:
	CStringParser() : CString(), _index(0) {}
	CStringParser(const CString &str) : CString(str), _index(0) {}

	/**
	 * Skips over any specified seperator characters in our string
	 * at the current index
	 */
	void skipSeperators(const CString &seperatorChars);

	/**
	 * Parses out a string from a source string at the current index
	 * @param resultStr			String to hold the resulting sring
	 * @param seperatorChras	List of characters that seperate string values
	 * @param allowQuotes		If true, handles double-quoted substrings
	 * @returns		True if a string entry was extracted
	 */
	bool parse(CString &resultStr, const CString &seperatorChars, bool allowQuotes = false);

	/**
	 * Reads an integer from the string
	 */
	uint readInt();

};

} // End of namespace Titanic

#endif /* TITANIC_STRING_PARSER_H */
