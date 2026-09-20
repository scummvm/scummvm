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

#ifndef SCOOBY_RESTART_SEQUENCE_H
#define SCOOBY_RESTART_SEQUENCE_H

#include "common/func.h"

#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"

// Prepares the recovered visual state for a reset and hands restart ownership back to the
// application loop.

namespace Scooby {

class RestartSequence {
public:
	// Binds reset preparation to the active runtime state and shared frame presenter.
	// state: recovered state whose display flags survive until the reset handover.
	// presenter: palette-transition owner used before reset.
	RestartSequence(RuntimeState &state, FramePresenter &presenter)
		: _state(state), _presenter(presenter) {
	}

	// Clears the recovered display latch and completes the authored fade before the owner rebuilds
	// reset state.
	// beforeRetrace: optional installed callback invoked once before each faded frame.
	// Returns true when reset should proceed; false after a host close.
	//
	// Ghidra: RestartApplication (0x00008F9A-0x00008FB7). Interrupt masking, stack-vector
	// restoration, and the jump through reset vector four become a process-level handover: the
	// application loop reconstructs intro, clock, runtime, menu, callbacks, and room owners without
	// recreating the native window or emulating CPU control flow.
	bool run(const Common::Functor0<void> *beforeRetrace = nullptr) {
		// Ghidra 0x00008F9A-0x00008FA5: clear display bit 6 and complete the shared fade-out contract.
		_state.DisplayFlags &= 0xBF;
		if (!_presenter.fadePaletteOut(beforeRetrace)) {
			return false;
		}

		// Ghidra 0x00008FA6-0x00008FB7: CPU interrupt, stack, and vector writes become a reset handover.
		return true;
	}

private:
	RuntimeState &_state;
	FramePresenter &_presenter;
};
} // namespace Scooby

#endif // SCOOBY_RESTART_SEQUENCE_H
