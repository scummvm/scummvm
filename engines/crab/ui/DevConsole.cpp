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

#include "crab/ui/DevConsole.h"

namespace Crab {

using namespace pyrodactyl::ui;

void DebugConsole::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("debug");
		if (nodeValid(node)) {
			if (nodeValid("menu", node))
				menu.load(node->first_node("menu"));

			if (nodeValid("variable", node)) {
				rapidxml::xml_node<char> *varnode = node->first_node("variable");

				if (nodeValid("bg", varnode))
					bg.load(varnode->first_node("bg"));

				if (nodeValid("check", varnode))
					check.load(varnode->first_node("check"));

				if (nodeValid("back", varnode))
					back.load(varnode->first_node("back"));

				if (nodeValid("value", varnode))
					value.load(varnode->first_node("value"));

				if (nodeValid("text", varnode))
					text_field.load(varnode->first_node("text"));
			}
		}
	}
}

void DebugConsole::draw(pyrodactyl::event::Info &info) {
	switch (state) {
	case STATE_NORMAL:
		menu.draw();
		break;
	case STATE_VAR:
		bg.draw();
		check.draw();
		back.draw();
		text_field.draw();

		{
			int temp = 0;
			if (info.varGet(var_name, temp))
				value.draw(NumberToString(temp));
			else
				value.draw("Does not exist.");
		}
		break;

	default:
		break;
	}
}

#if 0
void DebugConsole::handleEvents(const SDL_Event &Event) {
	switch (state) {
	case STATE_NORMAL: {
		int choice = menu.handleEvents(Event);
		if (choice == 0)
			state = STATE_VAR;
	} break;
	case STATE_VAR:
		text_field.handleEvents(Event);

		if (check.handleEvents(Event))
			var_name = text_field.text;

		// Control+V pastes clipboard text into text field
		if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_V && Event.key.keysym.mod & KMOD_CTRL) {
			if (SDL_HasClipboardText() == SDL_TRUE) {
				char *temp = SDL_GetClipboardText();
				text_field.text = temp;
				SDL_free(temp);
			}
		}

		if (back.handleEvents(Event))
			state = STATE_NORMAL;
		break;

	default:
		break;
	}
}
#endif

void DebugConsole::internalEvents() {
}

} // End of namespace Crab
