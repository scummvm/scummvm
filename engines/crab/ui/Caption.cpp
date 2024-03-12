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

void Caption::init(const Caption &c, const int &xOffset, const int &yOffset) {
	*this = c;
	x += xOffset;
	y += yOffset;
}

void Caption::load(rapidxml::xml_node<char> *node, Rect *parent) {
	if (TextData::load(node, parent, false)) {
		loadStr(_text, "text", node);
		loadNum(_colS, "color_s", node, false);
		_enabled = true;
	}
}

void Caption::draw(bool selected, const int &xOffset, const int &yOffset) {
	if (_enabled) {
		if (selected)
			TextData::drawColor(_text, _colS, xOffset, yOffset);
		else
			TextData::draw(_text, xOffset, yOffset);
	}
}

} // End of namespace Crab
