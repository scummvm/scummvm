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

#ifndef SCOOBY_MAIN_MENU_INPUT_STATE_H
#define SCOOBY_MAIN_MENU_INPUT_STATE_H

#include "common/scummsys.h"

#include "scooby/input/controller_buttons.h"
#include "scooby/runtime/runtime_state.h"

// Owns the comparison sample shared by main-menu selection and password entry.

namespace Scooby {

class MainMenuInputState {
public:
	// Binds the menu comparison sample to current and accumulated controller state.
	// state: shared active-low controller samples and accepted-change state.
	explicit MainMenuInputState(RuntimeState &state) : _state(state) {
	}

	// Seeds option selection from the prior sample retained by controller polling.
	void beginSelection() { _previousInput = _state.PreviousControllerOneInput; }

	// Tests one ordinary active-low press edge against the retained comparison sample.
	// button: single button whose press edge is required.
	// Returns true only on a released-to-held transition.
	bool wasPressed(ControllerButtons button) const {
		uint8 mask = static_cast<uint8>(button);
		return (_state.ControllerOneInput & mask) == 0 && (_previousInput & mask) != 0;
	}

	// Accepts a directional press from either the retained sample or PollControllers'
	// accumulated changes.
	// button: single directional button tested by the password grid.
	// Returns true when the original buffered-edge condition accepts the press.
	bool acceptBufferedPress(ControllerButtons button) {
		uint8 mask = static_cast<uint8>(button);
		if ((_state.ControllerOneInput & mask) != 0 ||
			((_previousInput & mask) == 0 && (_state.ControllerOneAccumulatedChanges & mask) == 0)) {
			return false;
		}

		_state.ControllerOneAccumulatedChanges = 0;
		_state.ControllerOneChangeBaseline = _state.ControllerOneInput;
		return true;
	}

	// Tests whether any button in an active-low group gained a press since the retained sample.
	// mask: button group whose released value has every bit set.
	// Returns true when the prior group was released and the current group is not.
	bool wasAnyPressed(uint8 mask) const {
		return (_previousInput & mask) == mask && (_state.ControllerOneInput & mask) != mask;
	}

	// Retains the current controller sample for the next menu-side edge test.
	void captureCurrent() { _previousInput = _state.ControllerOneInput; }

private:
	RuntimeState &_state;

	// Typed main-menu projection of Ghidra g_wSharedMenuAndSpriteScratch0010 at 0xFF0010.
	// Selection writes it before any input read, so no image-backed value is consumed.
	uint8 _previousInput{};
};
} // namespace Scooby

#endif // SCOOBY_MAIN_MENU_INPUT_STATE_H
