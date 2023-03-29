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

#ifndef CRAB_HOTKEY_H
#define CRAB_HOTKEY_H

#include "crab/common_header.h"
#include "crab/input/input.h"

namespace Crab {

namespace pyrodactyl {
namespace input {
// This class is built to integrate the input check for hotkeys bound to buttons
class HotKey {
	// The type of input the hotkey is checking for
	InputType input;

	// Has the key been pressed?
	bool keydown;

public:
	HotKey() {
		input = IT_NONE;
		keydown = false;
	}

	void Set(const InputType &val) { input = val; }
	const char *Name();

	void Load(rapidxml::xml_node<char> *node);
	bool HandleEvents(const SDL_Event &Event);
};
} // End of namespace input
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_HOTKEY_H
