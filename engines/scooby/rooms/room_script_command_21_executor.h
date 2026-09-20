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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_21_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_21_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x21's per-actor path-stream installation.

namespace Scooby {

class RoomScriptCommand21Executor {
public:
	// Binds inline path records to their selected actor's traversal state.
	// rom: verified cartridge containing the variable-length command and path records.
	// state: shared script cursor, room objects, path arrays, and actor masks.
	RoomScriptCommand21Executor(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Installs an inline path stream and starts traversal for the selected actor.
	// mode: zero scans the variable-length record; nonzero installs all selected path state.
	//
	// Ghidra: executeRoomScriptCommand21 (0x00004944). Unlike the preceding actor-selection commands, dynamic
	// selection sign-extends the room object's fixed-position byte before indexing every six-slot path array.
	// The record extent is also a signed word even though authored extents are positive.
	void executeRoomScriptCommand21(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_21_EXECUTOR_H
