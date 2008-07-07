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

#include "backends/common/virtual-keyboard-parser.h"

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

	if (!kbdNode->values.contains("initial_mode"))
		return parserError("Keyboard element must contain initial_mode attribute");

	_initialModeName = kbdNode->values["initial_mode"];

	if (kbdNode->values.contains("h_align")) {
		Common::String h = kbdNode->values["h_align"];
		if (h == "left")
			_keyboard->_hAlignment = VirtualKeyboard::kAlignLeft;
		else if (h == "centre" || h == "center")
			_keyboard->_hAlignment = VirtualKeyboard::kAlignCentre;
		else if (h == "right")
			_keyboard->_hAlignment = VirtualKeyboard::kAlignRight;
	}

	if (kbdNode->values.contains("v_align")) {
		Common::String v = kbdNode->values["h_align"];
		if (v == "top")
			_keyboard->_vAlignment = VirtualKeyboard::kAlignTop;
		else if (v == "middle" || v == "center")
			_keyboard->_vAlignment = VirtualKeyboard::kAlignMiddle;
		else if (v == "bottom")
			_keyboard->_vAlignment = VirtualKeyboard::kAlignBottom;
	}

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

	// create new mode
	VirtualKeyboard::Mode mode;
	mode.name = name;
	_keyboard->_modes[name] = mode;
	_mode = &(_keyboard->_modes[name]);
	
	// if this is the keyboard's initial mode
	// then set it to be the current mode
	if (name == _initialModeName)
		_keyboard->_initialMode = _mode;

	Common::String resolutions = modeNode->values["resolutions"];
	Common::StringTokenizer tok(resolutions, " ,");

	uint16 scrX = g_system->getOverlayWidth(), scrY = g_system->getOverlayHeight();
	uint32 diff = 0xFFFFFFFF;

	for (Common::String res = tok.nextToken(); res.size() > 0; res = tok.nextToken()) {
		int resX, resY;
		if (sscanf(res.c_str(), "%dx%d", &resX, &resY) != 2) {
			parserError("Invalid resolution specification");
		} else {
			if (resX == scrX && resY == scrY) {
				_mode->resolution = res;
				break;
			} else {
				uint16 newDiff = ABS(scrX - resX) + ABS(scrY - resY);
				if (newDiff < diff) {
					diff = newDiff;
					_mode->resolution = res;
				}
			}
		}
	}

	if (_mode->resolution.empty())
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

	assert(_mode);

	Common::String name = evtNode->values["name"];
	if (_mode->events.contains(name))
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

	_mode->events[name] = evt;

	return true;
}

bool VirtualKeyboardParser::parserCallback_Layout() {
	ParserNode *layoutNode = getActiveNode();

	assert(layoutNode->name == "layout");

	if (getParentNode(layoutNode) == 0 || getParentNode(layoutNode)->name != "mode")
		return parserError("Layout element must be child of mode element");

	if (!layoutNode->values.contains("resolution") || !layoutNode->values.contains("bitmap"))
		return parserError("Layout element must contain resolution and bitmap attributes");

	assert(!_mode->resolution.empty());

	Common::String res = layoutNode->values["resolution"];

	if (res != _mode->resolution) {
		layoutNode->ignore = true;
		return true;
	}

	_mode->bitmapName = layoutNode->values["bitmap"];

	
	if (!ImageMan.registerSurface(_mode->bitmapName, 0))
		return parserError("Error loading bitmap '%s'", _mode->bitmapName.c_str());

	_mode->image = ImageMan.getSurface(_mode->bitmapName);
	if (!_mode->image)
		return parserError("Error loading bitmap '%s'", _mode->bitmapName.c_str());

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
		int x1, y1, x2, y2;
		if (!parseIntegerKey(areaNode->values["coords"].c_str(), 4, &x1, &y1, &x2, &y2))
			return parserError("Invalid coords for rect area");

		Common::Rect rect(x1, y1, x2, y2);
		_mode->imageMap.addRectMapArea(rect, areaNode->values["target"]);
	} else if (shape == "poly") {
		Common::StringTokenizer tok (areaNode->values["coords"], ", ");
		Common::Polygon poly;
		for (Common::String st = tok.nextToken(); !st.empty(); st = tok.nextToken()) {
			int x, y;
			if (sscanf(st.c_str(), "%d", &x) != 1)
				return parserError("Invalid coords for polygon area");
			st = tok.nextToken();
			if (sscanf(st.c_str(), "%d", &y) != 1)
				return parserError("Invalid coords for polygon area");
			poly.addPoint(x, y);
		}
		if (poly.getPointCount() < 3)
			return parserError("Invalid coords for polygon area");
		_mode->imageMap.addPolygonMapArea(poly, areaNode->values["target"]);
	} else
		return parserError("Area shape '%s' not known", shape);

	return true;
}

} // end of namespace GUI
