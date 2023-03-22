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

#include "crab/TextManager.h"
#include "crab/common_header.h"
#include "crab/element.h"

namespace pyrodactyl {
namespace ui {
class TextData : public Element {
public:
	int col;
	FontKey font;
	Align align;
	bool background;

	TextData(void) {
		col = 0;
		font = 0;
		align = ALIGN_LEFT;
		background = false;
	}
	~TextData(void) {}

	bool Load(rapidxml::xml_node<char> *node, Rect *parent = NULL, const bool &echo = true);

	// Plain drawing
	void Draw(const std::string &val, const int &XOffset = 0, const int &YOffset = 0);

	// Draw with a different color
	void DrawColor(const std::string &val, const int &color, const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_TEXTDATA_H
