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
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
struct StateButtonImage {
	ButtonImage normal, select;

	StateButtonImage() {}
	StateButtonImage(rapidxml::xml_node<char> *node) {
		if (nodeValid("normal", node))
			normal.Load(node->first_node("normal"));

		if (nodeValid("select", node, false))
			select.Load(node->first_node("select"));
		else
			select = normal;
	}
};

struct StateButtonColor {
	int col, col_s;

	StateButtonColor() {
		col = 0;
		col_s = 0;
	}
};

// This button has two sets of images (b, h, s) and switching is done by clicking it
// Similar to the checkbox UI element in windows/web
class StateButton : public Button {
	// The two images
	StateButtonImage img_set;

	// The color for the caption when the image is selected
	StateButtonColor col_normal, col_select;

public:
	StateButton() {}
	~StateButton() {}

	void Init(const StateButton &ref, const int &XOffset = 0, const int &YOffset = 0);
	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);

	// The state of the button - false is original image, true is second image
	void State(const bool val);
	bool State() { return (img == img_set.select); }

	// Set the image
	void Img(const StateButtonImage &sbi);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_STATEBUTTON_H
