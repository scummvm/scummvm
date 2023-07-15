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

#ifndef CRAB_TRAITBUTTON_H
#define CRAB_TRAITBUTTON_H

#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/people/trait.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class TraitButton : public StateButton {
	// The offset for drawing the trait image
	Vector2i _offset;

	// The trait image
	ImageKey _traitImg;

public:
	TraitButton() {
		_traitImg = 0;
	}

	~TraitButton() {}

	void init(const TraitButton &ref, const int &xOffset = 0, const int &yOffset = 0);
	void load(rapidxml::xml_node<char> *node, const bool &echo = true);

	void draw(const int &xOffset = 0, const int &yOffset = 0, Rect *clip = NULL);

	void cache(const pyrodactyl::people::Trait &trait);
	void empty();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TRAITBUTTON_H
