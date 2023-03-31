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

void DebugConsole::Load(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("debug");
		if (NodeValid(node)) {
			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));

			if (NodeValid("variable", node)) {
				rapidxml::xml_node<char> *varnode = node->first_node("variable");

				if (NodeValid("bg", varnode))
					bg.Load(varnode->first_node("bg"));

				if (NodeValid("check", varnode))
					check.Load(varnode->first_node("check"));

				if (NodeValid("back", varnode))
					back.Load(varnode->first_node("back"));

				if (NodeValid("value", varnode))
					value.Load(varnode->first_node("value"));

				if (NodeValid("text", varnode))
					text_field.Load(varnode->first_node("text"));
			}
		}
	}
}

void DebugConsole::Draw(pyrodactyl::event::Info &info) {
	switch (state) {
	case STATE_NORMAL:
		menu.Draw();
		break;
	case STATE_VAR:
		bg.Draw();
		check.Draw();
		back.Draw();
		text_field.Draw();

		{
			int temp = 0;
			if (info.VarGet(var_name, temp))
				value.Draw(NumberToString(temp).c_str());
			else
				value.Draw("Does not exist.");
		}
		break;

	default:
		break;
	}
}

#if 0
void DebugConsole::HandleEvents(const SDL_Event &Event) {
	switch (state) {
	case STATE_NORMAL: {
		int choice = menu.HandleEvents(Event);
		if (choice == 0)
			state = STATE_VAR;
	} break;
	case STATE_VAR:
		text_field.HandleEvents(Event);

		if (check.HandleEvents(Event))
			var_name = text_field.text;

		// Control+V pastes clipboard text into text field
		if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_V && Event.key.keysym.mod & KMOD_CTRL) {
			if (SDL_HasClipboardText() == SDL_TRUE) {
				char *temp = SDL_GetClipboardText();
				text_field.text = temp;
				SDL_free(temp);
			}
		}

		if (back.HandleEvents(Event))
			state = STATE_NORMAL;
		break;

	default:
		break;
	}
}
#endif

void DebugConsole::InternalEvents() {
}

} // End of namespace Crab
