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

#include "titanic/true_talk/tt_parser.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_input.h"

namespace Titanic {

int TTparser::processInput(TTinput *input) {
	_input = input;
	if (normalize(input))
		return 0;

	warning("TODO: TTparser::processInput");
	return 0;
}

int TTparser::normalize(TTinput *input) {
	TTstring *destLine = new TTstring();
	const TTstring &srcLine = input->_initialLine;
	int srcSize = srcLine.size();
	int savedIndex = 0;
	int counter1 = 0;
	int commandVal;

	for (int index = 0; index < srcSize; ++index) {
		char c = srcLine[index];
		if (Common::isLower(c)) {
			(*destLine) += c;
		} else if (Common::isSpace(c)) {
			if (!destLine->empty() && destLine->lastChar() != ' ')
				(*destLine) += ' ';
		} else if (Common::isUpper(c)) {
			(*destLine) += toupper(c);
		} else if (Common::isDigit(c)) {
			if (c == '0' && isSpecialCommand(srcLine, index)) {
				input->set38(10);
			} else {
				// Iterate through all the digits of the number
				(*destLine) += c;
				while (Common::isDigit(srcLine[index + 1]))
					(*destLine) += srcLine[++index];
			}
		} else if (Common::isPunct(c)) {
			bool flag = false;
			switch (c) {
			case '!':
				input->set38(3);
				break;
			
			case '\'':
				if (!normalizeContraction(srcLine, index, *destLine))
					flag = true;
				break;
			
			case '.':
				input->set38(1);
				break;
			
			case ':':
				commandVal = isSpecialCommand(srcLine, index);
				if (commandVal) {
					input->set38(commandVal);
					index += 2;
				} else {
					flag = true;
				}
				break;
			
			case ';':
				commandVal = isSpecialCommand(srcLine, index);
				if (commandVal == 6) {
					input->set38(7);
					index += 2;
				} else if (commandVal != 0) {
					input->set38(commandVal);
					index += 2;
				}
				break;
			
			case '<':
				++index;
				commandVal = isSpecialCommand(srcLine, index);
				if (commandVal == 6) {
					input->set38(12);
				} else {
					--index;
					flag = true;
				}
				break;

			case '>':
				++index;
				commandVal = isSpecialCommand(srcLine, index);
				if (commandVal == 6 || commandVal == 9) {
					input->set38(11);
				} else {
					--index;
					flag = true;
				}
				break;

			case '?':
				input->set38(2);
				break;

			default:
				flag = true;
				break;
			}

			if (flag && (!savedIndex || (index - savedIndex) == 1))
				++counter1;

			savedIndex = index;
		}
	}

	if (counter1 >= 4)
		input->set38(4);

	// Remove any trailing spaces
	while (destLine->hasSuffix(" "))
		destLine->deleteLastChar();

	// Copy out the normalized line
	input->_normalizedLine = *destLine;
	delete destLine;

	return 0;
}

int TTparser::isSpecialCommand(const TTstring &str, int &index) {
	if (str[index] != ':' && str[index] != ';')
		return 0;

	if (str[index + 1] != '-')
		return 0;

	index += 2;
	switch (str[index]) {
	case '(':
	case '<':
		return 8;

	case ')':
	case '>':
		return 6;

	case 'P':
	case 'p':
		return 9;

	default:
		return 5;
	}
}

bool TTparser::normalizeContraction(const TTstring &srcLine, int srcIndex, TTstring &destLine) {
	int startIndex = srcIndex + 1;
	switch (srcLine[startIndex]) {
	case 'd':
		srcIndex += 2;
		if (srcLine.compareAt(srcIndex, " a ") || srcLine.compareAt(srcIndex, " the ")) {
			destLine += " had";
		} else {
			destLine += " would";
		}

		srcIndex = startIndex;
		break;

	case 'l':
		if (srcLine[srcIndex + 2] == 'l') {
			// 'll ending
			destLine += " will";
			srcIndex = startIndex;
		}
		break;

	case 'm':
		// 'm ending
		destLine += " am";
		srcIndex = startIndex;
		break;

	case 'r':
		// 're ending
		if (srcLine[srcIndex + 2] == 'e') {
			destLine += " are";
			srcIndex = startIndex;
		}
		break;

	case 's':
		destLine += "s*";
		srcIndex = startIndex;
		break;

	case 't':
		if (srcLine[srcIndex - 1] == 'n' && srcIndex >= 3) {
			if (srcLine[srcIndex - 3] == 'c' && srcLine[srcIndex - 2] == 'a' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// can't -> can not
				destLine += 'n';
			} else if (srcLine[srcIndex - 3] == 'w' && srcLine[srcIndex - 2] == 'o' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// won't -> will not
				destLine.deleteLastChar();
				destLine.deleteLastChar();
				destLine += "ill";
			} else if (srcLine[srcIndex - 3] == 'a' && srcLine[srcIndex - 2] == 'i' &&
				(srcIndex == 3 || srcLine[srcIndex - 4])) {
				// ain't -> am not
				destLine.deleteLastChar();
				destLine.deleteLastChar();
				destLine += "m";
			} else if (srcLine.hasSuffix(" sha") || 
					(srcIndex == 4 && srcLine.hasSuffix("sha"))) {
				// shan't -> shall not
				destLine.deleteLastChar();
				destLine += "ll";
			}

			destLine += " not";
		}
		break;

	case 'v':
		// 've ending
		if (srcLine[startIndex + 2] == 'e') {
			destLine += " have";
			srcIndex = startIndex;
		}
		break;

	default:
		break;
	}

	// TODO
	return false;
}

} // End of namespace Titanic
