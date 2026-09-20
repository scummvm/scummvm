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

#ifndef SCOOBY_ROOM_SCRIPT_CAMERA_PAN_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_CAMERA_PAN_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/runtime/runtime_state.h"

// Owns room-script camera-follow control and the shared blocking horizontal-pan lifecycle.

namespace Scooby {
class RoomScriptCameraPanExecutor {
public:
	// Binds the camera-pan actions to their shared state and asynchronous room-update boundary.
	// state: shared camera, room-behavior, and transition state.
	// updateActorAnimationFrames: recovered animation update called while a pan remains active.
	// waitForRoomVerticalBlank: callback-aware clocked frame whose scrolling owner advances the pan.
	RoomScriptCameraPanExecutor(RuntimeState &state, Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _state(state), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Locks the current camera origin by disabling automatic following.
	//
	// Ghidra: executeRoomScriptAction0D (0x00004482).
	void executeRoomScriptAction0D();

	// Restores automatic horizontal and vertical camera following.
	//
	// Ghidra: executeRoomScriptAction0E (0x0000448C).
	void executeRoomScriptAction0E();

	// Locks automatic following and pans the camera to the room's right edge.
	//
	// Ghidra: executeRoomScriptAction0F (0x000043C0). The installed room callback retains
	// UpdateScrollingAndStreamTiles as the owner that advances the pan and clears its active flag.
	void executeRoomScriptAction0F();

	// Locks automatic following and pans the camera to the room's left edge.
	//
	// Ghidra: executeRoomScriptAction10 (0x00004428). The installed room callback retains
	// UpdateScrollingAndStreamTiles as the owner that advances the pan and clears its active flag.
	void executeRoomScriptAction10();

private:
	static const uint8 kCameraPanActiveMask = 0x20;

	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_CAMERA_PAN_EXECUTOR_H
