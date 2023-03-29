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

#ifndef CRAB_CLIPBUTTON_H
#define CRAB_CLIPBUTTON_H
#include "crab/ui/button.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
// Sometimes we need to display a clipped version of the button image
class ClipButton : public Button {
public:
	// The clip rectangle
	Rect clip;

	ClipButton() {}
	~ClipButton() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void Draw(const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_CLIPBUTTON_H
