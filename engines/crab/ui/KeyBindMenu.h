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

#ifndef CRAB_KEYBINDMENU_H
#define CRAB_KEYBINDMENU_H

#include "crab/ui/ImageData.h"
#include "crab/ui/OptionSelect.h"
#include "crab/common_header.h"
#include "crab/ui/menu.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class KeyBindMenu {
	// The keyboard controls menu has 2 types of inputs
	enum Controls { CON_GAME,
					CON_UI,
					CON_TOTAL };

	// Each menu can be in these 2 states
	enum States { STATE_NORMAL,
				  STATE_KEY } state;

	// This button swaps between sub-sections "Gameplay" and "Interface"
	OptionSelect sel_controls;

	// These two buttons are the template buttons for the menu
	Button prim, alt;

	// This is the template text info
	TextData desc;

	// inc tells us what to add to the reference buttons to get multiple buttons
	// Divide is the space between two columns
	Vector2i inc, divide;

	// The number of rows and columns
	Vector2i dim;

	// The menu for the keyboard options in both sub categories
	// all control types have equal entries so we just need to change the text displayed
	ButtonMenu menu[CON_TOTAL];

	// The selected button in the current menu
	int choice;

	struct PromptInfo {
		int col, col_prev;
		Common::String text;

		PromptInfo() {
			col = 0;
			col_prev = 0;
		}

		void Load(rapidxml::xml_node<char> *node) {
			if (nodeValid(node)) {
				loadStr(text, "text", node);
				loadNum(col, "color", node);
			}
		}

		void Swap(Caption &c) {
			col_prev = c.col;
			c.text = text;
			c.col = col;
		}
	} prompt;

	void StartAndSize(const int &type, int &start, int &size);
	void InitMenu(const int &type);
	void DrawDesc(const int &type);

public:
	KeyBindMenu() {
		Reset();
		choice = -1;
	}
	~KeyBindMenu() {}

	void Reset() { state = STATE_NORMAL; }

#if 0
	void SwapKey(const SDL_Scancode &find);
#endif
	bool DisableHotkeys() { return state != STATE_NORMAL; }

	void Load(rapidxml::xml_node<char> *node);

	void HandleEvents(const Common::Event &Event);
#if 0
	void HandleEvents(const SDL_Event &Event);
#endif
	void SetCaption();

	void Draw();
	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_KEYBINDMENU_H
