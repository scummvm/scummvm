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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_02_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_02_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Registers the interaction-action menu entry embedded in room-script command 02.

namespace Scooby {

class RoomScriptCommand02Executor {
public:
	// Binds menu-entry registration to the verified ROM, active episode descriptor, and action registry.
	// rom: verified cartridge containing episode descriptors and command records.
	// state: current script cursor, active episode descriptor, gate flags, and action registry.
	RoomScriptCommand02Executor(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Conditionally registers one menu choice, then follows the record's signed cursor displacement.
	// mode: zero scans and may register the entry; nonzero only advances the cursor.
	//
	// Ghidra: executeRoomScriptCommand02 (0x000025AC). Active episode descriptor field +0x20 is 0x0014199C for
	// episode zero and 0x001AF33C for episode one. The original stores five parallel arrays;
	// InteractionActionRegistry preserves each indexed relationship as one value without retaining mutable
	// cartridge pointers.
	void executeRoomScriptCommand02(int mode);

private:
	static const int kInteractionTextBaseField = 0x20;
	static const int kNestedScriptField = 0x0E;
	static const uint8 kRegistrationEnabledMask = 0x04;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_02_EXECUTOR_H
