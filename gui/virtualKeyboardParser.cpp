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

#include "gui/virtualKeyboardParser.h"

#include "common/keyboard.h"
#include "graphics/imageman.h"
#include "common/util.h"

namespace GUI {

VirtualKeyboardParser::VirtualKeyboardParser(VirtualKeyboard *kbd) : XMLParser() {
	_keyboard = kbd;
	
	_callbacks["keyboard"] = &VirtualKeyboardParser::parserCallback_Keyboard;
	_callbacks["mode"]     = &VirtualKeyboardParser::parserCallback_Mode;
	_callbacks["event"]    = &VirtualKeyboardParser::parserCallback_Event;
	_callbacks["layout"]   = &VirtualKeyboardParser::parserCallback_Layout;
	_callbacks["map"]	   = &VirtualKeyboardParser::parserCallback_Map;
	_callbacks["area"]     = &VirtualKeyboardParser::parserCallback_Area;
}

bool VirtualKeyboardParser::keyCallback(Common::String keyName) {
	if (!_callbacks.contains(_activeKey.top()->name))
		return parserError("%s is not a valid key name.", keyName.c_str());

	return (this->*(_callbacks[_activeKey.top()->name]))();
}

bool VirtualKeyboardParser::parserCallback_Keyboard() {
	ParserNode *kbdNode = getActiveNode();

	assert(kbdNode->name == "keyboard");

	if (getParentNode(kbdNode) != 0)
		return parserError("Keyboard element must be root");

	if (_kbdParsed)
		return parserError("Only a single keyboard element is allowed");

	_kbdParsed = true;

	return true;
}

bool VirtualKeyboardParser::parserCallback_Mode() {
	ParserNode *modeNode = getActiveNode();

	assert(modeNode->name == "mode");

	if (getParentNode(modeNode) == 0 || getParentNode(modeNode)->name != "keyboard")
		return parserError("Mode element must be child of keyboard element");

	if (!modeNode->values.contains("name") || !modeNode->values.contains("resolutions"))
		return parserError("Mode element must contain name and resolutions attributes");

	Common::String name = modeNode->values["name"];

	if (_keyboard->_modes.contains(name))
		return parserError("Mode '%s' has already been defined", name);

	VirtualKeyboard::Mode mode;
	mode.name = name;
	_keyboard->_modes[name] = mode;
	_currentMode = &(_keyboard->_modes[name]);

	Common::String resolutions = modeNode->values["resolutions"];
	Common::StringTokenizer tok(resolutions, " ,");

	uint16 scrX = g_system->getOverlayWidth(), scrY = g_system->getOverlayHeight();
	uint16 diff = 0xFFFF;

	for (Common::String res = tok.nextToken(); res.size() > 0; res = tok.nextToken()) {
		uint16 resX, resY;
		if (sscanf(res.c_str(), "%dx%d", &resX, &resY) != 2)
			parserError("Invalid resolution specification");
		else {
			if (resX == scrX && resY == scrY) {
				_currentMode->resolution = res;
				break;
			} else if (resX < scrX && resY < scrY) {
				uint16 newDiff = (scrX - resX) + (scrY - resY);
				if (newDiff < diff) {
					diff = newDiff;
					_currentMode->resolution = res;
				}
			}
		}
	}

	if (_currentMode->resolution.empty())
		return parserError("No acceptable resolution was found");

	return true;
}

bool VirtualKeyboardParser::parserCallback_Event() {
	ParserNode *evtNode = getActiveNode();

	assert(evtNode->name == "event");

	if (getParentNode(evtNode) == 0 || getParentNode(evtNode)->name != "mode")
		return parserError("Event element must be child of mode element");

	if (!evtNode->values.contains("name") || !evtNode->values.contains("type"))
		return parserError("Event element must contain name and type attributes");

	assert(_currentMode);

	Common::String name = evtNode->values["name"];
	if (_currentMode->events.contains(name))
		return parserError("Event '%s' has already been defined", name);

	VirtualKeyboard::Event evt;
	evt.name = name;

	Common::String type = evtNode->values["type"];
	if (type == "key") {
		if (!evtNode->values.contains("code") || !evtNode->values.contains("ascii"))
			return parserError("Key event element must contain code and ascii attributes");

		evt.type = VirtualKeyboard::kEventKey;

		Common::KeyCode code = (Common::KeyCode)atoi(evtNode->values["code"].c_str());
		uint16 ascii = atoi(evtNode->values["ascii"].c_str());

		byte flags = 0;
		if (evtNode->values.contains("flags")) {
			Common::StringTokenizer tok(evtNode->values["flags"], ", ");
			for (Common::String fl = tok.nextToken(); !fl.empty(); fl = tok.nextToken()) {
				if (fl == "ctrl" || fl == "control")
					flags &= Common::KBD_CTRL;
				else if (fl == "alt")
					flags &= Common::KBD_ALT;
				else if (fl == "shift")
					flags &= Common::KBD_SHIFT;
			}
		}

		evt.data = new Common::KeyState(code, ascii, flags);

	} else if (type == "switch_mode") {
		if (!evtNode->values.contains("mode"))
			return parserError("Switch mode event element must contain mode attribute");

		evt.type = VirtualKeyboard::kEventSwitchMode;
		evt.data = new Common::String(evtNode->values["mode"]);
	} else
		return parserError("Event type '%s' not known", type);

	_currentMode->events[name] = evt;

	return true;
}

bool VirtualKeyboardParser::parserCallback_Layout() {
	ParserNode *layoutNode = getActiveNode();

	assert(layoutNode->name == "layout");

	if (getParentNode(layoutNode) == 0 || getParentNode(layoutNode)->name != "mode")
		return parserError("Layout element must be child of mode element");

	if (!layoutNode->values.contains("resolution") || !layoutNode->values.contains("bitmap"))
		return parserError("Layout element must contain resolution and bitmap attributes");

	assert(!_currentMode->resolution.empty());

	Common::String res = layoutNode->values["resolution"];

	if (res != _currentMode->resolution) {
		layoutNode->ignore = true;
		return true;
	}

	_currentMode->bitmapName = layoutNode->values["bitmap"];

	
	if (!ImageMan.registerSurface(_currentMode->bitmapName, 0))
		return parserError("Error loading bitmap '%s'", _currentMode->bitmapName.c_str());

	_currentMode->image = ImageMan.getSurface(_currentMode->bitmapName);
	if (!_currentMode->image)
		return parserError("Error loading bitmap '%s'", _currentMode->bitmapName.c_str());

	return true;
}

bool VirtualKeyboardParser::parserCallback_Map() {
	ParserNode *mapNode = getActiveNode();

	assert(mapNode->name == "map");

	if (getParentNode(mapNode) == 0 || getParentNode(mapNode)->name != "layout")
		return parserError("Map element must be child of layout element");

	return true;
}

bool VirtualKeyboardParser::parserCallback_Area() {
	ParserNode *areaNode = getActiveNode();

	assert(areaNode->name == "area");

	if (getParentNode(areaNode) == 0 || getParentNode(areaNode)->name != "map")
		return parserError("Area element must be child of map element");
	
	if (!areaNode->values.contains("shape") || !areaNode->values.contains("coords") || !areaNode->values.contains("target"))
		return parserError("Area element must contain shape, coords and target attributes");

	Common::String shape = areaNode->values["shape"];
	if (shape == "rect") {
		int16 x1, y1, x2, y2;
		if (!parseIntegerKey(areaNode->values["coords"].c_str(), 4, &x1, &y1, &x2, &y2))
			return parserError("Invalid coords for rect area");

		Common::Rect rect(x1, y1, x2, y2);
		_currentMode->imageMap.addRectMapArea(rect, areaNode->values["target"]);
	} else if (shape == "poly") {
		Common::StringTokenizer tok (areaNode->values["coords"], ", ");
		Common::Polygon poly;
		for (Common::String st = tok.nextToken(); !st.empty(); st = tok.nextToken()) {
			int16 x, y;
			if (sscanf(st.c_str(), "%d", &x) != 1)
				return parserError("Invalid coords for polygon area");
			st = tok.nextToken();
			if (sscanf(st.c_str(), "%d", &y) != 1)
				return parserError("Invalid coords for polygon area");
			poly.addPoint(x, y);
		}
		if (poly.getPointCount() < 3)
			return parserError("Invalid coords for polygon area");
		_currentMode->imageMap.addPolygonMapArea(poly, areaNode->values["target"]);
	} else
		return parserError("Area shape '%s' not known", shape);

	return true;
}

} // end of namespace GUI
