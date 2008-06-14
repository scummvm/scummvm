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

#ifndef THEME_PARSER_H
#define THEME_PARSER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"

namespace GUI {

class ThemeParser {

	static const int kParserMaxDepth = 4;
	typedef void (ThemeParser::*ParserCallback)();

public:
	ThemeParser() {
		_callbacks["drawdata"] = &ThemeParser::parserCallback_DRAWDATA;
		_callbacks["draw"] = &ThemeParser::parserCallback_DRAW;
	}

	~ThemeParser() {}

	enum ParserState {
		kParserNeedKey,
		kParserNeedKeyName,

		kParserNeedPropertyName,
		kParserNeedPropertyOperator,
		kParserNeedPropertyValue,

		kParserError
	};

	bool parse();
	void debug_testEval();
	
protected:
	void parserCallback_DRAW();
	void parserCallback_DRAWDATA();

	bool parseKeyValue(Common::String &key_name);
	void parseActiveKey(bool closed);
	void parserError(const char *error_string);

	inline bool skipSpaces() {
		if (!isspace(_text[_pos]))
			return false;

		while (_text[_pos] && isspace(_text[_pos]))
			_pos++;

		return true;
	}

	inline bool skipComments() {
		if (_text[_pos] == '/' && _text[_pos + 1] == '*') {
			_pos += 2;
			while (_text[_pos++]) {
				if (_text[_pos - 2] == '*' && _text[_pos - 1] == '/')
					break;
			}
			return true;
		}
		return false;
	}

	inline bool isValidNameChar(char c) {
		return isalnum(c) || c == '_';
	}

	inline bool parseToken() {
		_token.clear();
		while (isValidNameChar(_text[_pos]))
			_token += _text[_pos++];

		return isspace(_text[_pos]) != 0 || _text[_pos] == '>';
	}

	int _pos;
	char *_text;

	ParserState _state;

	Common::String _error;
	Common::String _token;

	Common::FixedStack<Common::String, kParserMaxDepth> _activeKey;
	Common::FixedStack<Common::StringMap, kParserMaxDepth> _keyValues;

	Common::HashMap<Common::String, ParserCallback, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _callbacks;
};

}

#endif
