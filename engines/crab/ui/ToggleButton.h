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

#ifndef CRAB_TOGGLEBUTTON_H
#define CRAB_TOGGLEBUTTON_H

#include "crab/ui/button.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
// This button has two states that can be switched between by clicking the button
// Similar to the radio button / checkbox UI element in windows/web
class ToggleButton : public Button {
	// The images corresponding to the state
	ImageKey _on, _off;

	// The offset at which the on/off image is drawn
	Vector2i _offset;

public:
	// The state of the button - true is on, false is off
	bool _state;

	ToggleButton() {
		_state = false;
		_on = 0;
		_off = 0;
	}

	~ToggleButton() {}

	void load(rapidxml::xml_node<char> *node);
	void draw(const int &xOffset = 0, const int &yOffset = 0, Rect *clip = nullptr);

	ButtonAction handleEvents(const Common::Event &event, const int &xOffset = 0, const int &yOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TOGGLEBUTTON_H
