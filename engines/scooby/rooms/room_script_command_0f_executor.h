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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_0F_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_0F_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "actor_animation_descriptor_catalog.h"
#include "scooby/assets/rom.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/runtime/runtime_state.h"

// Owns relative room-script movement across actor and room-object coordinate sources.

namespace Scooby {

class RoomScriptCommand0FExecutor {
public:
	// Binds relative movement records to their descriptor, object, actor, and movement owners.
	// rom: verified cartridge containing command records and episode shape data.
	// state: shared script cursor, room objects, actor coordinates, and position indices.
	// actorDescriptorCatalog: shared owner of all 36 actor animation descriptors.
	// movement: shared owner of the original scripted movement boundaries.
	// requestMovementHostClose: records the non-returning handover from shared movement waits.
	RoomScriptCommand0FExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								const ActorAnimationDescriptorCatalog &actorDescriptorCatalog,
								ScriptedRoomMovementExecutor &movement,
								Common::Functor0<void> &requestMovementHostClose)
		: _rom(rom), _state(state), _actorDescriptorCatalog(actorDescriptorCatalog), _movement(movement),
		  _requestMovementHostClose(requestMovementHostClose) {
	}

	// Moves one encoded target relative to an actor or room-object coordinate source.
	// mode: zero scans the record; nonzero executes every descriptor, source, and target branch.
	//
	// Ghidra: executeRoomScriptCommand0F (0x00004C20). The 12-byte record masks target bit 15 for selection
	// but retains the encoded word for lead and room-object nonnegative movement completion policy;
	// room-object direct placement still waits for requested animation graphics, while companion-direct
	// movement does not inspect it. Source identities one and two use actor integer coordinates; every other
	// source uses its room-position selector and the target animation descriptor's type-two record before
	// fixed or shape-relative placement.
	void executeRoomScriptCommand0F(int mode);

private:
	static const int kEpisodeShapeDataBaseField = 0x1C;
	static const int kEpisodeShapePointerTableField = 0x24;
	static const uint8 kFixedPositionMask = 0x02;
	static const int kRecordSize = 12;
	static const int kRoomObjectRecordSize = 0x1A;

	// Replaces the C# private "(short X, short Y)" tuple return.
	struct RelativeDestination {
		int16 X;
		int16 Y;

		RelativeDestination(int16 x, int16 y) : X(x), Y(y) {
		}
	};

	bool tryResolveTargetAnimationDescriptor(uint16 targetIdentity, int &descriptorOffset);
	RelativeDestination resolveRelativeDestination(int commandOffset, int animationDescriptorOffset);
	RoomObject &resolveRoomObject(uint16 objectIdentity);
	static int16 getIntegerCoordinate(int fixedCoordinate);

	// Resolves signed coordinates through an animation descriptor's type-two interaction record.
	// animationDescriptorOffset: ROM offset of the selected actor animation descriptor.
	// positionOffset: signed position selector used to index the descriptor's word table.
	// Returns the type-two record's signed X and Y coordinates.
	//
	// Ghidra: findInteractionTypeTwoRecord (0x0000197E). The complete body is 0x0000197E-0x000019AF.
	// Descriptor-relative offsets and the doubled position selector retain signed word wrapping and sign
	// extension. Four-byte {type, recordOffset} entries are scanned without a sentinel, bound, or fallback
	// until type 2; that entry resolves a descriptor-relative record whose signed coordinate words are at
	// +0x0E/+0x10.
	RelativeDestination
	findInteractionTypeTwoRecord(int animationDescriptorOffset, int16 positionOffset);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	const ActorAnimationDescriptorCatalog &_actorDescriptorCatalog;
	ScriptedRoomMovementExecutor &_movement;
	Common::Functor0<void> &_requestMovementHostClose;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_0F_EXECUTOR_H
