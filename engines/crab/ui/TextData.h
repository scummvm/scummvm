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

#ifndef CRAB_TEXTDATA_H
#define CRAB_TEXTDATA_H

#include "crab/text/TextManager.h"
#include "crab/common_header.h"
#include "crab/ui/element.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class TextData : public Element {
public:
	int _col;
	FontKey _font;
	Align _align;
	bool _background;

	TextData(void) {
		_col = 0;
		_font = 0;
		_align = ALIGN_LEFT;
		_background = false;
	}
	~TextData(void) {}

	bool load(rapidxml::xml_node<char> *node, Rect *parent = nullptr, const bool &echo = true);

	// Plain drawing
	void draw(const Common::String &val, const int &xOffset = 0, const int &yOffset = 0);

	// Draw with a different color
	void drawColor(const Common::String &val, const int &color, const int &xOffset = 0, const int &yOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTDATA_H
