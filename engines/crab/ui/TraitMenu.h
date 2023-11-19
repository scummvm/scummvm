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

#ifndef CRAB_TRAITMENU_H
#define CRAB_TRAITMENU_H

#include "crab/people/person.h"
#include "crab/ui/menu.h"
#include "crab/ui/ParagraphData.h"
#include "crab/ui/TraitButton.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class TraitMenu {
	// The menu for displaying all the traits
	Menu<TraitButton> _menu;

	// The reference button (from which all buttons are initialized)
	TraitButton _ref;

	// This vector stores the increments in x,y for each new button
	Vector2i _inc;

	// How to draw the selected trait description
	ParagraphData _desc;

	// The selected trait, and size of the menu
	int _select;

	// The size and dimensions of the menu
	uint _size, _rows, _cols;

public:
	TraitMenu() {
		_select = -1;
		_size = 1;
		_rows = 1;
		_cols = 1;
	}

	~TraitMenu() {}

	void reset() {
		_select = -1;
	}

	void clear();

	void load(rapidxml::xml_node<char> *node);
	void draw(const pyrodactyl::people::Person *obj);

	void handleEvents(pyrodactyl::people::Person *obj, const Common::Event &event);

	void cache(const pyrodactyl::people::Person &obj);
	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TRAITMENU_H
