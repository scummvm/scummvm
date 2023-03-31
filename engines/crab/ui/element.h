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
#include "crab/ScreenSettings.h"
#include "crab/common_header.h"
#include "crab/vectors.h"
#include "crab/Rectangle.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Element : public Rect {
	// The position loaded directly from xml
	Vector2i raw;

	// Which side of the screen is this object aligned to?
	struct {
		Align x, y;
	} align;

	void BasicLoad(rapidxml::xml_node<char> *node, const bool &echo = true);

public:
	Element() {
		align.x = ALIGN_LEFT;
		align.y = ALIGN_LEFT;
	}
	~Element() {}

	// Initialize an element without loading it from file
	void Init(const int &X, const int &Y, const Align &align_x, const Align &align_y,
			  const ImageKey image = 0, const int &W = 0, const int &H = 0);

	// Initialize an element from another
	void Init(const Element &e, ImageKey img = 0, const int &XOffset = 0, const int &YOffset = 0) {
		raw.x = e.raw.x + XOffset;
		raw.y = e.raw.y + YOffset;
		Init(e.x + XOffset, e.y + YOffset, e.align.x, e.align.y, img, e.w, e.h);
	}

	// The parent is the object inside which the element exists
	void Load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo = true);

	// The parent is the object inside which the element exists
	void Load(rapidxml::xml_node<char> *node, Rect *parent = NULL, const bool &echo = true);

	void SetUI(Rect *parent = NULL);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ELEMENT_H
