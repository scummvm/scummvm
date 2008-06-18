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

#include "gui/InterfaceManager.h"
#include "gui/ThemeParser.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

using namespace Graphics;
using namespace Common;

ThemeParser::ThemeParser() : XMLParser() {
	_callbacks["drawstep"] = &ThemeParser::parserCallback_DRAWSTEP;
	_callbacks["drawdata"] = &ThemeParser::parserCallback_DRAWDATA;
}

bool ThemeParser::keyCallback(Common::String keyName) {
	// automatically handle with a function from the hash table.
	if (!_callbacks.contains(_activeKey.top()->name))
		return false;

	return (this->*(_callbacks[_activeKey.top()->name]))();
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

bool ThemeParser::parserCallback_DRAWSTEP() {
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
		return false;
	}	

	drawstep->drawingCall = _drawFunctions[functionName];

	if (stepNode->values.contains("stroke")) {

	}

	if (functionName == "roundedsq") {

	}

	g_InterfaceManager.addDrawStep(drawdataNode->values["id"], drawstep);

	_activeKey.push(drawdataNode);
	_activeKey.push(stepNode);

	return true;
}

bool ThemeParser::parserCallback_DRAWDATA() {
	printf("Drawdata callback!\n");
	return true;
}

}

