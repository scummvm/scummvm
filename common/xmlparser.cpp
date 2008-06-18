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
#include "common/xmlparser.h"

namespace Common {

using namespace Graphics;

void XMLParser::debug_testEval() {
	static const char *debugConfigText =
		"</* lol this is just a moronic test */drawdata id = \"background_default\" cache = true>\n"
		"<drawstep func = \"roundedsq\" fill = \"gradient\" gradient_start = \"255, 255, 128\" gradient_end = \"128, 128, 128\" size = \"auto\"/>\n"
		"//<drawstep func = \"roundedsq\" fill = \"none\" color = /*\"0, 0, 0\"*/\"0, 1, 2\" size = \"auto\"/>\n"
		"</ drawdata>/* lol this is just a simple test*/\n";

	_text = strdup(debugConfigText);

	Common::String test = "12,  125, 125";

	printf("\n\nRegex result: %s.\n\n", test.regexMatch("^[d]*,[d]*,[d]*$", true) ? "Success." : "Fail");

	parse();
}


void XMLParser::parserError(const char *error_string) {
	_state = kParserError;
	printf("PARSER ERROR: %s\n", error_string);
}

void XMLParser::parseActiveKey(bool closed) {
	if (keyCallback(_activeKey.top()->name) == false) {
		parserError("Unhandled value inside key.");
		return;
	}
	
	if (closed) {
		delete _activeKey.pop();
	}
}

bool XMLParser::parseKeyValue(Common::String keyName) {
	assert(_activeKey.empty() == false);

	if (_activeKey.top()->values.contains(keyName))
		return false;

	_token.clear();
	char stringStart;

	if (_text[_pos] == '"' || _text[_pos] == '\'') {
		stringStart = _text[_pos++];

		while (_text[_pos] && _text[_pos] != stringStart)
			_token += _text[_pos++];

		if (_text[_pos++] == 0)
			return false;

	} else if (!parseToken()) {
		return false;
	}

	_activeKey.top()->values[keyName] = _token;
	return true;
}

bool XMLParser::parse() {

	bool activeClosure = false;
	bool selfClosure = false;

	_state = kParserNeedKey;
	_pos = 0;
	_activeKey.clear();
	
	while (_text[_pos]) {
		if (_state == kParserError)
			break;

		if (skipSpaces())
			continue;

		if (skipComments())
			continue;

		switch (_state) {
			case kParserNeedKey:
				if (_text[_pos++] != '<') {
					parserError("Expecting key start.");
					break;
				}

				if (_text[_pos] == 0) {
					parserError("Unexpected end of file.");
					break;
				}

				if (_text[_pos] == '/' && _text[_pos + 1] != '*') {
					_pos++;
					activeClosure = true;
				}

				_state = kParserNeedKeyName;
				break;

			case kParserNeedKeyName:
				if (!parseToken()) {
					parserError("Invalid key name.");
					break;
				}

				if (activeClosure) {
					if (_activeKey.empty() || _token != _activeKey.top()->name)
						parserError("Unexpected closure.");
				} else {
					ParserNode *node = new ParserNode;
					node->name = _token;
					_activeKey.push(node);
				}

				_state = kParserNeedPropertyName;
				break;

			case kParserNeedPropertyName:
				if (activeClosure) {
					activeClosure = false;
					delete _activeKey.pop();

					if (_text[_pos++] != '>')
						parserError("Invalid syntax in key closure.");
					else 
						_state = kParserNeedKey;

					break;
				}

				selfClosure = (_text[_pos] == '/');

				if ((selfClosure && _text[_pos + 1] == '>') || _text[_pos] == '>') {
					parseActiveKey(selfClosure);
					_pos += selfClosure ? 2 : 1;
					_state = kParserNeedKey;
					break;
				}

				if (!parseToken()) 
					parserError("Error when parsing key value.");
				else 
					_state = kParserNeedPropertyOperator;

				break;

			case kParserNeedPropertyOperator:
				if (_text[_pos++] != '=') 
					parserError("Unexpected character after key name.");
				else  
					_state = kParserNeedPropertyValue;

				break;

			case kParserNeedPropertyValue:
				if (!parseKeyValue(_token)) 
					parserError("Unable to parse key value.");
				else 
					_state = kParserNeedPropertyName;

				break;

			default:
				break;
		}
	}

	if (_state == kParserError) {
		return false;
	}

	if (_state != kParserNeedKey || !_activeKey.empty()) {
		parserError("Unexpected end of file.");
		return false;
	}

	return true;
}

}

