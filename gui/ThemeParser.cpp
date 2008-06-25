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

	_drawFunctions["circle"]  = &Graphics::VectorRenderer::drawCallback_CIRCLE;
	_drawFunctions["square"]  = &Graphics::VectorRenderer::drawCallback_SQUARE;
	_drawFunctions["roundedsq"]  = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	_drawFunctions["bevelsq"]  = &Graphics::VectorRenderer::drawCallback_BEVELSQ;
	_drawFunctions["line"]  = &Graphics::VectorRenderer::drawCallback_LINE;
	_drawFunctions["triangle"]  = &Graphics::VectorRenderer::drawCallback_TRIANGLE;
	_drawFunctions["fill"]  = &Graphics::VectorRenderer::drawCallback_FILLSURFACE;
	_drawFunctions["void"]  = &Graphics::VectorRenderer::drawCallback_VOID;
}

bool ThemeParser::keyCallback(Common::String keyName) {
	// automatically handle with a function from the hash table.
	if (!_callbacks.contains(_activeKey.top()->name)) {
		parserError("%s is not a valid key name.", keyName.c_str());
		return false;
	}

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
	step->fillArea = true;
	step->fillMode = Graphics::VectorRenderer::kFillDisabled;
	step->scale = (1 << 16);
	step->shadow = 0;
	step->stroke = 1;

	return step;
}

bool ThemeParser::parserCallback_DRAWSTEP() {
	ParserNode *stepNode = _activeKey.top();

	// HACK: Any cleaner way to access the second item from
	// the top without popping? Let's keep it this way and hope
	// the internal representation doesn't change
	ParserNode *drawdataNode = _activeKey[_activeKey.size() - 2];

	assert(stepNode->name == "drawstep");
	assert(drawdataNode->name == "drawdata");
	assert(drawdataNode->values.contains("id"));

	Graphics::DrawStep *drawstep = newDrawStep();

	Common::String functionName = stepNode->values["func"]; 

	if (_drawFunctions.contains(functionName) == false) {
		parserError("%s is not a valid drawing function name", functionName.c_str());
		return false;
	}	

	drawstep->drawingCall = _drawFunctions[functionName];

	uint32 red, green, blue, w, h;

/**
 * Helper macro to sanitize and assign an integer value from a key
 * to the draw step.
 *
 * @param struct_name Name of the field of a DrawStep struct that must be
 *                    assigned.
 * @param key_name Name as STRING of the key identifier as it appears in the
 *                 theme description format.
 * @param force Sets if the key is optional or necessary.
 */
#define __PARSER_ASSIGN_INT(struct_name, key_name, force) \
	if (stepNode->values.contains(key_name)) { \
		if (!validateKeyInt(stepNode->values[key_name].c_str())) {\
			parserError("Error when parsing key value for '%s'.", key_name); \
			return false; \
		} \
		drawstep->struct_name = atoi(stepNode->values[key_name].c_str()); \
	} else if (force) { \
		parserError("Missing necessary key '%s'.", key_name); \
		return false; \
	}

/**
 * Helper macro to sanitize and assign a RGB value from a key to the draw
 * step. RGB values have the following syntax: "R, G, B".
 *
 * TODO: Handle also specific name colors such as "red", "green", etc.
 *
 * @param struct_name Name of the field of a DrawStep struct that must be
 *                    assigned.
 * @param key_name Name as STRING of the key identifier as it appears in the
 *                 theme description format.
 */
#define __PARSER_ASSIGN_RGB(struct_name, key_name) \
	if (stepNode->values.contains(key_name)) { \
		if (sscanf(stepNode->values[key_name].c_str(), "%d, %d, %d", &red, &green, &blue) != 3 || \
			red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {\
				parserError("Error when parsing color struct '%s'", stepNode->values[key_name].c_str());\
				return false; \
			}\
		drawstep->struct_name.r = red; \
		drawstep->struct_name.g = green; \
		drawstep->struct_name.b = blue; \
		drawstep->struct_name.set = true; \
	}

	__PARSER_ASSIGN_INT(stroke, "stroke", false);
	__PARSER_ASSIGN_INT(shadow, "shadow", false);
	__PARSER_ASSIGN_INT(factor, "gradient_factor", false);

	__PARSER_ASSIGN_RGB(fgColor, "fg_color");
	__PARSER_ASSIGN_RGB(bgColor, "bg_color");
	__PARSER_ASSIGN_RGB(gradColor1, "gradient_start");
	__PARSER_ASSIGN_RGB(gradColor2, "gradient_end");

	if (functionName == "roundedsq" || functionName == "circle") {
		__PARSER_ASSIGN_INT(radius, "radius", true)
	}

	if (functionName == "bevelsq") {
		__PARSER_ASSIGN_INT(extraData, "bevel", true);
	}

	if (functionName == "triangle") {

	}

	if (stepNode->values.contains("size")) {
		if (stepNode->values["size"] == "auto") {
			drawstep->fillArea = true;
		} else if (sscanf(stepNode->values["size"].c_str(), "%d, %d", &w, &h) == 2) {
			drawstep->fillArea = false;
			drawstep->w = w;
			drawstep->h = h;
		} else {
			parserError("Invalid value in 'size' subkey: Valid options are 'auto' or 'X, X' to define width and height.");
			return false;
		}
	}

	if (stepNode->values.contains("fill")) {
		if (stepNode->values["fill"] == "none")
			drawstep->fillMode = VectorRenderer::kFillDisabled;
		else if (stepNode->values["fill"] == "foreground")
			drawstep->fillMode = VectorRenderer::kFillForeground;
		else if (stepNode->values["fill"] == "background")
			drawstep->fillMode = VectorRenderer::kFillBackground;
		else if (stepNode->values["fill"] == "gradient")
			drawstep->fillMode = VectorRenderer::kFillGradient;
		else {
			parserError("'%s' is not a valid fill mode for a shape.", stepNode->values["fill"].c_str());
			return false;
		}
	}

#undef __PARSER_ASSIGN_INT
#undef __PARSER_ASSIGN_RGB

	g_InterfaceManager.addDrawStep(drawdataNode->values["id"], drawstep);
	return true;
}

bool ThemeParser::parserCallback_DRAWDATA() {
	ParserNode *drawdataNode = _activeKey.top();
	bool cached = false;

	if (drawdataNode->values.contains("id") == false) {
		parserError("DrawData notes must contain an identifier.");
		return false;
	}

	InterfaceManager::DrawData id = g_InterfaceManager.getDrawDataId(drawdataNode->values["id"]);

	if (id == -1) {
		parserError("%d is not a valid DrawData set identifier.", drawdataNode->values["id"].c_str());
		return false;
	}

	if (drawdataNode->values.contains("cached") && drawdataNode->values["cached"] == "true") {
		cached = true;
	}

	// Both Max and Johannes suggest using a non-platform specfic approach based on available
	// resources and active resolution. getHostPlatformString() has been removed, so fix this.

/*	if (drawdataNode->values.contains("platform")) {
		if (drawdataNode->values["platform"].compareToIgnoreCase(Common::getHostPlatformString()) != 0) {
			drawdataNode->ignore = true;
			return true;
		}
	}*/

	if (g_InterfaceManager.addDrawData(id, cached) == false) {
		parserError("Repeated DrawData: Only one set of Drawing Data for a widget may be specified on each platform.");
		return false;
	}

	return true;
}

}

