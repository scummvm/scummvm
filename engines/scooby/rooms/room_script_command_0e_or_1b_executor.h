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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_0E_OR_1B_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_0E_OR_1B_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/runtime/runtime_state.h"

// Owns indexed and immediate room-script placement commands for actors and room objects.

namespace Scooby {

class RoomScriptCommand0EOr1BExecutor {
public:
	// Binds authored placement records to actor state and its asynchronous publication owners.
	// rom: verified cartridge containing command records and active-room coordinates.
	// state: shared script cursor, room-position source, and actor state.
	// updateActorAnimationFrames: recovered actor-animation interpreter used by direct placement.
	// waitForRoomVerticalBlank: callback-aware clocked frame that publishes staged graphics.
	// movement: shared owner of the original scripted movement boundaries.
	// requestMovementHostClose: records the non-returning handover from shared movement waits.
	RoomScriptCommand0EOr1BExecutor(const ScoobyDooRom &rom, RuntimeState &state,
									Common::Functor0<void> &updateActorAnimationFrames,
									Common::Functor0<bool> &waitForRoomVerticalBlank,
									ScriptedRoomMovementExecutor &movement,
									Common::Functor0<void> &requestMovementHostClose)
		: _rom(rom), _state(state), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank), _movement(movement),
		  _requestMovementHostClose(requestMovementHostClose) {
	}

	// Places or moves one encoded actor or room object from an indexed or immediate destination.
	// mode: zero scans the record; nonzero executes every identity and movement branch.
	//
	// Ghidra: executeRoomScriptCommand0EOr1B (0x000049BA). Command 0x0E consumes ten bytes and scales one
	// indexed room-position pair by eight; command 0x1B consumes twelve bytes and uses its immediate signed
	// coordinates. Encoded identity bit 15 suppresses nonnegative movement completion waits but is masked from
	// identity selection; room-object direct placement still waits for a requested animation's graphics.
	// Negative movement mode replaces only the integer half of each 16.16 coordinate; actor zero restarts at
	// delay -1, while actor one retains the original asymmetric delay 1.
	void executeRoomScriptCommand0EOr1B(int mode);

private:
	static const uint16 kIndexedPositionCommand = 0x0E;
	static const int kIndexedRecordSize = 10;
	static const int kImmediateRecordSize = 12;

	// Replaces the C# private "(short X, short Y)" tuple return.
	struct TargetCoordinates {
		int16 X;
		int16 Y;

		TargetCoordinates(int16 x, int16 y) : X(x), Y(y) {
		}
	};

	TargetCoordinates readTargetCoordinates(int commandOffset, uint16 command);
	bool placeActorDirectly(int actorSlot, int16 targetX, int16 targetY,
							int16 animationOffset,
							int16 initialDelay, uint8 actorMask);
	static int replaceIntegerCoordinate(int fixedCoordinate, int16 integerCoordinate);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	ScriptedRoomMovementExecutor &_movement;
	Common::Functor0<void> &_requestMovementHostClose;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_0E_OR_1B_EXECUTOR_H
