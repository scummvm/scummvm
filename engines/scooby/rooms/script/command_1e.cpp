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

#include "scooby/rooms/script/command_executor.h"

#include "scooby/rooms/actor_facing_animation.h"

// Owns command 0x1E's actor-position transition selection and optional hold wait.

namespace Scooby {

void RoomScriptCommandExecutor::executeRoomScriptCommand1E(int mode) {
	int32 commandOffset = _state.roomScriptCurrentOffset;

	// Ghidra 0x0000268E-0x00002691: scan mode bypasses every selector, position, animation, and wait effect.
	if (mode != 0) {
		// Ghidra 0x00002692-0x000026A3: clear selector bit fifteen only for actor selection, retain the
		// original sign for the independent wait branch, and preserve the signed next-position word.
		uint16 selector = _rom.readUInt16(commandOffset + 2);
		int16 nextPositionIndex = _rom.readInt16(commandOffset + 4);
		if ((selector & 0x7FFF) == 1) {
			// Ghidra 0x000026A4-0x000026C5: slot zero writes its new position before selecting the row-major
			// lead matrix entry from the prior and next signed words.
			int16 previousPositionIndex = _state.actorPositionIndices[kShaggyActorSlot];
			_state.actorPositionIndices[kShaggyActorSlot] = nextPositionIndex;
			_state.actorAnimationOffsets[kShaggyActorSlot] =
				resolveShaggyTurnAnimation(previousPositionIndex, nextPositionIndex);

			// Ghidra 0x000026C6-0x000026E5: slot zero always requests restart. A negative selector also
			// advances all animation streams at least once until hold bit zero is set; nonnegative selectors
			// retain the immediate exit.
			_state.actorAnimationRestartFlags |= kShaggyActorMask;
			if (static_cast<int16>(selector) < 0) {
				while (true) {
					_actorAnimator.updateActorAnimationFrames();
					if ((_state.actorAnimationHoldFlags & kShaggyActorMask) != 0) {
						break;
					}

					if (!waitForRoomVerticalBlank()) {
						return;
					}
				}
			}
		} else {
			// Ghidra 0x000026E6-0x00002707: every other masked selector chooses slot one, writes its new
			// position, and selects the row-major companion matrix entry from the prior and next words.
			int16 previousPositionIndex = _state.actorPositionIndices[kScoobyActorSlot];
			_state.actorPositionIndices[kScoobyActorSlot] = nextPositionIndex;
			_state.actorAnimationOffsets[kScoobyActorSlot] =
				resolveScoobyTurnAnimation(
					previousPositionIndex, nextPositionIndex);

			// Ghidra 0x00002708-0x0000272B: nonnegative selectors exit without requesting a slot-one restart.
			// Negative selectors set it, advance animation at least once until hold bit one is set, then
			// retain the native final rewrite of the preserved next-position word.
			if (static_cast<int16>(selector) < 0) {
				_state.actorAnimationRestartFlags |= kScoobyActorMask;
				while (true) {
					_actorAnimator.updateActorAnimationFrames();
					if ((_state.actorAnimationHoldFlags & kScoobyActorMask) != 0) {
						break;
					}

					if (!waitForRoomVerticalBlank()) {
						return;
					}
				}
				_state.actorPositionIndices[kScoobyActorSlot] = nextPositionIndex;
			}
		}
	}

	// Ghidra 0x0000272C-0x00002731: every mode, actor, sign, and hold outcome consumes six bytes.
	_state.roomScriptCurrentOffset = commandOffset + 6;
}

} // namespace Scooby
