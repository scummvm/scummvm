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
	Button::init(ref, XOffset, YOffset);
	img_set = ref.img_set;
	col_normal = ref.col_normal;
	col_select = ref.col_select;
}

void StateButton::load(rapidxml::xml_node<char> *node, const bool &echo) {
	Button::load(node, echo);

	img_set.normal = _img;
	col_normal.col = _caption.col;
	col_normal.col_s = _caption.col_s;

	if (nodeValid("select", node, false)) {
		rapidxml::xml_node<char> *selnode = node->first_node("select");

		img_set.select.load(selnode, echo);
		loadNum(col_select.col, "color", selnode);
		loadNum(col_select.col_s, "color_s", selnode);
	} else {
		img_set.select = _img;
		col_select.col = _caption.col;
		col_select.col_s = _caption.col_s;
	}
}

void StateButton::State(const bool val) {
	if (val) {
		_img = img_set.select;
		_caption.col = col_select.col;
		_caption.col_s = col_select.col_s;
	} else {
		_img = img_set.normal;
		_caption.col = col_normal.col;
		_caption.col_s = col_normal.col_s;
	}

	// Images might be different in size
	w = g_engine->_imageManager->getTexture(_img._normal).w();
	h = g_engine->_imageManager->getTexture(_img._normal).h();
}

void StateButton::Img(const StateButtonImage &sbi) {
	// Find which is the current image and set it
	if (_img == img_set.normal)
		_img = sbi.normal;
	else
		_img = sbi.select;

	img_set = sbi;
}

} // End of namespace Crab
