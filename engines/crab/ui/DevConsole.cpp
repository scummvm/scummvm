/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/XMLDoc.h"
#include "crab/ui/DevConsole.h"

namespace Crab {

using namespace pyrodactyl::ui;

void DebugConsole::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("debug");
		if (nodeValid(node)) {
			if (nodeValid("menu", node))
				_menu.load(node->first_node("menu"));

			if (nodeValid("variable", node)) {
				rapidxml::xml_node<char> *varnode = node->first_node("variable");

				if (nodeValid("bg", varnode))
					_bg.load(varnode->first_node("bg"));

				if (nodeValid("check", varnode))
					_check.load(varnode->first_node("check"));

				if (nodeValid("back", varnode))
					_back.load(varnode->first_node("back"));

				if (nodeValid("value", varnode))
					_value.load(varnode->first_node("value"));

				if (nodeValid("text", varnode))
					_textField.load(varnode->first_node("text"));
			}
		}
	}
}

void DebugConsole::draw(pyrodactyl::event::Info &info) {
	switch (_state) {
	case STATE_NORMAL:
		_menu.draw();
		break;
	case STATE_VAR:
		_bg.draw();
		_check.draw();
		_back.draw();
		_textField.draw();

		{
			int temp = 0;
			if (info.varGet(_varName, temp))
				_value.draw(numberToString(temp));
			else
				_value.draw("Does not exist.");
		}
		break;

	default:
		break;
	}
}

void DebugConsole::handleEvents(const Common::Event &event) {
	switch (_state) {
	case STATE_NORMAL: {
		int choice = _menu.handleEvents(event);
		if (choice == 0)
			_state = STATE_VAR;
	} break;
	case STATE_VAR:
		_textField.handleEvents(event);

		if (_check.handleEvents(event))
			_varName = _textField._text;

		// Control+V pastes clipboard text into text field
		if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_v) {
			if (g_system->hasTextInClipboard()) {
				Common::U32String str = g_system->getTextFromClipboard();
				_textField._text = convertFromU32String(str).c_str();
			}
		}

		if (_back.handleEvents(event)) {
			_state = STATE_NORMAL;
		}
		break;

	default:
		break;
	}
}

void DebugConsole::internalEvents() {
}

} // End of namespace Crab
