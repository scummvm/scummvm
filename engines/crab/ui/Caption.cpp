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


#include "crab/ui/Caption.h"

namespace Crab {

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void Caption::Init(const Caption &c, const int &XOffset, const int &YOffset) {
	*this = c;
	x += XOffset;
	y += YOffset;
}

void Caption::Load(rapidxml::xml_node<char> *node, Rect *parent) {
	if (TextData::Load(node, parent, false)) {
		loadStr(text, "text", node);
		loadNum(col_s, "color_s", node, false);
		enabled = true;
	}
}

void Caption::Draw(bool selected, const int &XOffset, const int &YOffset) {
	if (enabled) {
		if (selected)
			TextData::DrawColor(text, col_s, XOffset, YOffset);
		else
			TextData::Draw(text, XOffset, YOffset);
	}
}

} // End of namespace Crab
