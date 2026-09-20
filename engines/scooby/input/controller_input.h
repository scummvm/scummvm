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

#include "controller_buttons.h"
#include "scooby/raylib_host.h"
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
	ControllerInput(RaylibHost &host, RuntimeState &state)
		: _backButton(ControllerButtons::A), _host(host), _state(state) {
	}

	// Selects the original controller action produced by Escape or
	// Backspace for the current screen. Ordinary play uses A; screens with
	// a different recovered exit restore this value when they leave.
	ControllerButtons _backButton;

	// Reimplements Ghidra PollControllers at 0x00009D34, retaining its
	// current, previous, baseline, and accumulated-change states without
	// its controller-port or Z80-bus operations.
	void pollControllers();

private:
	RaylibHost &_host;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_CONTROLLER_INPUT_H
