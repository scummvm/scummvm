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

#ifndef TESORO_CORE_POINTERSTATE_H
#define TESORO_CORE_POINTERSTATE_H

#include "common/scummsys.h"

namespace Tesoro {

struct PointerState {
	enum Button { kLeft, kRight };
	enum InputEvent {
		kReleased = 1,		// went up this frame
		kPressed = 8,		// went down this frame
		kDoublePress = 0x10	// went down again within kDoublePressDelay ms of going up
	};

	/** Milliseconds after a release in which a new press counts as a double click. */
	static const uint32 kDoublePressDelay = 300;

	int32 x = 0, y = 0;
	byte buttons[2] = { 0, 0 };
	uint32 releasedAt[2] = { 0, 0 };

	byte buttonState(Button button) const { return buttons[button]; }
};

} // End of namespace Tesoro

#endif
