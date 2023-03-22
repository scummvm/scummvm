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

#ifndef CRAB_RADIOBUTTONMENU_H
#define CRAB_RADIOBUTTONMENU_H

#include "RadioButton.h"
#include "menu.h"

namespace pyrodactyl {
namespace ui {
class RadioButtonMenu : public Menu<RadioButton> {
	// The description of the menu
	HoverInfo desc;

	// The selected radio button
	int select;

public:
	RadioButtonMenu() { select = 0; }
	~RadioButtonMenu() {}

	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid("desc", node))
			desc.Load(node->first_node("desc"));

		if (NodeValid("menu", node))
			Menu::Load(node->first_node("menu"));
	}

	void Draw(const int &XOffset = 0, const int &YOffset = 0) {
		desc.Draw(XOffset, YOffset);
		Menu::Draw(XOffset, YOffset);
	}

	int HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0) {
		int result = Menu::HandleEvents(Event, XOffset, YOffset);

		if (result >= 0) {
			select = result;

			for (int i = 0; i < element.size(); ++i)
				element.at(i).state = (i == result);
		}

		return result;
	}

	void SetUI() {
		Menu::SetUI();
		desc.SetUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_RADIOBUTTONMENU_H
