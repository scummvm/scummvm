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

#include "gui/InterfaceManager.h"
#include "gui/ThemeParser.h"
#include "graphics/VectorRenderer.h"

/**

<drawdata = "background_default" cache = true>
	<draw func = "roundedsq" fill = "gradient" gradient_start = "255, 255, 128" gradient_end = "128, 128, 128" size = "auto">
	<draw func = "roundedsq" fill = "none" color = "0, 0, 0" size = "auto">
</drawdata>

*/

namespace GUI {

using namespace Graphics;

void ThemeParser::debug_testEval() {
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
	

void ThemeParser::parserError(const char *error_string) {
	_state = kParserError;
	printf("PARSER ERROR: %s\n", error_string);
}

Graphics::DrawStep *ThemeParser::newDrawStep() {

	Graphics::DrawStep *step = new DrawStep;

	step->fgColor.set = false;
	step->bgColor.set = false;
	step->gradColor1.set = false;
	step->gradColor2.set = false;

	step->extraData = 0;
	step->factor = 1;
	step->fillArea = false;
	step->fillMode = Graphics::VectorRenderer::kFillDisabled;
	step->scale = (1 << 16);
	step->shadow = 0;
	step->stroke = 1;

	return step;
}

void ThemeParser::parserCallback_DRAWSTEP() {
	ParserNode *stepNode = _activeKey.pop();
	ParserNode *drawdataNode = _activeKey.pop();

	assert(stepNode->name == "drawstep");
	assert(drawdataNode->name == "drawdata");
	assert(drawdataNode->values.contains("id"));

	Graphics::DrawStep *drawstep = newDrawStep();

	Common::String functionName = stepNode->values["func"]; 

	if (_drawFunctions.contains(functionName) == false) {
		parserError("Invalid drawing function in draw step.");
		_activeKey.push(drawdataNode);
		_activeKey.push(stepNode);
		return;
	}	

	drawstep->drawingCall = _drawFunctions[functionName];

	if (stepNode->values.contains("stroke")) {

	}

	if (functionName == "roundedsq") {

	}

	g_InterfaceManager.addDrawStep(drawdataNode->values["id"], drawstep);

	_activeKey.push(drawdataNode);
	_activeKey.push(stepNode);
}

void ThemeParser::parserCallback_DRAWDATA() {
	printf("Drawdata callback!\n");
}

void ThemeParser::parseActiveKey(bool closed) {
	printf("Parsed key %s.\n", _activeKey.top()->name.c_str());

	if (!_callbacks.contains(_activeKey.top()->name)) {
		parserError("Unhandled value inside key.");
		return;
	}

	// Don't you just love C++ syntax? Water clear.
	(this->*(_callbacks[_activeKey.top()->name]))();

//	for (Common::StringMap::const_iterator t = _activeKey.top()->values.begin(); t != _activeKey.top()->values.end(); ++t)
//		printf("    Key %s = %s\n", t->_key.c_str(), t->_value.c_str());

	if (closed) {
		delete _activeKey.pop();
	}
}

bool ThemeParser::parseKeyValue(Common::String keyName) {
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

bool ThemeParser::parse() {

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

