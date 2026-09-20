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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_20_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_20_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x20's selected-actor movement cancellation or loop-exit request.

namespace Scooby {

class RoomScriptCommand20Executor {
public:
	// Binds actor identity records to movement and one-shot animation-loop state.
	// rom: verified cartridge containing command fields.
	// state: shared script cursor, room objects, movement mask, and loop-exit mask.
	RoomScriptCommand20Executor(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Cancels selected active movement or requests an animation-loop exit when movement is clear.
	// mode: zero scans the four-byte record; nonzero evaluates every identity and state branch.
	//
	// Ghidra: executeRoomScriptCommand20 (0x000048F8). The movement-active and movement-clear branches
	// deliberately update different masks without changing the other one.
	void executeRoomScriptCommand20(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_20_EXECUTOR_H
