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

#include "crab/input/hotkey.h"

namespace Crab {

using namespace pyrodactyl::input;

void HotKey::Load(rapidxml::xml_node<char> *node) {
	LoadEnum(input, "input", node);
}

#if 0
bool HotKey::HandleEvents(const SDL_Event &Event) {
	if (input > IT_NONE && input < IT_TOTAL) {
		int result = gInput.Equals(input, Event);

		if (result == SDL_PRESSED)
			keydown = true;
		else if ((keydown && result == SDL_RELEASED) || result == ANALOG_PRESSED) {
			keydown = false;
			return true;
		}
	}

	return false;
}
#endif

bool HotKey::HandleEvents(const Common::Event &Event) {
	warning("STUB: HotKey::HandleEvents()");

	return false;
}

const char *HotKey::Name() {
	warning("STUB: HotKey::Name()");

#if 0
	if (input > IT_NONE && input < IT_TOTAL)
		return SDL_GetScancodeName(gInput.iv[input].key);
#endif

	return "";
}

} // End of namespace Crab
