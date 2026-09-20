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

#include "room_script_paired_actor_transition_executor.h"

namespace Scooby {

void RoomScriptPairedActorTransitionExecutor::executeRoomScriptAction15() {
	while (true) {
		// Ghidra 0x0000432C-0x0000433B: advance all animations at least once and retain the back edge until
		// actor zero reaches a hold command. Room VBlank remains asynchronous on the original, so service the
		// installed callback before each managed continuation.
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & 0x01) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x0000433C-0x00004355: test slots two through five in order. A clear bit exits immediately; after
	// active slot five, retain the terminal branch that selects six without testing active bit six.
	int transitionActor = 2;
	while ((_state.ActiveActorFlags & (1 << transitionActor)) != 0) {
		transitionActor++;
		if (transitionActor == 6) {
			break;
		}
	}

	_state.TransitionActorIndex = static_cast<uint16>(transitionActor);

	// Ghidra 0x00004356-0x00004363: enable paired transition processing and discard all prior readiness.
	_state.RoomBehaviorFlags |= 0x08;
	_state.ActorTransitionFrameReadyFlags = 0;

	// Ghidra 0x00004364-0x00004369: replace WaitForVerticalBlank's hardware handshake with one installed room
	// callback followed by one host presentation, then return after that single retrace.
	if (!_waitForRoomVerticalBlank()) {
		return;
	}
}

void RoomScriptPairedActorTransitionExecutor::executeRoomScriptAction16() {
	// Ghidra 0x0000436A-0x00004373: clear only room-behavior bit three and return.
	_state.RoomBehaviorFlags &= 0xF7;
}
} // namespace Scooby