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

bool XMLParser::parserError(const char *errorString, ...) {
	_state = kParserError;

	int pos = _pos;
	int lineCount = 1;
	int lineStart = 0;

	if (_fileName == "Memory Stream") {
		lineStart = MAX(0, _pos - 35);
		lineCount = 0;
	} else {
		do {
			if (_text[pos] == '\n' || _text[pos] == '\r') {
				lineCount++;
		
				if (lineStart == 0)
					lineStart = MAX(pos + 1, _pos - 60);
			}
		} while (pos-- > 0);
	}

	char lineStr[70];
	_text.stream()->seek(lineStart, SEEK_SET);
	_text.stream()->readLine(lineStr, 70);
	
	for (int i = 0; i < 70; ++i)
		if (lineStr[i] == '\n')
			lineStr[i] = ' ';

	printf("\n  File <%s>, line %d:\n", _fileName.c_str(), lineCount);

	bool startFull = lineStr[0] == '<';
	bool endFull = lineStr[strlen(lineStr) - 1] == '>';

	printf("%s%s%s\n", startFull ? "" : "...", lineStr, endFull ? "" : "...");

	int cursor = MIN(_pos - lineStart, 70);

	if (!startFull)
		cursor += 3;

	while (cursor--)
		printf(" ");

	printf("^\n");
	printf("Parser error: ");

	va_list args;
	va_start(args, errorString);
	vprintf(errorString, args);
	va_end(args);

	printf("\n\n");

	return false;
}

bool XMLParser::parseActiveKey(bool closed) {
	bool ignore = false;
	assert(_activeKey.empty() == false);

	ParserNode *key = _activeKey.top();
	XMLKeyLayout *layout = (_activeKey.size() == 1) ? _XMLkeys : getParentNode(key)->layout;
	
	if (layout->children.contains(key->name) == false)
		return parserError("Unexpected key in the active scope: '%s'.", key->name.c_str());
		
	key->layout = layout->children[key->name];
	
	Common::StringMap localMap = key->values;
	
	for (Common::List<XMLKeyLayout::XMLKeyProperty>::const_iterator i = key->layout->properties.begin(); i != key->layout->properties.end(); ++i) {
		if (localMap.contains(i->name))
			localMap.erase(i->name);
		else if (i->required)
			return parserError("Missing required property '%s' inside key '%s'", i->name.c_str(), key->name.c_str());
	}
	
	if (key->layout.anyProps == false && localMap.empty() == false)
		return parserError("Unhandled property inside key '%s': '%s'", key->name.c_str(), localMap.begin()->_key.c_str());

	// check if any of the parents must be ignored.
	// if a parent is ignored, all children are too.
	for (int i = _activeKey.size() - 1; i >= 0; --i) {
		if (_activeKey[i]->ignore)
			ignore = true;
	}

	if (ignore == false && keyCallback(key) == false) {
		// HACK:  People may be stupid and overlook the fact that
		// when keyCallback() fails, a parserError() must be set.
		// We set it manually in that case.
		if (_state != kParserError)
			parserError("Unhandled exception when parsing '%s' key.", key->name.c_str());
			
		return false;
	}
	
	if (closed)
		delete _activeKey.pop();

	return true;
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

	if (_text.ready() == false)
		return parserError("XML stream not ready for reading.");
		
	if (_XMLkeys == 0)
		buildLayout();

	cleanup();

	bool activeClosure = false;
	bool selfClosure = false;

	_state = kParserNeedKey;
	_pos = 0;
	_activeKey.clear();
	
	while (_text[_pos] && _state != kParserError) {
		if (skipSpaces())
			continue;

		if (skipComments())
			continue;

		switch (_state) {
			case kParserNeedKey:
				if (_text[_pos++] != '<') {
					parserError("Parser expecting key start.");
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
					if (_activeKey.empty() || _token != _activeKey.top()->name) {
						parserError("Unexpected closure.");
						break;
					}
				} else {
					ParserNode *node = new ParserNode;
					node->name = _token;
					node->ignore = false;
					node->depth = _activeKey.size();
					node->layout = 0;
					_activeKey.push(node);
				}

				_state = kParserNeedPropertyName;
				break;

			case kParserNeedPropertyName:
				if (activeClosure) {
					if (!closedKeyCallback(_activeKey.top())) {
						parserError("Missing data when closing key '%s'.", _activeKey.top()->name.c_str()); 
						break;
					}

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
					if (parseActiveKey(selfClosure)) {
						_pos += selfClosure ? 2 : 1;
						_state = kParserNeedKey;
					}
					break;
				}

				if (!parseToken()) 
					parserError("Error when parsing key value.");
				else 
					_state = kParserNeedPropertyOperator;

				break;

			case kParserNeedPropertyOperator:
				if (_text[_pos++] != '=') 
					parserError("Syntax error after key name.");
				else  
					_state = kParserNeedPropertyValue;

				break;

			case kParserNeedPropertyValue:
				if (!parseKeyValue(_token)) 
					parserError("Invalid key value.");
				else 
					_state = kParserNeedPropertyName;

				break;

			default:
				break;
		}
	}

	if (_state == kParserError)
		return false;

	if (_state != kParserNeedKey || !_activeKey.empty())
		return parserError("Unexpected end of file.");

	return true;
}

}

