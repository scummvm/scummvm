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

#include "room_script_camera_pan_executor.h"

namespace Scooby {

void RoomScriptCameraPanExecutor::executeRoomScriptAction0D() {
	// Ghidra 0x00004482-0x00004489: set only the camera-follow lock.
	_state.RoomBehaviorFlags |= 0x02;
	// Ghidra 0x0000448A-0x0000448B: return with every other room-behavior flag preserved.
}

void RoomScriptCameraPanExecutor::executeRoomScriptAction0E() {
	// Ghidra 0x0000448C-0x00004493: clear only the camera-follow lock.
	_state.RoomBehaviorFlags &= 0xFD;
	// Ghidra 0x00004494-0x00004495: return with every other room-behavior flag preserved.
}

void RoomScriptCameraPanExecutor::executeRoomScriptAction0F() {
	// Ghidra 0x000043C0-0x000043C7: disable automatic camera following without changing other behavior.
	_state.RoomBehaviorFlags |= 0x02;

	// Ghidra 0x000043C8-0x000043EF: clear only camera-X bit zero, then preserve the separate signed 16-bit
	// next-tile comparison and its early return.
	_state.CameraX &= 0xFFFE;
	int16 alignedNextTileX = static_cast<int16>((_state.CameraX + 8) & 0xFFF8);
	int16 cameraTargetX = static_cast<int16>((_state.RoomWidthTiles - 32) << 3);
	if (alignedNextTileX >= cameraTargetX) {
		return;
	}

	// Ghidra 0x000043F0-0x00004409: retain both normal and alternate positive-step branches.
	_state.CameraPanStep = (_state.TransitionFlags & 0x10) == 0
							   ? static_cast<int16>(2)
							   : static_cast<int16>(8);

	// Ghidra 0x0000440A-0x00004425: publish the exact target, activate the pan, advance animations at least
	// once, and preserve the complete back edge. Original VBlank interrupts update camera X asynchronously,
	// so service one callback-aware clocked frame after every still-active iteration.
	_state.CameraPanTargetX = cameraTargetX;
	_state.TransitionFlags |= kCameraPanActiveMask;
	do {
		_updateActorAnimationFrames();
		if ((_state.TransitionFlags & kCameraPanActiveMask) != 0) {
			if (!_waitForRoomVerticalBlank()) {
				return;
			}
		}
	} while ((_state.TransitionFlags & kCameraPanActiveMask) != 0);

	// Ghidra 0x00004426-0x00004427: return after the scrolling owner reaches the exact target.
}

void RoomScriptCameraPanExecutor::executeRoomScriptAction10() {
	// Ghidra 0x00004428-0x0000442F: disable automatic camera following without changing other behavior.
	_state.RoomBehaviorFlags |= 0x02;

	// Ghidra 0x00004430-0x00004445: align camera X to an even pixel and preserve the zero early return.
	_state.CameraX &= 0xFFFE;
	if (_state.CameraX == 0) {
		return;
	}

	// Ghidra 0x00004446-0x00004467: target the left edge and retain both normal and alternate step branches.
	_state.CameraPanTargetX = 0;
	_state.CameraPanStep = (_state.TransitionFlags & 0x10) == 0
							   ? static_cast<int16>(-2)
							   : static_cast<int16>(-8);

	// Ghidra 0x00004468-0x0000447F: activate the pan, advance animations at least once, and preserve the
	// complete back edge. Original VBlank interrupts update camera X asynchronously, so service the installed
	// room callback and shared clock after every still-active managed iteration.
	_state.TransitionFlags |= kCameraPanActiveMask;
	do {
		_updateActorAnimationFrames();
		if ((_state.TransitionFlags & kCameraPanActiveMask) != 0) {
			if (!_waitForRoomVerticalBlank()) {
				return;
			}
		}
	} while ((_state.TransitionFlags & kCameraPanActiveMask) != 0);

	// Ghidra 0x00004480-0x00004481: return after the scrolling owner reaches the exact target.
}
} // namespace Scooby