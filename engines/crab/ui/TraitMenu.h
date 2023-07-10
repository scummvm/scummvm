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

#include "crab/ui/ParagraphData.h"
#include "crab/ui/TraitButton.h"
#include "crab/common_header.h"
#include "crab/ui/menu.h"
#include "crab/people/person.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class TraitMenu {
	// The menu for displaying all the traits
	Menu<TraitButton> menu;

	// The reference button (from which all buttons are initialized)
	TraitButton ref;

	// This vector stores the increments in x,y for each new button
	Vector2i inc;

	// How to draw the selected trait description
	ParagraphData desc;

	// The selected trait, and size of the menu
	int select;

	// The size and dimensions of the menu
	unsigned int size, rows, cols;

public:
	TraitMenu() {
		select = -1;
		size = 1;
		rows = 1;
		cols = 1;
	}
	~TraitMenu() {}

	void Reset() { select = -1; }
	void Clear();

	void load(rapidxml::xml_node<char> *node);
	void draw(const pyrodactyl::people::Person *obj);

	void HandleEvents(pyrodactyl::people::Person *obj, const Common::Event &Event);
#if 0
	void HandleEvents(pyrodactyl::people::Person *obj, const SDL_Event &Event);
#endif

	void Cache(const pyrodactyl::people::Person &obj);
	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TRAITMENU_H
