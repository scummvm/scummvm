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

#ifndef CRAB_REPLYBUTTON_H
#define CRAB_REPLYBUTTON_H

#include "crab/ui/button.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
// This button is used to draw multiple lines of text instead of an image
class ReplyButton : public Button {
	// Information about drawing reply options
	int _colB, _colS, _colH;
	FontKey _font;
	Align _replyAlign;
	Vector2D<uint> _lineSize;

	// Reply options get moved around a lot, this remembers their actual position
	Rect _orig;

	// The text for this button
	Common::String _text;

public:
	// The object it points to
	int _index;

	ReplyButton() {
		_index = 0;
		_colB = 0;
		_colS = 0;
		_colH = 0;
		_font = 0;
		_replyAlign = ALIGN_LEFT;
	}
	~ReplyButton() {}

	void load(rapidxml::xml_node<char> *node);
	void draw(const int &xOffset = 0, const int &yOffset = 0);

	// Used to calculate size and set the string
	// Spacing is the minimum space between buttons added in case of overflow
	// Bottom edge is the y+h value of the previous choice
	void cache(const Common::String &val, const int &spacing, const int &bottomEdge, Rect *parent);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_REPLYBUTTON_H
