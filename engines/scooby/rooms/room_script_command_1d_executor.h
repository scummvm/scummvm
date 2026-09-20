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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_1D_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_1D_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x1D's actor-animation hold projection into episode progress state.

namespace Scooby {

class RoomScriptCommand1DExecutor {
public:
	// Binds command records to the shared room-object, animation-hold, and progress owners.
	// rom: verified cartridge containing command fields.
	// state: shared script cursor, room objects, actor hold mask, and progress state.
	RoomScriptCommand1DExecutor(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Mirrors the selected actor's animation-hold bit into progress byte-zero bit three.
	// mode: zero scans the four-byte record; nonzero evaluates every actor-identity branch.
	//
	// Ghidra: executeRoomScriptCommand1D (0x00002510). Object-selected actors retain the signed low word of
	// the native record offset and the fixed-position byte's unsigned modulo-eight bit.
	void executeRoomScriptCommand1D(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_1D_EXECUTOR_H
