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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "common/system.h"
#include "common/events.h"

#include "gui/ThemeParser.h"

/**

<drawdata = "background_default" cache = true>
	<draw func = "roundedsq" fill = "gradient" gradient_start = "255, 255, 128" gradient_end = "128, 128, 128" size = "auto">
	<draw func = "roundedsq" fill = "none" color = "0, 0, 0" size = "auto">
</drawdata>

*/

namespace GUI {

inline bool isValidNameChar(char c) {
	return !isspace(c) && c != '/' && c != '*' && c != '\\' && 
		c != '|' && c != '"' && c != '\'' && c != ',' && 
		c != '<' && c != '>' && c != '=';
}

void ThemeParser::parserError(const char *error_string) {
	_state = kParserError;
}

bool ThemeParser::parseDrawData() {

	_state = kParserNeedKey;
	
	while (_text[_pos++]) {

		while (isspace(_text[_pos]))
			_pos++;

		// comment handling: skip everything between /* and */
		if (_text[_pos] == '/') {
			if (_text[++_pos] != '*') {
				parserError("Malformed comment string.");
				return false;
			}
			
			_pos++;

			while (_text[_pos] != '*' && _text[_pos + 1] != '/')
				_pos++;
		}

		switch (_state) {
			case kParserNeedKey:
				if (_text[_pos] != '<') {
					parserError("Expecting key start.");
					return false;
				}

				_state = kParserKeyNeedName;
				break;

			case kParserKeyNeedName:
				_token.clear();
				while (isValidNameChar(_text[_pos]))
					_token += _text[_pos++];

				if (!isspace(_text[_pos])) {
					parserError("Invalid character in token name.");
					return false;
				}

				_state = kParserKeyNeedToken;
				break;

			case kParserKeyNeedToken:
				_token.clear();
				break;

			default:
				return false;
		}
	}

	return true;
}

}

