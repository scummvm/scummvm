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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_1F_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_1F_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns command 0x1F's combined actor movement and path-completion wait.

namespace Scooby {

class RoomScriptCommand1FExecutor {
public:
	// Binds actor identity records to movement, path, and animation-update state.
	// rom: verified cartridge containing command fields.
	// state: shared script cursor, room objects, movement mask, and path mask.
	// presenter: shared host presentation owner enforcing the room retrace cadence.
	// updateActorAnimationFrames: recovered animation update invoked by every wait iteration.
	// handleRoomVBlank: installed room callback that advances movement and path state.
	// requestSessionExit: parent handover used when the host closes during a wait.
	RoomScriptCommand1FExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								FramePresenter &presenter,
								Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<void> &handleRoomVBlank,
								Common::Functor1<SessionExit, void> &requestSessionExit)
		: _rom(rom), _state(state), _presenter(presenter),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _handleRoomVBlank(handleRoomVBlank), _requestSessionExit(requestSessionExit) {
	}

	// Advances animation until the selected actor has neither movement nor path traversal active.
	// mode: zero scans the four-byte record; nonzero evaluates every identity and wait branch.
	//
	// Ghidra: executeRoomScriptCommand1F (0x00004F70). The native loop checks movement before path state and
	// always performs at least one animation update. Managed continuations service the installed room callback
	// and one clocked host frame to replace asynchronous movement and path progress.
	void executeRoomScriptCommand1F(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	bool waitForRoomVerticalBlank();

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor1<SessionExit, void> &_requestSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_1F_EXECUTOR_H
