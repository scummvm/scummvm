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

#include "scooby/rooms/script/action_27_executor.h"

#include "common/scummsys.h"

#include "common/array.h"
#include "scooby/sound/gems_audio.h"

namespace Scooby {

void RoomScriptAction27Executor::executeRoomScriptAction27() {
	// Ghidra 0x00003F58-0x00003F79: enter forced staged-actor selection, retain the signed horizontal
	// step in D1 across input-loop iterations, seed no-input state, and clear inactive path slot two.
	_state.transitionFlags |= 0x04;
	int16 horizontalStep = 0;
	_state.controllerOneInput = 0xFF;
	if ((_state.actorPathActiveFlags & 0x04) == 0) {
		_state.actorPathRecordOffsets[kPathActorSlot] = 0;
	}

	while (true) {
		// Ghidra 0x00003F7A-0x00003FBD: advance animations, retain left/right's active-low signed
		// step with right taking precedence, and move both actors only inside the inclusive X bounds.
		_state.retraceCountdown = 0;
		_actorAnimator.updateActorAnimationFrames();
		if (_state.isButtonDown(ControllerButtons::Left)) {
			horizontalStep = -2;
		}

		if (_state.isButtonDown(ControllerButtons::Right)) {
			horizontalStep = 2;
		}

		int16 candidateX = static_cast<int16>(
			getIntegerWord(_state.actorXFixedCoordinates[kMovingActorSlot]) + horizontalStep);
		if (candidateX >= kMinimumX && candidateX <= kMaximumX) {
			setPairedActorX(candidateX);
		}

		// Ghidra 0x00003FBE-0x00003FDF: A has exit priority over B; with neither held, wait until the
		// installed room callback decrements the zero retrace countdown through -1 and repeat.
		ControllerButtons testedButton;
		if (_state.isButtonDown(ControllerButtons::A)) {
			testedButton = ControllerButtons::A;
		} else if (_state.isButtonDown(ControllerButtons::B)) {
			testedButton = ControllerButtons::B;
		} else {
			while (_state.retraceCountdown >= 0) {
				if (!waitForRoomVerticalBlank()) {
					return;
				}
			}

			continue;
		}

		// Ghidra 0x00003FE0-0x00004007: retain the raw Z-clear fallthrough in full. The incoming
		// active-low button branches normally carry Z set and skip it, but that does not remove this body:
		// if the tested state differs here, move the pair once per retrace until the next step is out of range.
		if (!_state.isButtonDown(testedButton)) {
			while (true) {
				if (!waitForRoomVerticalBlank()) {
					return;
				}

				candidateX = static_cast<int16>(
					getIntegerWord(_state.actorXFixedCoordinates[kMovingActorSlot]) + horizontalStep);
				if (candidateX < kMinimumX || candidateX > kMaximumX) {
					break;
				}

				setPairedActorX(candidateX);
			}
		}

		break;
	}

	// Ghidra 0x00004008-0x00004027: write slot three's current integer Y before each retrace, advance
	// by signed word steps of two, and replace the final value with exactly 0x004A.
	int16 movingActorY = getIntegerWord(_state.actorYFixedCoordinates[kMovingActorSlot]);
	do {
		setIntegerWord(_state.actorYFixedCoordinates, kMovingActorSlot, movingActorY);
		if (!waitForRoomVerticalBlank()) {
			return;
		}

		movingActorY += 2;
	} while (movingActorY < kTargetY);
	setIntegerWord(_state.actorYFixedCoordinates, kMovingActorSlot, kTargetY);

	// Ghidra 0x00004028-0x00004051: endpoint 0x0094 accepts unsigned path offsets 0x005C-0x0078;
	// every other endpoint accepts 0x00F8-0x0114, retaining both inclusive comparison branches.
	uint16 pathRecordOffset = _state.actorPathRecordOffsets[kPathActorSlot];
	bool success = getIntegerWord(_state.actorXFixedCoordinates[kMovingActorSlot]) == kMaximumX
					   ? (pathRecordOffset >= 0x005C && pathRecordOffset <= 0x0078)
					   : (pathRecordOffset >= 0x00F8 && pathRecordOffset <= 0x0114);

	// Ghidra 0x00004052-0x0000407D: publish success audio, animation 0x1C, and result one, or retain
	// the independent failure branch selecting animation 0x1B and result zero.
	if (success) {
		playAudioCommand(0x63);
		_state.actorAnimationOffsets[kMovingActorSlot] = 0x1C;
		_state.roomObjects[0].scriptStateWords[0] = 1;
	} else {
		_state.actorAnimationOffsets[kMovingActorSlot] = 0x1B;
		_state.roomObjects[0].scriptStateWords[0] = 0;
	}

	// Ghidra 0x0000407E-0x000040AD: restart slot three, advance every actor until its command stream
	// holds, preserve audio command 0x62, and block companion visibility only for nonzero result. Native
	// room VBlank can interrupt the hold loop, so wait for one callback-aware retrace before continuing.
	_state.actorAnimationRestartFlags |= 0x08;
	while (true) {
		_actorAnimator.updateActorAnimationFrames();
		if ((_state.actorAnimationHoldFlags & 0x08) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	playAudioCommand(0x62);
	if (_state.roomObjects[0].scriptStateWords[0] != 0) {
		_state.actorVisibilityBlockFlags |= 0x04;
	}

	// Ghidra 0x000040AE-0x000040B9: after each retrace, decrement only slot three's signed integer
	// Y word with wrap until it is exactly zero, preserving its fractional low word.
	do {
		if (!waitForRoomVerticalBlank()) {
			return;
		}

		movingActorY =
			static_cast<int16>(getIntegerWord(_state.actorYFixedCoordinates[kMovingActorSlot]) - 1);
		setIntegerWord(_state.actorYFixedCoordinates, kMovingActorSlot, movingActorY);
	} while (movingActorY != 0);

	// Ghidra 0x000040BA-0x000040DF: leave forced actor selection, suppress the complete room-update
	// block during FadePaletteOut, restore it afterward, reveal the companion on both outcomes, and return.
	_state.transitionFlags &= 0xFB;
	_state.displayFlags &= 0xBF;
	if (!_presenter.fadePaletteOut(_roomVBlank.beforeRetraceCallback())) {
		_requestSessionExit(SessionExit::HostClosed);
		return;
	}

	_state.displayFlags |= 0x40;
	_state.actorVisibilityBlockFlags &= 0xFB;
}

int16 RoomScriptAction27Executor::getIntegerWord(int coordinate) {
	return static_cast<int16>(coordinate >> 16);
}

void RoomScriptAction27Executor::setIntegerWord(Common::Array<int32> &coordinates, int actorSlot,
												int16 integer) {
	coordinates[static_cast<size_t>(actorSlot)] =
		(coordinates[static_cast<size_t>(actorSlot)] & 0x0000FFFF) | (static_cast<int32>(integer) << 16);
}

void RoomScriptAction27Executor::setPairedActorX(int16 integer) {
	setIntegerWord(_state.actorXFixedCoordinates, kMovingActorSlot, integer);
	setIntegerWord(_state.actorXFixedCoordinates, kPairedActorSlot, integer);
}

bool RoomScriptAction27Executor::waitForRoomVerticalBlank() {
	_roomVBlank.handleRoomVBlank();
	if (_presenter.waitForVerticalBlank()) {
		return true;
	}

	_requestSessionExit(SessionExit::HostClosed);
	return false;
}

// Ghidra PlayAudioCommand (gemsStartSong, 0x00009762).
void RoomScriptAction27Executor::playAudioCommand(int command) {
	gemsStartSong(command);
}
} // namespace Scooby
