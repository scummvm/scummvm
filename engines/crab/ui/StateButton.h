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

#ifndef CRAB_STATEBUTTON_H
#define CRAB_STATEBUTTON_H

#include "crab/ui/button.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
struct StateButtonImage {
	ButtonImage _normal, _select;

	StateButtonImage() {}
	StateButtonImage(rapidxml::xml_node<char> *node) {
		if (nodeValid("normal", node))
			_normal.load(node->first_node("normal"));

		if (nodeValid("select", node, false))
			_select.load(node->first_node("select"));
		else
			_select = _normal;
	}
};

struct StateButtonColor {
	int _col, _colS;

	StateButtonColor() {
		_col = 0;
		_colS = 0;
	}
};

// This button has two sets of images (b, h, s) and switching is done by clicking it
// Similar to the checkbox UI element in windows/web
class StateButton : public Button {
	// The two images
	StateButtonImage _imgSet;

	// The color for the caption when the image is selected
	StateButtonColor _colNormal, _colSelect;

public:
	StateButton() {}
	~StateButton() {}

	void init(const StateButton &ref, const int &xOffset = 0, const int &yOffset = 0);
	void load(rapidxml::xml_node<char> *node, const bool &echo = true);

	// The state of the button - false is original image, true is second image
	void state(const bool val);
	bool state() {
		return (_img == _imgSet._select);
	}

	// Set the image
	void img(const StateButtonImage &sbi);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_STATEBUTTON_H
