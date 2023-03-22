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

#include "TextData.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

bool TextData::Load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo) {
	if (NodeValid(node, echo)) {
		Element::Load(node, parent, echo);
		LoadFontKey(font, "font", node);
		LoadNum(col, "color", node);
		LoadAlign(align, node, false);
		LoadBool(background, "background", node, false);
		return true;
	}

	return false;
}

void TextData::Draw(const std::string &val, const int &XOffset, const int &YOffset) {
	pyrodactyl::text::gTextManager.Draw(x + XOffset, y + YOffset, val, col, font, align, background);
}

void TextData::DrawColor(const std::string &val, const int &color, const int &XOffset, const int &YOffset) {
	pyrodactyl::text::gTextManager.Draw(x + XOffset, y + YOffset, val, color, font, align, background);
}
