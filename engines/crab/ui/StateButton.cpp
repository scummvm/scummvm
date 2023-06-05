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
#include "crab/ui/StateButton.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void StateButton::Init(const StateButton &ref, const int &XOffset, const int &YOffset) {
	Button::Init(ref, XOffset, YOffset);
	img_set = ref.img_set;
	col_normal = ref.col_normal;
	col_select = ref.col_select;
}

void StateButton::Load(rapidxml::xml_node<char> *node, const bool &echo) {
	Button::Load(node, echo);

	img_set.normal = img;
	col_normal.col = caption.col;
	col_normal.col_s = caption.col_s;

	if (NodeValid("select", node, false)) {
		rapidxml::xml_node<char> *selnode = node->first_node("select");

		img_set.select.Load(selnode, echo);
		LoadNum(col_select.col, "color", selnode);
		LoadNum(col_select.col_s, "color_s", selnode);
	} else {
		img_set.select = img;
		col_select.col = caption.col;
		col_select.col_s = caption.col_s;
	}
}

void StateButton::State(const bool val) {
	if (val) {
		img = img_set.select;
		caption.col = col_select.col;
		caption.col_s = col_select.col_s;
	} else {
		img = img_set.normal;
		caption.col = col_normal.col;
		caption.col_s = col_normal.col_s;
	}

	// Images might be different in size
	w = g_engine->_imageManager->GetTexture(img.normal).W();
	h = g_engine->_imageManager->GetTexture(img.normal).H();
}

void StateButton::Img(const StateButtonImage &sbi) {
	// Find which is the current image and set it
	if (img == img_set.normal)
		img = sbi.normal;
	else
		img = sbi.select;

	img_set = sbi;
}

} // End of namespace Crab
