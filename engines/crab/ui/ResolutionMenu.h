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

#ifndef CRAB_RESOLUTIONMENU_H
#define CRAB_RESOLUTIONMENU_H
#include "crab/common_header.h"

#include "crab/ui/menu.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class ResolutionMenu : public ButtonMenu {
	enum State {
		STATE_NORMAL,
		STATE_CHANGE
	} _state;

	HoverInfo _info;
	Common::String _defInfo;
	Button _change, _cancel, _custom;

	// Menu stores the button for each of the item in the dimension array
	Common::Array<Dimension> _dim;

	// The reference button for resolution
	Button _ref;

	// How much the button is incremented by
	Vector2i _inc;

	// The number of rows and columns
	int _columns;

public:
	ResolutionMenu(void) {
		_state = STATE_NORMAL;
		_columns = 1;
	}
	~ResolutionMenu(void) {}

	void load(rapidxml::xml_node<char> *node);
	void draw();

	// Return 1 if one of resolution buttons is pressed, 2 if custom button is pressed, 0 otherwise
	int handleEvents(const Common::Event &event);

	void setInfo();

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_RESOLUTIONMENU_H
