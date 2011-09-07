/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/system.h"

#include "pegasus/input.h"

namespace Pegasus {

InputDevice::InputDevice() {
	_lastRawBits = kAllUpBits;
}

InputDevice::~InputDevice() {
}

void InputDevice::getInput(Input &input, const tInputBits filter) {
	// TODO: Save/Load keys

	tInputBits currentBits = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// We only care about two events here
		// We're mapping from ScummVM events to pegasus events, which
		// are based on pippin events.
		if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				currentBits |= (kRawButtonDown << kUpButtonShift);
				break;
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				currentBits |= (kRawButtonDown << kLeftButtonShift);
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP5:
				currentBits |= (kRawButtonDown << kDownButtonShift);
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				currentBits |= (kRawButtonDown << kRightButtonShift);
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_SPACE:
				currentBits |= (kRawButtonDown << kTwoButtonShift);
				break;
			case Common::KEYCODE_t:
			case Common::KEYCODE_KP_EQUALS:
				currentBits |= (kRawButtonDown << kThreeButtonShift);
				break;
			case Common::KEYCODE_i:
			case Common::KEYCODE_KP_DIVIDE:
				currentBits |= (kRawButtonDown << kFourButtonShift);
				break;
			case Common::KEYCODE_q:
				currentBits |= (kRawButtonDown << kMod1ButtonShift);
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_p:
				currentBits |= (kRawButtonDown << kMod3ButtonShift);
				break;
			case Common::KEYCODE_TILDE:
			case Common::KEYCODE_NUMLOCK: // Yes, the original uses Num Lock/Clear on the Mac...
				currentBits |= (kRawButtonDown << kLeftFireButtonShift);
				break;
			case Common::KEYCODE_DELETE:
				currentBits |= (kRawButtonDown << kRightFireButtonShift);
				break;
			default:
				break;
			}

			if (event.kbd.flags & Common::KBD_ALT) // Alt, option, same thing!
				currentBits |= (kRawButtonDown << kMod2ButtonShift);
		}
	}

	// Update mouse button state
	// Note that we don't use EVENT_LBUTTONUP/EVENT_LBUTTONDOWN because
	// they do not show if the button is being held down. We're treating
	// both mouse buttons as the same for ease of use.
	if (g_system->getEventManager()->getButtonState() != 0)
		currentBits |= (kRawButtonDown << kTwoButtonShift);

	// Update the mouse position too
	input.setInputLocation(g_system->getEventManager()->getMousePos());
	_lastRawBits = currentBits;

	tInputBits filteredBits = currentBits & filter;
	input.setInputBits((filteredBits & kAllButtonDownBits) | (filteredBits & _lastRawBits & kAllAutoBits));
}

//	Wait until the input device stops returning input allowed by filter...
void InputDevice::waitInput(const tInputBits filter) {
	if (filter != 0) {
		for (;;) {
			Input input;
			getInput(input, filter);
			if (!input.anyInput())
				break;
		}
	}
}

int operator==(const Input &arg1, const Input &arg2) {
	return arg1._inputState == arg2._inputState;
}

int operator!=(const Input &arg1, const Input &arg2) {
	return !operator==(arg1, arg2);
}

} // End of namespace Pegasus
