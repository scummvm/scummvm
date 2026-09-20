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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_05_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_05_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "actor_animation_descriptor_catalog.h"
#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns room-object relocation across actor slots, inventory order, and action-prompt publication.

namespace Scooby {

class RoomScriptCommand05Executor {
public:
	// Binds object relocation to its actor, inventory, retrace, and prompt owners.
	// rom: verified cartridge containing command records and the parallel actor-shape catalogs.
	// state: shared room objects, actor slots, inventory order, and script cursor.
	// actorDescriptorCatalog: shared owner of the parallel actor-shape catalogs.
	// updateActorAnimationFrames: recovered animation update used while a new actor waits for graphics.
	// waitForRoomVerticalBlank: callback-aware clocked frame publishing actor and object state.
	// commitActionPromptUpdate: recovered action-prompt commit boundary.
	// refreshActionPrompts: recovered action-prompt refresh boundary.
	RoomScriptCommand05Executor(const ScoobyDooRom &rom, RuntimeState &state,
								const ActorAnimationDescriptorCatalog &actorDescriptorCatalog,
								Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<bool> &waitForRoomVerticalBlank,
								Common::Functor0<void> &commitActionPromptUpdate,
								Common::Functor0<void> &refreshActionPrompts)
		: _rom(rom), _state(state), _actorDescriptorCatalog(actorDescriptorCatalog),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank),
		  _commitActionPromptUpdate(commitActionPromptUpdate),
		  _refreshActionPrompts(refreshActionPrompts) {
	}

	// Relocates one room object and consumes its complete six-byte command record.
	// mode: zero scans the record; any nonzero value applies the relocation.
	//
	// Ghidra: executeRoomScriptCommand05 (0x000028A0). The 36-entry descriptor and movement-step catalogs
	// remain ROM-owned authored data. Four dynamic actor movement slots replace the original overlapping RAM
	// aliases, while the typed inventory list replaces its trailing -1 word.
	void executeRoomScriptCommand05(int mode);

private:
	static const uint8 kCompactActorMask = 0x08;
	static const int kDynamicActorCount = 4;
	static const int kDynamicActorFirstSlot = 2;
	static const int kDynamicActorSlotLimit = kDynamicActorFirstSlot + kDynamicActorCount;
	static const uint8 kFixedPositionMask = 0x02;
	static const int16 kInventoryRoomId = 1;
	static const uint8 kObjectTilePatchPendingMask = 0x01;
	static const int kRoomObjectRecordSize = 0x1A;
	static const uint8 kSuppressActionPromptRefreshMask = 0x04;

	// Returns false when host closure interrupted a synchronous wait (the caller must abort without consuming
	// the command record), true on normal completion.
	bool relocateFixedObject(RoomObject &roomObject, int16 newRoomId);

	// Returns true when host closure interrupted a synchronous wait (the caller must abort without consuming
	// the command record), false on normal completion. Mirrors the C# source's asymmetric true/false polarity
	// relative to RelocateFixedObject exactly.
	bool relocatePortableObject(RoomObject &roomObject, int objectIndex, uint16 encodedObjectIdentity,
								int16 newRoomId);

	// Returns true when host closure interrupted the wait, false otherwise.
	bool refreshActionPromptsAfterRelocation(uint16 encodedObjectIdentity);

	static int replaceIntegerCoordinate(int coordinate, int16 integer);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	const ActorAnimationDescriptorCatalog &_actorDescriptorCatalog;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	Common::Functor0<void> &_commitActionPromptUpdate;
	Common::Functor0<void> &_refreshActionPrompts;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_05_EXECUTOR_H
