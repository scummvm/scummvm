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

#ifndef SCOOBY_INTERACTION_RESOLVER_H
#define SCOOBY_INTERACTION_RESOLVER_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/rooms/room_object.h"
#include "scooby/runtime/runtime_state.h"

// Resolves cursor and actor positions against the room-object collision data read by Ghidra RunAdventure at
// 0x00000BF4.

namespace Scooby {

class InteractionResolver {
public:
	// Binds interaction hit testing to the verified ROM and current room state.
	// rom: verified cartridge address space containing collision tables.
	// state: shared runtime state carrying room objects and coordinates.
	InteractionResolver(const ScoobyDooRom &rom,
						RuntimeState &state) : _rom(rom), _state(state) {
	}

	// Finds the highest-priority interaction under the adjusted room-space cursor.
	// Returns the original one-based interaction identifier, or zero when no object is targeted.
	//
	// Ghidra: RunAdventure at 0x00000EA0-0x00001065 resolves episode descriptor fields +0x24 and +0x1C before
	// traversing the complete room-object table. Dynamic shapes use half-open bounds; fixed actor-backed
	// shapes retain their independent inclusive bounds.
	int16 findAdjustedCursorInteraction();

	// Finds the room interaction intersecting actor zero's current tile.
	// Returns the original one-based interaction identifier, or zero when no object is intersected.
	//
	// Ghidra: RunAdventure at 0x000012CC-0x00001359 reloads the same two active-episode shape pointers before
	// scanning every object through the inclusive final runtime index.
	int16 findAutomaticInteraction();

private:
	static const int kEpisodeRoomShapeDataField = 0x1C;
	static const int kEpisodeRoomShapeOffsetTableField = 0x24;

	bool containsDynamicObject(const RoomObject &roomObject, int16 x, int16 y,
							   int shapeOffsetTableOffset, int shapeDataOffset) const;
	bool containsFixedObject(const RoomObject &roomObject, int16 worldX,
							 int16 worldY) const;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_INTERACTION_RESOLVER_H
