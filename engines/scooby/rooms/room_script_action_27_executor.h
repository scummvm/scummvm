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

#ifndef SCOOBY_ROOM_SCRIPT_ACTION_27_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_ACTION_27_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns room-script action 0x27's paired-actor input sequence, path outcome, and presentation lifecycle.

namespace Scooby {

class RoomScriptAction27Executor {
public:
	// Binds the action to its shared actor state and room presentation boundaries.
	// state: shared input, actor, path, result, and display state.
	// presenter: shared retrace and palette-transition owner.
	// updateActorAnimationFrames: recovered actor-animation update used by the action loops.
	// handleRoomVBlank: installed room callback run at each recovered retrace boundary.
	// requestSessionExit: parent handover used when the host closes during a wait.
	RoomScriptAction27Executor(RuntimeState &state, FramePresenter &presenter,
							   Common::Functor0<void> &updateActorAnimationFrames,
							   Common::Functor0<void> &handleRoomVBlank,
							   Common::Functor1<SessionExit, void> &requestSessionExit)
		: _state(state), _presenter(presenter), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _handleRoomVBlank(handleRoomVBlank), _requestSessionExit(requestSessionExit) {
	}

	// Runs the paired-actor sequence and publishes its path-dependent room-object outcome.
	//
	// Ghidra: executeRoomScriptAction27 (0x00003F58). Slot-three and slot-four X movement preserves both
	// 16.16 fractional words. Slot-two's path-record byte offset selects the result after slot three reaches
	// its authored endpoint. Room callbacks remain active at every recovered retrace, while FramePresenter
	// replaces the original synchronous VBlank and palette-fade waits.
	void executeRoomScriptAction27();

private:
	static const int kCompanionActorSlot = 2;
	static const int kMovingActorSlot = 3;
	static const int kPairedActorSlot = 4;
	static const uint8 kAButtonMask = 0x40;
	static const uint8 kBButtonMask = 0x10;
	static const uint8 kLeftButtonMask = 0x04;
	static const uint8 kRightButtonMask = 0x08;
	static const int16 kMaximumX = 0x0094;
	static const int16 kMinimumX = 0x0064;
	static const int16 kTargetY = 0x004A;

	static int16 getIntegerWord(int coordinate);
	static void setIntegerWord(Common::Array<int32> &coordinates, int actorSlot, int16 integer);
	void setPairedActorX(int16 integer);
	bool waitForRoomVerticalBlank();
	static void playAudioCommand(int command);

	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor1<SessionExit, void> &_requestSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_ACTION_27_EXECUTOR_H
