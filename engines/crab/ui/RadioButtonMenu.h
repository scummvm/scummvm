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

#include "crab/ui/RadioButton.h"
#include "crab/ui/menu.h"

namespace Crab {

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

	void load(rapidxml::xml_node<char> *node) {
		if (nodeValid("desc", node))
			desc.load(node->first_node("desc"));

		if (nodeValid("menu", node))
			Menu::load(node->first_node("menu"));
	}

	void draw(const int &XOffset = 0, const int &YOffset = 0) {
		desc.draw(XOffset, YOffset);
		Menu::draw(XOffset, YOffset);
	}

	int handleEvents(const Common::Event &Event, const int &XOffset = 0, const int &YOffset = 0) {
		int result = Menu::handleEvents(Event, XOffset, YOffset);

		if (result >= 0) {
			select = result;

			for (int i = 0; i < (int)element.size(); ++i)
				element[i]._state = (i == result);
		}

		return result;
	}

#if 0
	int handleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0) {
		int result = Menu::handleEvents(Event, XOffset, YOffset);

		if (result >= 0) {
			select = result;

			for (int i = 0; i < element.size(); ++i)
				element[i].state = (i == result);
		}

		return result;
	}
#endif

	void setUI() {
		Menu::setUI();
		desc.setUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_RADIOBUTTONMENU_H
