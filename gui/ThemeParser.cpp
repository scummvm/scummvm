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
	
	_drawFunctions["circle"]  = &Graphics::VectorRenderer::drawCallback_CIRCLE;
	_drawFunctions["square"]  = &Graphics::VectorRenderer::drawCallback_SQUARE;
	_drawFunctions["roundedsq"]  = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	_drawFunctions["bevelsq"]  = &Graphics::VectorRenderer::drawCallback_BEVELSQ;
	_drawFunctions["line"]  = &Graphics::VectorRenderer::drawCallback_LINE;
	_drawFunctions["triangle"]  = &Graphics::VectorRenderer::drawCallback_TRIANGLE;
	_drawFunctions["fill"]  = &Graphics::VectorRenderer::drawCallback_FILLSURFACE;
	_drawFunctions["tab"]  = &Graphics::VectorRenderer::drawCallback_TAB;
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
	step->bevel = 0;
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

bool ThemeParser::parserCallback_defaults(ParserNode *node) {
	ParserNode *parentNode = getParentNode(node);
	Graphics::DrawStep *step = 0;

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

	return parseDrawStep(node, step, false);
}

bool ThemeParser::parserCallback_font(ParserNode *node) {		
	int red, green, blue;

	if (_palette.contains(node->values["color"]))
		getPaletteColor(node->values["color"], red, green, blue);
	else if (!parseIntegerKey(node->values["color"].c_str(), 3, &red, &green, &blue))
		return parserError("Error when parsing color value for font definition.");
	
	if (!_theme->addFont(node->values["id"], red, green, blue))
		return parserError("Error when loading Font in theme engine.");
		
	return true;
}

bool ThemeParser::parserCallback_fonts(ParserNode *node) {		
	return true;	
}

bool ThemeParser::parserCallback_text(ParserNode *node) {		
	GUI::Theme::TextAlign alignH;
	GUI::Theme::TextAlignVertical alignV;
		
	if (node->values["horizontal_align"] == "left")
		alignH = GUI::Theme::kTextAlignLeft;
	else if (node->values["horizontal_align"] == "right")
		alignH = GUI::Theme::kTextAlignRight;
	else if (node->values["horizontal_align"] == "center")
		alignH = GUI::Theme::kTextAlignCenter;
	else return parserError("Invalid value for text alignment.");
	
	if (node->values["vertical_align"] == "top")
		alignV = GUI::Theme::kTextAlignVTop;
	else if (node->values["vertical_align"] == "center")
		alignV = GUI::Theme::kTextAlignVCenter;
	else if (node->values["vertical_align"] == "bottom")
		alignV = GUI::Theme::kTextAlignVBottom;
	else return parserError("Invalid value for text alignment.");
	
	if (!_theme->addTextData(getParentNode(node)->values["id"], node->values["font"], alignH, alignV))
		return parserError("Error when adding Text Data for '%s'.", getParentNode(node)->values["id"].c_str());

	return true;
}

bool ThemeParser::parserCallback_render_info(ParserNode *node) {
	// TODO: Skip key if it's not for this platform.
	return true;
}

bool ThemeParser::parserCallback_layout_info(ParserNode *node) {
	// TODO: skip key
	return true;
}

bool ThemeParser::parserCallback_palette(ParserNode *node) {
	return true;
}

bool ThemeParser::parserCallback_color(ParserNode *node) {
	Common::String name = node->values["name"];

	if (_palette.contains(name))
		return parserError("Color '%s' has already been defined.", name.c_str());

	int red, green, blue;

	if (parseIntegerKey(node->values["rgb"].c_str(), 3, &red, &green, &blue) == false ||
		red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
		return parserError("Error when parsing RGB values for palette color '%s'", name.c_str());\

	_palette[name].r = red;
	_palette[name].g = green;
	_palette[name].b = blue;

	return true;
}


bool ThemeParser::parserCallback_drawstep(ParserNode *node) {
	Graphics::DrawStep *drawstep = newDrawStep();

	Common::String functionName = node->values["func"]; 

	if (_drawFunctions.contains(functionName) == false)
		return parserError("%s is not a valid drawing function name", functionName.c_str());

	drawstep->drawingCall = _drawFunctions[functionName];

	if (!parseDrawStep(node, drawstep, true))
		return false;

	_theme->addDrawStep(getParentNode(node)->values["id"], *drawstep);
	delete drawstep;

	return true;
}

bool ThemeParser::parserCallback_drawdata(ParserNode *node) {
	bool cached = false;

	if (node->values.contains("cache")) {
		if (node->values["cache"] == "true") 
			cached = true;
		else if (node->values["cache"] == "false")
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

	if (_theme->addDrawData(node->values["id"], cached) == false)
		return parserError("Error when adding Draw Data set: Invalid DrawData name.");

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
	__PARSER_ASSIGN_INT(bevel, "bevel", false);
	__PARSER_ASSIGN_INT(shadow, "shadow", false);
	__PARSER_ASSIGN_INT(factor, "gradient_factor", false);

	__PARSER_ASSIGN_RGB(fgColor, "fg_color");
	__PARSER_ASSIGN_RGB(bgColor, "bg_color");
	__PARSER_ASSIGN_RGB(gradColor1, "gradient_start");
	__PARSER_ASSIGN_RGB(gradColor2, "gradient_end");
	__PARSER_ASSIGN_RGB(bevelColor, "bevel_color");

	if (functionSpecific) {
		assert(stepNode->values.contains("func"));
		Common::String functionName = stepNode->values["func"];

		if (functionName == "roundedsq" || functionName == "circle" || functionName == "tab") {
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

bool ThemeParser::parserCallback_def(ParserNode *node) {
	Common::String var = "Globals." + node->values["var"];
	int value;
	
	if (!parseIntegerKey(node->values["value"].c_str(), 1, &value))
		return parserError("Invalid definition for '%s'.", var.c_str());
		
	_theme->themeEval()->setVar(var, value);
	return true;	
}

bool ThemeParser::parserCallback_widget(ParserNode *node) {
	Common::String var;
	
	if (getParentNode(node)->name == "globals") {
		var = "Globals." + node->values["name"] + ".";
		if (!parseCommonLayoutProps(node, var))
			return parserError("Error when parsing Layout properties of '%s'.", var.c_str());
	} else {
		var = node->values["name"];
		int width = -1;
		int height = -1;
		
		if (node->values.contains("width")) {
			if (_theme->themeEval()->hasVar(node->values["width"]) == true)
				width = _theme->themeEval()->getVar(node->values["width"]);
				
			else if (!parseIntegerKey(node->values["width"].c_str(), 1, &width))
				return parserError("Corrupted width value in key for %s", var.c_str());
		}
		
		if (node->values.contains("height")) {
			if (_theme->themeEval()->hasVar(node->values["height"]) == true)
				height = _theme->themeEval()->getVar(node->values["height"]);
				
			else if (!parseIntegerKey(node->values["height"].c_str(), 1, &height))
				return parserError("Corrupted height value in key for %s", var.c_str());
		}
		
		_theme->themeEval()->addWidget(var, width, height);
		
	}

	return true;
}

bool ThemeParser::parserCallback_child(ParserNode *node) {
	Common::String var = "Globals." + getParentNode(node)->values["name"] + "." + node->values["name"] + ".";
	
	if (!parseCommonLayoutProps(node, var))
		return parserError("Error when parsing Layout properties of '%s'.", var.c_str());
	
	return true;
}

bool ThemeParser::parserCallback_dialog(ParserNode *node) {
	Common::String var = "Dialog." + node->values["name"] + ".";
	
//	if (!parseCommonLayoutProps(node, var))
//		return parserError("Error when parsing Layout properties of '%s'.", var.c_str());
		
	_theme->themeEval()->addDialog(var);
		
	return true;
}

bool ThemeParser::parserCallback_layout(ParserNode *node) {
	
	if (!node->values.contains("type"))
		return parserError("Layouts need a specific type (vertical or horizontal).");
		
	GUI::ThemeLayout::LayoutType type = GUI::ThemeLayout::kLayoutNone;
	GUI::ThemeLayout::LayoutParsing parsing = GUI::ThemeLayout::kLayoutParseDefault;
	
	if (node->values["type"] == "vertical")
		type = GUI::ThemeLayout::kLayoutVertical;
	else if (node->values["type"] == "horizontal")
		type = GUI::ThemeLayout::kLayoutHorizontal;

	if (node->values.contains("direction")) {
		if (node->values["direction"] == "right2left")
			parsing = GUI::ThemeLayout::kLayoutParseRight2Left;
	}
		
	_theme->themeEval()->addLayout(type, parsing);
	return true;
}

bool ThemeParser::closedKeyCallback(ParserNode *node) {
	if (node->name == "layout")
		_theme->themeEval()->closeLayout();
	else if (node->name == "dialog")
		_theme->themeEval()->closeDialog();
		
	return true;
}

bool ThemeParser::parseCommonLayoutProps(ParserNode *node, const Common::String &var) {
	if (node->values.contains("size")) {
		int width, height;
		
		if (!parseIntegerKey(node->values["size"].c_str(), 2, &width, &height)) {
			Common::StringTokenizer tokenizer(node->values["size"], " ,");
			Common::String wtoken, htoken;
			char *parseEnd;
			
			wtoken = tokenizer.nextToken();
			
			if (_theme->themeEval()->hasVar(wtoken)) {
				width = _theme->themeEval()->getVar(wtoken);
			} else {
				width = strtol(wtoken.c_str(), &parseEnd, 10);
				
				if (*parseEnd != 0 && !(*parseEnd == '%' && *(parseEnd + 1) == 0))
					return false;
				
				if (wtoken.lastChar() == '%')
					width = g_system->getOverlayWidth() * width / 100;
			}
			
			htoken = tokenizer.nextToken();
			
			if (_theme->themeEval()->hasVar(htoken)) {
				height = _theme->themeEval()->getVar(htoken);
			} else {
				height = strtol(htoken.c_str(), &parseEnd, 10);
				
				if (*parseEnd != 0 && !(*parseEnd == '%' && *(parseEnd + 1) == 0))
					return false;
				
				if (htoken.lastChar() == '%')
					height = g_system->getOverlayHeight() * height / 100;
			}
			
			if (!tokenizer.empty())
				return false;
		}
			
		
		_theme->themeEval()->setVar(var + "Width", width);
		_theme->themeEval()->setVar(var + "Height", height);
	}
	
	if (node->values.contains("pos")) {
		int x, y;
		
		if (!parseIntegerKey(node->values["pos"].c_str(), 2, &x, &y)) {
			Common::StringTokenizer tokenizer(node->values["pos"], " ,");
			Common::String xpos, ypos;
			char *parseEnd;
			
			xpos = tokenizer.nextToken();
			
			if (xpos == "center") {
				if (!_theme->themeEval()->hasVar(var + "Width"))
					return false;
					
				x = (g_system->getOverlayWidth() / 2) - (_theme->themeEval()->getVar(var + "Width") / 2);
				
			} else if (_theme->themeEval()->hasVar(xpos)) {
				x = _theme->themeEval()->getVar(xpos);
			} else {
				x = strtol(xpos.c_str(), &parseEnd, 10);
				
				if (*parseEnd != 0 && !(*parseEnd == 'r' && *(parseEnd + 1) == 0))
					return false;
				
				if (xpos.lastChar() == 'r')
					x = g_system->getOverlayWidth() - x;
			}	
			
			ypos = tokenizer.nextToken();
			
			if (ypos == "center") {
				if (!_theme->themeEval()->hasVar(var + "Height"))
					return false;
					
				y = (g_system->getOverlayHeight() / 2) - (_theme->themeEval()->getVar(var + "Height") / 2);
				
			} else if (_theme->themeEval()->hasVar(ypos)) {
				y = _theme->themeEval()->getVar(ypos);
			} else {
				y = strtol(ypos.c_str(), &parseEnd, 10);
				
				if (*parseEnd != 0 && !(*parseEnd == 'b' && *(parseEnd + 1) == 0))
					return false;
				
				if (ypos.lastChar() == 'b')
					y = g_system->getOverlayHeight() - y;
			}
			
			if (!tokenizer.empty())
				return false;
		}
		
		_theme->themeEval()->setVar(var + "X", x);
		_theme->themeEval()->setVar(var + "Y", y);
	}
	
	if (node->values.contains("padding")) {
		int paddingL, paddingR, paddingT, paddingB;
		
		if (!parseIntegerKey(node->values["padding"].c_str(), 4, &paddingL, &paddingR, &paddingT, &paddingB))
			return false;
		
		_theme->themeEval()->setVar(var + "Padding.Left", paddingL);
		_theme->themeEval()->setVar(var + "Padding.Right", paddingR);
		_theme->themeEval()->setVar(var + "Padding.Top", paddingT);
		_theme->themeEval()->setVar(var + "Padding.Bottom", paddingB);
	}
	
	return true;
}

}
