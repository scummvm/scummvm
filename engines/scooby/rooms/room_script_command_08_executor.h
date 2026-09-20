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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_08_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_08_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"

// Owns word-width value transfers between progress state and complete room-object records.

namespace Scooby {

class RoomScriptCommand08Executor {
public:
	// Binds command value selection and arithmetic to their ROM, state, and deterministic-random owners.
	// rom: verified cartridge containing the six-word command records.
	// state: shared progress bytes, room-object records, and script cursor.
	// random: shared recovered random sequence used by the optional scaling branch.
	RoomScriptCommand08Executor(const ScoobyDooRom &rom, RuntimeState &state,
								DeterministicRandom &random)
		: _rom(rom), _state(state), _random(random) {
	}

	// Transfers one selected value through the command's complete source and destination operation matrix.
	// mode: zero scans past the command; nonzero applies its value transfer.
	//
	// Ghidra: executeRoomScriptCommand08 (0x00002C56). Room-object addresses retain the original wrapped word
	// sum and even alignment, including offsets that cross into a neighboring record. The progress-word XOR
	// uses the command destination held in D3; raw instruction 0x00002D08 instead indexes through inherited
	// A3.W, which neither dispatcher establishes.
	void executeRoomScriptCommand08(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	// Preserves a mutable reference to the resolved room-object record and its aligned command-word offset,
	// replacing the C# "(RoomObject RoomObject, int ByteOffset)" tuple return.
	struct RoomObjectWordAddress {
		RoomObject &TargetRoomObject;
		int ByteOffset;

		RoomObjectWordAddress(RoomObject &targetRoomObject, int byteOffset)
			: TargetRoomObject(targetRoomObject), ByteOffset(byteOffset) {
		}
	};

	RoomObjectWordAddress resolveRoomObjectWordAddress(uint16 objectIdentity, uint16 fieldOffset);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	DeterministicRandom &_random;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_08_EXECUTOR_H
