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

#ifndef SCOOBY_ROOM_SCRIPT_PAIRED_ACTOR_TRANSITION_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_PAIRED_ACTOR_TRANSITION_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/runtime/runtime_state.h"

// Owns room-script entry and exit for paired actor-transition publication.

namespace Scooby {

class RoomScriptPairedActorTransitionExecutor {
public:
	// Binds the paired transition lifecycle to its shared actor and behavior state.
	// state: shared actor-transition state mutated and consumed across room updates.
	// updateActorAnimationFrames: recovered actor-animation update run by transition setup.
	// waitForRoomVerticalBlank: callback-aware clocked frame servicing transition retraces.
	RoomScriptPairedActorTransitionExecutor(RuntimeState &state,
											Common::Functor0<void> &updateActorAnimationFrames,
											Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _state(state), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Selects the paired transition target and starts synchronized frame publication with actor zero.
	//
	// Ghidra: executeRoomScriptAction15 (0x0000432C). Slots two through five are tested in order; the
	// original stores sentinel 6 without testing bit six when every candidate is active.
	void executeRoomScriptAction15();

	// Ends paired actor-transition processing while preserving its prepared state.
	//
	// Ghidra: executeRoomScriptAction16 (0x0000436A).
	void executeRoomScriptAction16();

private:
	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_PAIRED_ACTOR_TRANSITION_EXECUTOR_H
