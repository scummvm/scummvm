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
#include "common/hashmap.h"
#include "common/hash-str.h"

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

void ThemeParser::debug_testEval() {
	static const char *debug_config_text =
		"<drawdata id = \"background_default\" cache = true>"
		"<draw func = \"roundedsq\" /*/fill = \"gradient\" gradient_start = \"255, 255, 128\" gradient_end = \"128, 128, 128\" size = \"auto\"/>"
		"/*<draw func = \"roundedsq\" fill = \"none\" color = \"0, 0, 0\" size = \"auto\"/>*/"
		"</drawdata>/* lol this is just a simple test*/";

	_text = strdup(debug_config_text);
	parseDrawData();
}
	

void ThemeParser::parserError(const char *error_string) {
	_state = kParserError;
	printf("PARSER ERROR: %s\n", error_string);
}

void ThemeParser::parseActiveKey(bool closed) {
	printf("Parsed key %s.\n", _activeKey.top().c_str());

	for (Common::StringMap::const_iterator t = _keyValues.begin(); t != _keyValues.end(); ++t)
		printf("    Key %s = %s\n", t->_key.c_str(), t->_value.c_str());

	if (closed)
		_activeKey.pop();

	_keyValues.clear();
}

void ThemeParser::parseKeyValue(Common::String &key_name) {
	assert(_text[_pos++] == '=');

	while (isspace(_text[_pos]))
		_pos++;

	Common::String data;

	if (_text[_pos] == '"') {
		data += _text[_pos++];

		while (_text[_pos] != '"')
			data += _text[_pos++];

		data += _text[_pos++];
	} else {
		while (isValidNameChar(_text[_pos]))
			data += _text[_pos++];
	}

	_keyValues[key_name] = data;
}

bool ThemeParser::parseDrawData() {

	_state = kParserNeedKey;
	_pos = 0;
	_keyValues.clear();
	
	while (_text[_pos]) {
		while (isspace(_text[_pos]))
			_pos++;

		// comment handling: skip everything between /* and */
		if (_text[_pos] == '/' && _text[_pos + 1] == '*') {
			_pos += 2;
			while (_text[_pos++]) {
				if (_text[_pos - 2] == '*' && _text[_pos - 1] == '/')
					break;
			}
			continue;
		}

		switch (_state) {
			case kParserNeedKey:
				if (_text[_pos++] != '<') {
					parserError("Expecting key start.");
					return false;
				}

				if (_text[_pos] == '/') {
					_pos++;
					_token.clear();
					while (isValidNameChar(_text[_pos]))
						_token += _text[_pos++];

					if (_token == _activeKey.top()) {
						_activeKey.pop();

						while (isspace(_text[_pos]) || _text[_pos] == '>')
							_pos++;

						break;
					} else {
						parserError("Unexpected closure.");
						return false;
					}
				}

				_state = kParserKeyNeedName;
				break;

			case kParserKeyNeedName:
				_token.clear();
				while (isValidNameChar(_text[_pos]))
					_token += _text[_pos++];

				if (!isspace(_text[_pos]) && _text[_pos] != '>') {
					parserError("Invalid character in token name.");
					return false;
				}

				_state = kParserKeyNeedToken;
				_activeKey.push(_token);
				break;

			case kParserKeyNeedToken:
				_token.clear();

				if ((_text[_pos] == '/' && _text[_pos + 1] == '>') || _text[_pos] == '>') {
					bool closed = _text[_pos] == '/';
					parseActiveKey(closed);
					_pos += closed ? 2 : 1;
					_state = kParserNeedKey;
					break;
				}

				while (isValidNameChar(_text[_pos]))
					_token += _text[_pos++];

				while (isspace(_text[_pos]))
					_pos++;

				if (_text[_pos] != '=') {
					parserError("Unexpected character after key name.");
					return false;
				}

				parseKeyValue(_token);
				break;

			default:
				return false;
		}
	}

	if (_state != kParserNeedKey || !_activeKey.empty()) {
		parserError("Unexpected end of file.");
	}

	return true;
}

}

