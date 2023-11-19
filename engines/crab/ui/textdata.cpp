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

#include "crab/crab.h"
#include "crab/ui/TextData.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

bool TextData::load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo) {
	if (nodeValid(node, echo)) {
		Element::load(node, parent, echo);
		LoadFontKey(_font, "font", node);
		loadNum(_col, "color", node);
		loadAlign(_align, node, false);
		loadBool(_background, "background", node, false);
		return true;
	}

	return false;
}

void TextData::draw(const Common::String &val, const int &xOffset, const int &yOffset) {
	g_engine->_textManager->draw(x + xOffset, y + yOffset, val, _col, _font, _align, _background);
}

void TextData::drawColor(const Common::String &val, const int &color, const int &xOffset, const int &yOffset) {
	g_engine->_textManager->draw(x + xOffset, y + yOffset, val, color, _font, _align, _background);
}

} // End of namespace Crab
