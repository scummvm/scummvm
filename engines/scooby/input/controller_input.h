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

#ifndef SCOOBY_CONTROLLER_INPUT_H
#define SCOOBY_CONTROLLER_INPUT_H

#include "common/keyboard.h"
#include "controller_buttons.h"
#include "scooby/runtime/runtime_state.h"

// Owns the host-input replacement for Ghidra PollControllers at 0x00009D34
// without carrying Genesis controller-port hardware into managed code.
//
// Original controller-one state occupies current, prior, consumer baseline,
// and accumulated-change bytes at 0xFF09E0, 0xFF09E2, 0xFF09E4, and
// 0xFF09E6. The corresponding odd bytes belong to controller two and have
// no runtime reader, so this owner retains only controller one's
// active-low and transition semantics.

namespace Scooby {

class ControllerInput {
public:
	// Binds the process input owner to the recovered active-low input state.
	ControllerInput()
		: _backButton(ControllerButtons::A) {
	}

	// Selects the original controller action produced by Escape or
	// Backspace for the current screen. Ordinary play uses A; screens with
	// a different recovered exit restore this value when they leave.
	ControllerButtons _backButton;

	// Reimplements Ghidra PollControllers at 0x00009D34, retaining its
	// current, previous, baseline, and accumulated-change states without
	// its controller-port or Z80-bus operations.
	void pollControllers(RuntimeState &state);

	// Consumes one Escape press used to skip exactly one current intro
	// presentation. True only on the key-down edge reported by raylib.
	// Every queued event is consumed through the same key-state tracking as getControllerButtons, so key
	// releases that arrive during the intro are not lost (a lost release would leave that button held).
	bool isIntroSkipPressed();

private:
	// Samples the backend-neutral player controls from keyboard, mouse
	// buttons, and gamepad zero.
	Scooby::ControllerButtons getControllerButtons(Scooby::ControllerButtons backButton);

	// Drains all pending host events, applying key presses and releases to _buttons. Escape sets
	// backButton only when mapEscapeToBackButton is true. Returns whether an Escape press was seen.
	bool processEvents(Scooby::ControllerButtons backButton, bool mapEscapeToBackButton);

	static Scooby::ControllerButtons keyToButton(Common::KeyCode keycode);
	static Scooby::ControllerButtons mappedActionEventToButton(uint16 action);
	static Scooby::ControllerButtons getKeyboardAndMouseButtons(Scooby::ControllerButtons backButton);
	static Scooby::ControllerButtons getGamepadButtons();

	ControllerButtons _buttons = ControllerButtons::None;
	// The controller button the currently held Escape key is mapped to, or None.
	ControllerButtons _escapeButton = ControllerButtons::None;
};
} // namespace Scooby

#endif // SCOOBY_CONTROLLER_INPUT_H
