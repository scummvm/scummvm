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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_1E_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_1E_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns command 0x1E's actor-position transition selection and optional hold wait.

namespace Scooby {

class RoomScriptCommand1EExecutor {
public:
	// Binds transition commands to actor state, authored matrices, and animation advancement.
	// rom: verified cartridge containing command fields.
	// state: shared script cursor, actor positions, animation selections, and control masks.
	// presenter: shared host presentation owner enforcing the room retrace cadence.
	// updateActorAnimationFrames: recovered animation update used by requested hold waits.
	// handleRoomVBlank: installed room callback that advances interrupt-owned animation delays.
	// requestSessionExit: parent handover used when the host closes during a wait.
	RoomScriptCommand1EExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								FramePresenter &presenter,
								Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<void> &handleRoomVBlank,
								Common::Functor1<SessionExit, void> &requestSessionExit)
		: _rom(rom), _state(state), _presenter(presenter),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _handleRoomVBlank(handleRoomVBlank), _requestSessionExit(requestSessionExit) {
	}

	// Selects one lead or companion position-transition animation and optionally waits for its hold.
	// mode: zero scans the six-byte record; nonzero executes every selector and sign branch.
	//
	// Ghidra: executeRoomScriptCommand1E (0x0000268E). Selector bit fifteen requests the hold wait
	// independently of the actor choice, and the two actor branches deliberately restart asymmetrically.
	// Managed continuations service the installed room callback and one clocked host frame because native
	// VBlank can interrupt either tight wait and decrement positive animation delays.
	void executeRoomScriptCommand1E(int mode);

private:
	static const uint8 kCompanionActorMask = 0x02;
	static const uint8 kLeadActorMask = 0x01;

	bool waitForRoomVerticalBlank();

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor1<SessionExit, void> &_requestSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_1E_EXECUTOR_H
