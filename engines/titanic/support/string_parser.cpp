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

#include "titanic/support/string_parser.h"
#include "common/util.h"

namespace Titanic {

void CStringParser::skipSeperators(const CString &seperatorChars) {
	for (; _index < size(); ++_index) {
		char c = (*this)[_index];
		if (seperatorChars.indexOf(c) == -1)
			break;
	}
}

bool CStringParser::parse(CString &resultStr, const CString &seperatorChars, bool allowQuotes) {
	if (_index >= size())
		return false;

	resultStr.clear();
	bool quoteFlag = false;
	while (_index < size()) {
		char c = (*this)[_index];
		if (!quoteFlag && seperatorChars.indexOf(c) >= 0)
			break;

		if (allowQuotes) {
			if (quoteFlag) {
				if (c == '"') {
					// End of quoted string
					++_index;
					break;
				}
			} else {
				if (c == '"') {
					// Start of quoted string
					++_index;
					quoteFlag = true;
					continue;
				}
			}
		}

		resultStr += c;
		++_index;
	}

	return true;
}

uint CStringParser::readInt() {
	// Get digits from the string
	CString numStr;
	while (Common::isDigit(currentChar()))
		numStr += getNextChar();

	// Throw a wobbly if there wasn't a number
	if (numStr.empty())
		error("ReadInt(): No number to read");

	return atoi(numStr.c_str());
}

char CStringParser::currentChar() const {
	return (_index >= size()) ? '\0' : (*this)[_index];
}

char CStringParser::getNextChar() {
	return (_index >= size()) ? '\0' : (*this)[_index++];
}

void CStringParser::skipSpaces() {
	while (_index < size() && Common::isSpace(currentChar()))
		++_index;
}

} // End of namespace Titanic
