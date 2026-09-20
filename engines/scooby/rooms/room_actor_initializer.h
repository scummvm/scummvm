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

#ifndef SCOOBY_ROOM_ACTOR_INITIALIZER_H
#define SCOOBY_ROOM_ACTOR_INITIALIZER_H

#include "actor_animation_descriptor_catalog.h"
#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Initializes the active room's built-in and object-backed actor slots.

namespace Scooby {

class RoomActorInitializer {
public:
	// Binds room actor initialization to the authored shape catalogs and shared actor state.
	// rom: verified cartridge containing the parallel descriptor and movement-step catalogs.
	// state: shared room-object and actor-slot state.
	RoomActorInitializer(const ScoobyDooRom &rom, RuntimeState &state)
		: _descriptorCatalog(rom), _state(state) {
	}

	// Resets transient actor state and assigns active fixed-position objects to slots two through five.
	//
	// Ghidra: initializeRoomActors (0x00007A8C). The object-table loop is inclusive in the original and the
	// descriptor lookup scans all 36 entries for each eligible object. Managed iteration covers the same
	// complete decoded table, preserves every no-match branch, and stops only after all four dynamic slots have
	// been assigned.
	void initializeRoomActors();

private:
	static const int kActorCount = 6;
	static const uint8 kCompactActorMask = 0x08;
	static const int kDynamicActorFirstSlot = 2;
	static const int kDynamicActorSlotLimit = kActorCount;
	static const uint8 kFixedPositionMask = 0x02;

	ActorAnimationDescriptorCatalog _descriptorCatalog;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_ACTOR_INITIALIZER_H
