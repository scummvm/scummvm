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

void StateButton::init(const StateButton &ref, const int &XOffset, const int &YOffset) {
	Button::init(ref, XOffset, YOffset);
	_imgSet = ref._imgSet;
	_colNormal = ref._colNormal;
	_colSelect = ref._colSelect;
}

void StateButton::load(rapidxml::xml_node<char> *node, const bool &echo) {
	Button::load(node, echo);

	_imgSet._normal = _img;
	_colNormal._col = _caption._col;
	_colNormal._colS = _caption._colS;

	if (nodeValid("select", node, false)) {
		rapidxml::xml_node<char> *selnode = node->first_node("select");

		_imgSet._select.load(selnode, echo);
		loadNum(_colSelect._col, "color", selnode);
		loadNum(_colSelect._colS, "color_s", selnode);
	} else {
		_imgSet._select = _img;
		_colSelect._col = _caption._col;
		_colSelect._colS = _caption._colS;
	}
}

void StateButton::state(const bool val) {
	if (val) {
		_img = _imgSet._select;
		_caption._col = _colSelect._col;
		_caption._colS = _colSelect._colS;
	} else {
		_img = _imgSet._normal;
		_caption._col = _colNormal._col;
		_caption._colS = _colNormal._colS;
	}

	// Images might be different in size
	w = g_engine->_imageManager->getTexture(_img._normal).w();
	h = g_engine->_imageManager->getTexture(_img._normal).h();
}

void StateButton::img(const StateButtonImage &sbi) {
	// Find which is the current image and set it
	if (_img == _imgSet._normal)
		_img = sbi._normal;
	else
		_img = sbi._select;

	_imgSet = sbi;
}

} // End of namespace Crab
