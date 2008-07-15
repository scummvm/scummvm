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

#include "gui/ThemeRenderer.h"
#include "gui/ThemeParser.h"
#include "gui/newgui.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

using namespace Graphics;
using namespace Common;

ThemeParser::ThemeParser(ThemeRenderer *parent) : XMLParser() {
	_callbacks["drawstep"] = &ThemeParser::parserCallback_DRAWSTEP;
	_callbacks["drawdata"] = &ThemeParser::parserCallback_DRAWDATA;
	_callbacks["palette"] = &ThemeParser::parserCallback_palette;
	_callbacks["color"] = &ThemeParser::parserCallback_color;
	_callbacks["render_info"] = &ThemeParser::parserCallback_renderInfo;
	_callbacks["layout_info"] = &ThemeParser::parserCallback_layoutInfo;
	_callbacks["default"] = &ThemeParser::parserCallback_defaultSet;
	_callbacks["text"] = &ThemeParser::parserCallback_text;
	
	_drawFunctions["circle"]  = &Graphics::VectorRenderer::drawCallback_CIRCLE;
	_drawFunctions["square"]  = &Graphics::VectorRenderer::drawCallback_SQUARE;
	_drawFunctions["roundedsq"]  = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	_drawFunctions["bevelsq"]  = &Graphics::VectorRenderer::drawCallback_BEVELSQ;
	_drawFunctions["line"]  = &Graphics::VectorRenderer::drawCallback_LINE;
	_drawFunctions["triangle"]  = &Graphics::VectorRenderer::drawCallback_TRIANGLE;
	_drawFunctions["fill"]  = &Graphics::VectorRenderer::drawCallback_FILLSURFACE;
	_drawFunctions["void"]  = &Graphics::VectorRenderer::drawCallback_VOID;

	_defaultStepGlobal = defaultDrawStep();
	_defaultStepLocal = 0;
	_theme = parent;
}

void ThemeParser::cleanup() {
	delete _defaultStepGlobal;
	delete _defaultStepLocal;

	_defaultStepGlobal = defaultDrawStep();
	_defaultStepLocal = 0;
	_palette.clear();
}

bool ThemeParser::keyCallback(Common::String keyName) {
	// automatically handle with a function from the hash table.
	if (!_callbacks.contains(_activeKey.top()->name))
		return parserError("%s is not a valid key name.", keyName.c_str());

	return (this->*(_callbacks[_activeKey.top()->name]))();
}

Graphics::DrawStep *ThemeParser::defaultDrawStep() {
	Graphics::DrawStep *step = new DrawStep;

	step->fgColor.set = false;
	step->bgColor.set = false;
	step->gradColor1.set = false;
	step->gradColor2.set = false;

	step->xAlign = Graphics::DrawStep::kVectorAlignManual;
	step->yAlign = Graphics::DrawStep::kVectorAlignManual;
	step->x = 0;
	step->y = 0;
	step->w = 0;
	step->h = 0;
	
	step->extraData = 0;
	step->factor = 1;
	step->autoWidth = true;
	step->autoHeight = true;
	step->fillMode = Graphics::VectorRenderer::kFillDisabled;
	step->scale = (1 << 16);
	step->shadow = 0;
	step->stroke = 0;
	step->radius = 0xFF;

	return step;
}

Graphics::DrawStep *ThemeParser::newDrawStep() {
	assert(_defaultStepGlobal);
	Graphics::DrawStep *step = new DrawStep;

	if (_defaultStepLocal) {
		memcpy(step, _defaultStepLocal, sizeof(DrawStep));
	} else {
		memcpy(step, _defaultStepGlobal, sizeof(DrawStep));
	}

	return step;
}

bool ThemeParser::parserCallback_defaultSet() {
	ParserNode *defNode = getActiveNode();
	ParserNode *parentNode = getParentNode(defNode);
	Graphics::DrawStep *step = 0;

	if (parentNode == 0)
		return parserError("The <default> key must be contained inside <render_info> keys.");

	if (parentNode->name == "render_info") {
		step = _defaultStepGlobal;
	} else if (parentNode->name == "drawdata") {
		if (_defaultStepLocal == 0)
			_defaultStepLocal = new DrawStep;

		memcpy(_defaultStepLocal, _defaultStepGlobal, sizeof(DrawStep));
		step = _defaultStepLocal;
	} else {
		return parserError("<default> key out of scope. Must be inside <drawdata> or <render_info> keys.");
	}

	return parseDrawStep(defNode, step, false);
}

bool ThemeParser::parserCallback_text() {
	ParserNode *tNode = getActiveNode();
	ParserNode *parentNode = getParentNode(tNode);
	
	if (parentNode == 0 || parentNode->name != "drawdata")
		return parserError("Text Steps must be contained inside <drawdata> keys.");
		
	Graphics::TextStep step;
	
	if (tNode->values.contains("horizontal_align") == false || tNode->values.contains("vertical_align") == false)
		return parserError("Text inside widgets requires proper alignment keys.");
		
	if (tNode->values["horizontal_align"] == "left")
		step.alignHorizontal = GUI::Theme::kTextAlignLeft;
	else if (tNode->values["horizontal_align"] == "right")
		step.alignHorizontal = GUI::Theme::kTextAlignRight;
	else if (tNode->values["horizontal_align"] == "center")
		step.alignHorizontal = GUI::Theme::kTextAlignCenter;
	else return parserError("Invalid value for text alignment.");
	
	if (tNode->values["vertical_align"] == "top")
		step.alignVertical = GUI::Theme::kTextAlignVTop;
	else if (tNode->values["vertical_align"] == "center")
		step.alignVertical = GUI::Theme::kTextAlignVCenter;
	else if (tNode->values["vertical_align"] == "bottom")
		step.alignVertical = GUI::Theme::kTextAlignVBottom;
	else return parserError("Invalid value for text alignment.");
	
	Common::String paletteColor = "text_default";
	int red, green, blue;
	
	if (tNode->name.contains("hover"))
		paletteColor = "text_hover";
	
	if (tNode->name.contains("disabled"))
		paletteColor = "text_disabled";
	
	if (tNode->values.contains("color")) {

		if (_palette.contains(tNode->values["color"]))
			getPaletteColor(tNode->values["color"], red, green, blue);
		else if (!parseIntegerKey(tNode->values["color"].c_str(), 3, &red, &green, &blue))
			return parserError("Error when parsing color value for text definition");		
			
	} else if (_palette.contains(paletteColor)) {
		getPaletteColor(paletteColor, red, green, blue);
	} else {
		return parserError("Cannot assign color for text drawing.");
	}
	
	step.color.r = red;
	step.color.g = green;
	step.color.b = blue;
	step.color.set = true;

	_theme->addTextStep(parentNode->values["id"], step);
	return true;
}

bool ThemeParser::parserCallback_renderInfo() {
	ParserNode *infoNode = getActiveNode();

	assert(infoNode->name == "render_info");

	if (getParentNode(infoNode) != 0)
		return parserError("<render_info> keys must be root elements.");

	// TODO: Skip key if it's not for this platform.

	return true;
}

bool ThemeParser::parserCallback_layoutInfo() {
	ParserNode *layoutNode = getActiveNode();

	assert(layoutNode->name == "layout_info");

	if (getParentNode(layoutNode) != 0)
		return parserError("<layout_info> keys must be root elements.");

	return true;
}

bool ThemeParser::parserCallback_palette() {
	ParserNode *paletteNode = getActiveNode();

	assert(paletteNode->name == "palette");

	if (getParentNode(paletteNode) == 0 || getParentNode(paletteNode)->name != "render_info")
		return parserError("Palette keys must be contained inside a <render_info> section.");

	return true;
}

bool ThemeParser::parserCallback_color() {
	ParserNode *colorNode = getActiveNode();

	if (getParentNode(colorNode) == 0 || getParentNode(colorNode)->name != "palette")
		return parserError("Colors must be specified inside <palette> tags.");

	if (!colorNode->values.contains("name") || !colorNode->values.contains("rgb"))
		return parserError("Color keys must contain 'name' and 'rgb' values for the color.");

	Common::String name = colorNode->values["name"];

	if (_palette.contains(name))
		return parserError("Color '%s' has already been defined.", name.c_str());

	int red, green, blue;

	if (parseIntegerKey(colorNode->values["rgb"].c_str(), 3, &red, &green, &blue) == false ||
		red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
		return parserError("Error when parsing RGB values for palette color '%s'", name.c_str());\

	_palette[name].r = red;
	_palette[name].g = green;
	_palette[name].b = blue;

	return true;
}


bool ThemeParser::parserCallback_DRAWSTEP() {
	ParserNode *stepNode = _activeKey.top();
	ParserNode *drawdataNode = getParentNode(stepNode);

	if (!drawdataNode || drawdataNode->name != "drawdata")
		return parserError("DrawStep keys must be located inside a DrawData set.");

	assert(stepNode->name == "drawstep");
	assert(drawdataNode->values.contains("id"));

	Graphics::DrawStep *drawstep = newDrawStep();

	if (!stepNode->values.contains("func"))
		return parserError("All Draw Steps must contain a 'func' definition.");

	Common::String functionName = stepNode->values["func"]; 

	if (_drawFunctions.contains(functionName) == false)
		return parserError("%s is not a valid drawing function name", functionName.c_str());

	drawstep->drawingCall = _drawFunctions[functionName];

	if (!parseDrawStep(stepNode, drawstep, true))
		return false;

	_theme->addDrawStep(drawdataNode->values["id"], *drawstep);
	delete drawstep;

	return true;
}

bool ThemeParser::parserCallback_DRAWDATA() {
	ParserNode *drawdataNode = _activeKey.top();
	bool cached = false;

	assert(drawdataNode->name == "drawdata");

	if (getParentNode(drawdataNode) == 0 || getParentNode(drawdataNode)->name != "render_info")
		return parserError("DrawData keys must be contained inside a <render_info> section.");

	if (drawdataNode->values.contains("id") == false)
		return parserError("DrawData keys must contain an identifier.");

	ThemeRenderer::DrawData id = _theme->getDrawDataId(drawdataNode->values["id"]);

	if (id == -1)
		return parserError("%s is not a valid DrawData set identifier.", drawdataNode->values["id"].c_str());

	if (drawdataNode->values.contains("cache")) {
		if (drawdataNode->values["cache"] == "true") 
			cached = true;
		else if (drawdataNode->values["cache"] == "false")
			cached = false;
		else return parserError("'Parsed' value must be either true or false.");
	}

	// Both Max and Johannes suggest using a non-platform specfic approach based on available
	// resources and active resolution. getHostPlatformString() has been removed, so fix this.

/*	if (drawdataNode->values.contains("platform")) {
		if (drawdataNode->values["platform"].compareToIgnoreCase(Common::getHostPlatformString()) != 0) {
			drawdataNode->ignore = true;
			return true;
		}
	}*/

	if (_theme->addDrawData(id, cached) == false)
		return parserError("Repeated DrawData: Only one set of Drawing Data for a widget may be specified on each platform.");

	if (_defaultStepLocal) {
		delete _defaultStepLocal;
		_defaultStepLocal = 0;
	}

	return true;
}

bool ThemeParser::parseDrawStep(ParserNode *stepNode, Graphics::DrawStep *drawstep, bool functionSpecific) {
	int red, green, blue, x;
	Common::String val;

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
		if (!parseIntegerKey(stepNode->values[key_name].c_str(), 1, &x)) \
			return parserError("Error when parsing key value for '%s'.", key_name); \
		\
		drawstep->struct_name = x; \
	} else if (force) { \
		return parserError("Missing necessary key '%s'.", key_name); \
	}

/**
 * Helper macro to sanitize and assign a RGB value from a key to the draw
 * step. RGB values have the following syntax: "R, G, B".
 *
 * @param struct_name Name of the field of a DrawStep struct that must be
 *                    assigned.
 * @param key_name Name as STRING of the key identifier as it appears in the
 *                 theme description format.
 */
#define __PARSER_ASSIGN_RGB(struct_name, key_name) \
	if (stepNode->values.contains(key_name)) { \
		val = stepNode->values[key_name]; \
		if (_palette.contains(val)) { \
			red = _palette[val].r; \
			green = _palette[val].g; \
			blue = _palette[val].b; \
		} else if (parseIntegerKey(val.c_str(), 3, &red, &green, &blue) == false || \
			red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) \
			return parserError("Error when parsing color struct '%s'", val.c_str());\
		\
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

	if (functionSpecific) {
		assert(stepNode->values.contains("func"));
		Common::String functionName = stepNode->values["func"];

		if (functionName == "roundedsq" || functionName == "circle") {
			if (stepNode->values.contains("radius") && stepNode->values["radius"] == "auto") {
				drawstep->radius = 0xFF;
			} else {
				__PARSER_ASSIGN_INT(radius, "radius", true);
			}
		}

		if (functionName == "bevelsq") {
			__PARSER_ASSIGN_INT(extraData, "bevel", true);
		}

		if (functionName == "triangle") {
			drawstep->extraData = VectorRenderer::kTriangleUp;

			if (stepNode->values.contains("orientation")) {
				val = stepNode->values["orientation"];

				if ( val == "top")
					drawstep->extraData = VectorRenderer::kTriangleUp;
				else if (val == "bottom")
					drawstep->extraData = VectorRenderer::kTriangleDown;
				else if (val == "left")
					drawstep->extraData = VectorRenderer::kTriangleLeft;
				else if (val == "right")
					drawstep->extraData = VectorRenderer::kTriangleRight;
				else
					return parserError("'%s' is not a valid value for triangle orientation.", val.c_str());
			}
		}

		if (stepNode->values.contains("size")) {
			warning("The <size> keyword has been deprecated. Use <width> and <height> instead");
		}
		
		if (stepNode->values.contains("width") && stepNode->values["width"] != "auto") {
			drawstep->autoWidth = false;
			
			val = stepNode->values["width"];
			if (parseIntegerKey(val.c_str(), 1, &x))
				drawstep->w = x;
			else if (val == "height")
				drawstep->w = -1;
			else return parserError("Invalid value for vector width.");
			
			if (stepNode->values.contains("xpos")) {
				val = stepNode->values["xpos"];
				
				if (parseIntegerKey(val.c_str(), 1, &x))
					drawstep->x = x;
				else if (val == "center")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignCenter;
				else if (val == "left")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignLeft;
				else if (val == "right")
					drawstep->xAlign = Graphics::DrawStep::kVectorAlignRight;
				else 
					return parserError("Invalid value for X Position");
			} else {
				return parserError("When width is not set to 'auto', a <xpos> tag must be included.");
			}
		}
		
		if (stepNode->values.contains("height") && stepNode->values["height"] != "auto") {
			drawstep->autoHeight = false;
			
			val = stepNode->values["height"];
			if (parseIntegerKey(val.c_str(), 1, &x))
				drawstep->h = x;
			else if (val == "width")
				drawstep->h = -1;
			else return parserError("Invalid value for vector height.");

			if (stepNode->values.contains("ypos")) {
				val = stepNode->values["ypos"];
				
				if (parseIntegerKey(val.c_str(), 1, &x))
					drawstep->y = x;
				else if (val == "center")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignCenter;
				else if (val == "top")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignTop;
				else if (val == "bottom")
					drawstep->yAlign = Graphics::DrawStep::kVectorAlignBottom;
				else 
					return parserError("Invalid value for Y Position");
			} else {
				return parserError("When height is not set to 'auto', a <ypos> tag must be included.");
			}
		}
		
		if (drawstep->h == -1 && drawstep->w == -1)
			return parserError("Cross-reference in Vector Size: Height is set to width and width is set to height.");
	}

	if (stepNode->values.contains("fill")) {
		val = stepNode->values["fill"];
		if (val == "none")
			drawstep->fillMode = VectorRenderer::kFillDisabled;
		else if (val == "foreground")
			drawstep->fillMode = VectorRenderer::kFillForeground;
		else if (val == "background")
			drawstep->fillMode = VectorRenderer::kFillBackground;
		else if (val == "gradient")
			drawstep->fillMode = VectorRenderer::kFillGradient;
		else
			return parserError("'%s' is not a valid fill mode for a shape.", stepNode->values["fill"].c_str());
	}

#undef __PARSER_ASSIGN_INT
#undef __PARSER_ASSIGN_RGB

	return true;
}

}

