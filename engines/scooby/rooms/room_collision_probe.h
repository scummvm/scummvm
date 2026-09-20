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

#ifndef SCOOBY_ROOM_COLLISION_PROBE_H
#define SCOOBY_ROOM_COLLISION_PROBE_H

#include "common/scummsys.h"

#include "room_collision_probe_result.h"
#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Resolves room-space points against the decoded collision map and authored per-pixel masks.

namespace Scooby {

class RoomCollisionProbe {
public:
	// Binds collision-mask decoding to the verified cartridge and current decoded room map.
	RoomCollisionProbe(const ScoobyDooRom &rom, RuntimeState &state) : _rom(rom), _state(state) {
	}

	// Ghidra: probeLeadActorCollision (0x000073D6). Tests a lead-actor candidate point and retains the
	// original D0 word needed after a collision.
	// candidateX/candidateY: signed 16.16 candidate coordinates. maximumX/maximumY: inclusive signed
	// room-space bounds.
	RoomCollisionProbeResult probeLeadActorCollision(int candidateX, int candidateY, int16 maximumX,
													 int16 maximumY) const;

	// Ghidra: probeStagedCollision (0x00007476). Tests one staged integer point against the active room's
	// per-pixel collision mask. Returns true when the point is negative or selects an occupied collision nibble.
	bool probeStagedCollision(int16 x, int16 y) const;

private:
	static const int kCollisionCellShapeDescriptorsOffset = 0x2AF8E;
	static const uint16 kCollisionCellDescriptorIndexMask = 0x7FFF;
	static const uint16 kCollisionDescriptorHorizontalOrientationMask = 0x0800;
	static const uint16 kCollisionDescriptorShapeIndexMask = 0x07FF;
	static const uint16 kCollisionDescriptorVerticalOrientationMask = 0x1000;
	static const int kCollisionShapeByteCount = 0x20;
	static const int kCollisionShapeRowMasksOffset = 0x2B0BA;
	static const uint32 kCollisionShapePixelMask = 0x0F;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_COLLISION_PROBE_H
