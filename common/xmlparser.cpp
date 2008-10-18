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
	
	const int startPosition = _stream->pos();
	const int middle = kErrorMessageWidth / 2;
	
	int currentPosition = startPosition;
	int lineCount = 1, realMiddle = 0;
	char errorBuffer[kErrorMessageWidth];
	char c, *errorKeyStart = 0;
	
	_stream->seek(0, SEEK_SET);
	
	while (currentPosition--) {
		c = _stream->readByte();
		
		if (c == '\n' || c == '\r')
			lineCount++;
	}
	
	_stream->seek(-MIN(middle, startPosition), SEEK_CUR);
	
	for (int i = 0, j = 0; i < kErrorMessageWidth; ++i, ++j) {
		c = _stream->readByte();
		
		if (c == '\n' || c == '\r') {
			errorBuffer[i++] = ' ';
			j++;
			
			while (c && isspace(c)) {
				c = _stream->readByte();
				j++;
			}
		}
			
		errorBuffer[i] = c;
		if (!realMiddle && j >= middle)
			realMiddle = i;
	}
		
	for (int i = realMiddle; i >= 0; --i)
		if (errorBuffer[i] == '<') {
			errorKeyStart = &errorBuffer[i];
			break;
		}
		
	for (int i = realMiddle; i < kErrorMessageWidth; ++i)
		if (errorBuffer[i] == '>') {
			errorBuffer[i + 1] = 0;
			break;
		}
	
	fprintf(stderr, "\n  File <%s>, line %d:\n", _fileName.c_str(), lineCount);
	
	if (!errorKeyStart)
		fprintf(stderr, "...%s%s\n", errorBuffer, errorBuffer[strlen(errorBuffer) - 1] == '>' ? "" : "...");
	else
		fprintf(stderr, "%s%s\n", errorKeyStart, errorBuffer[strlen(errorBuffer) - 1] == '>' ? "" : "...");

	fprintf(stderr, "\nParser error: ");

	va_list args;
	va_start(args, errorString);
	vfprintf(stderr, errorString, args);
	va_end(args);

	fprintf(stderr, "\n\n");

	return false;
}

bool XMLParser::parseXMLHeader(ParserNode *node) {
	assert(node->header);
	
	if (_activeKey.size() != 1)
		return parserError("XML Header is expected in the global scope.");
		
	if (!node->values.contains("version"))
		return parserError("Missing XML version in XML header.");
		
	if (node->values["version"] != "1.0")
		return parserError("Unsupported XML version.");
		
	return true;
}

bool XMLParser::parseActiveKey(bool closed) {
	bool ignore = false;
	assert(_activeKey.empty() == false);

	ParserNode *key = _activeKey.top();
	
	if (key->name == "xml" && key->header == true) {
		assert(closed);
		return parseXMLHeader(key) && closeKey();
	}
	
	XMLKeyLayout *layout = (_activeKey.size() == 1) ? _XMLkeys : getParentNode(key)->layout;
	
	if (layout->children.contains(key->name)) {
		key->layout = layout->children[key->name];
	
		Common::StringMap localMap = key->values;
		int keyCount = localMap.size();
	
		for (Common::List<XMLKeyLayout::XMLKeyProperty>::const_iterator i = key->layout->properties.begin(); i != key->layout->properties.end(); ++i) {
			if (i->required && !localMap.contains(i->name))
				return parserError("Missing required property '%s' inside key '%s'", i->name.c_str(), key->name.c_str());
			else if (localMap.contains(i->name))
				keyCount--;
		}
	
		if (keyCount > 0)
			return parserError("Unhandled property inside key '%s'.", key->name.c_str());
			
	} else {
		return parserError("Unexpected key in the active scope ('%s').", key->name.c_str());
	}

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
		return closeKey();

	return true;
}

bool XMLParser::parseKeyValue(Common::String keyName) {
	assert(_activeKey.empty() == false);

	if (_activeKey.top()->values.contains(keyName))
		return false;

	_token.clear();
	char stringStart;

	if (_char == '"' || _char == '\'') {
		stringStart = _char;
		_char = _stream->readByte();

		while (_char && _char != stringStart) {
			_token += _char;
			_char = _stream->readByte();
		}

		if (_char == 0)
			return false;

		_char = _stream->readByte();

	} else if (!parseToken()) {
		return false;
	}

	_activeKey.top()->values[keyName] = _token;
	return true;
}

bool XMLParser::closeKey() {
	bool ignore = false;
	bool result = true;
	
	for (int i = _activeKey.size() - 1; i >= 0; --i) {
		if (_activeKey[i]->ignore)
			ignore = true;
	}
	
	if (ignore == false)
		result = closedKeyCallback(_activeKey.top());
		
	freeNode(_activeKey.pop());
	
	return result;
}

bool XMLParser::parse() {

	if (_stream == 0)
		return parserError("XML stream not ready for reading.");
		
	if (_XMLkeys == 0)
		buildLayout();

	while (!_activeKey.empty())
		freeNode(_activeKey.pop());

	cleanup();

	bool activeClosure = false;
	bool activeHeader = false;
	bool selfClosure;

	_state = kParserNeedHeader;
	_activeKey.clear();

	_char = _stream->readByte();
	
	while (_char && _state != kParserError) {
		if (skipSpaces())
			continue;

		if (skipComments())
			continue;

		switch (_state) {
			case kParserNeedHeader:
			case kParserNeedKey:
				if (_char != '<') {
					parserError("Parser expecting key start.");
					break;
				}

				if ((_char = _stream->readByte()) == 0) {
					parserError("Unexpected end of file.");
					break;
				}

				if (_state == kParserNeedHeader) {
					if (_char != '?') {
						parserError("Expecting XML header.");
						break;
					}
					
					_char = _stream->readByte();
					activeHeader = true;
				} else if (_char == '/') {
					_char = _stream->readByte();
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
					ParserNode *node = allocNode(); //new ParserNode;
					node->name = _token;
					node->ignore = false;
					node->header = activeHeader;
					node->depth = _activeKey.size();
					node->layout = 0;
					_activeKey.push(node);
				}

				_state = kParserNeedPropertyName;
				break;

			case kParserNeedPropertyName:
				if (activeClosure) {
					if (!closeKey()) {
						parserError("Missing data when closing key '%s'.", _activeKey.top()->name.c_str()); 
						break;
					}

					activeClosure = false;

					if (_char != '>')
						parserError("Invalid syntax in key closure.");
					else 
						_state = kParserNeedKey;

					_char = _stream->readByte();
					break;
				}
				
				selfClosure = false;

				if (_char == '/' || (_char == '?' && activeHeader)) {
					selfClosure = true;
					_char = _stream->readByte();
				}

				if (_char == '>') {
					if (activeHeader && !selfClosure) {
						parserError("XML Header must be self-closed.");
					} else if (parseActiveKey(selfClosure)) {
						_char = _stream->readByte();
						_state = kParserNeedKey;
					}
					
					activeHeader = false;
					break;
				}
				
				if (selfClosure)
					parserError("Expecting key closure after '/' symbol.");
				else if (!parseToken()) 
					parserError("Error when parsing key value.");
				else 
					_state = kParserNeedPropertyOperator;

				break;

			case kParserNeedPropertyOperator:
				if (_char != '=') 
					parserError("Syntax error after key name.");
				else  
					_state = kParserNeedPropertyValue;

				_char = _stream->readByte();
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

