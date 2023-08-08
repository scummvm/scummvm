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

#ifndef CRAB_ELEMENT_H
#define CRAB_ELEMENT_H

#include "crab/image/ImageManager.h"
#include "crab/Rectangle.h"
#include "crab/vectors.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Element : public Rect {
	// The position loaded directly from xml
	Vector2i _raw;

	// Which side of the screen is this object aligned to?
	struct {
		Align x, y;
	} _align;

	void basicload(rapidxml::xml_node<char> *node, const bool &echo = true);

public:
	Element() {
		_align.x = ALIGN_LEFT;
		_align.y = ALIGN_LEFT;
	}
	~Element() {}

	// Initialize an element without loading it from file
	void init(const int &x, const int &y, const Align &alignX, const Align &alignY,
			  const ImageKey image = 0, const int &w = 0, const int &h = 0);

	// Initialize an element from another
	void init(const Element &e, ImageKey img = 0, const int &xOffset = 0, const int &yOffset = 0) {
		_raw.x = e._raw.x + xOffset;
		_raw.y = e._raw.y + yOffset;
		init(e.x + xOffset, e.y + yOffset, e._align.x, e._align.y, img, e.w, e.h);
	}

	// The parent is the object inside which the element exists
	void load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo = true);

	// The parent is the object inside which the element exists
	void load(rapidxml::xml_node<char> *node, Rect *parent = nullptr, const bool &echo = true);

	void setUI(Rect *parent = nullptr);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ELEMENT_H
