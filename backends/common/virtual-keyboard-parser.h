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

#ifndef COMMON_VIRTUAL_KEYBOARD_PARSER
#define COMMON_VIRTUAL_KEYBOARD_PARSER

#include "common/xmlparser.h"
#include "backends/common/virtual-keyboard.h"

namespace Common {

class VirtualKeyboardParser : public Common::XMLParser {

	typedef bool (VirtualKeyboardParser::*ParserCallback)();

public:

	VirtualKeyboardParser(VirtualKeyboard *kbd);

protected:
	VirtualKeyboard *_keyboard;

	/** internal state variables of parser */
	VirtualKeyboard::Mode *_mode; // pointer to mode currently being parsed
	bool _modeParsed;
	Common::String _initialModeName;  // name of initial keyboard mode
	bool _kbdParsed;

	bool keyCallback(Common::String keyName);
	void cleanup() {
		_mode = 0;
		_kbdParsed = _modeParsed = false;
		_initialModeName.clear();
	}

	bool parserCallback_Keyboard();
	bool parserCallback_Mode();
	bool parserCallback_Event();
	bool parserCallback_Layout();
	bool parserCallback_Map();
	bool parserCallback_Area();

	Common::HashMap<Common::String, ParserCallback, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _callbacks;
};

} // end of namespace GUI

#endif
