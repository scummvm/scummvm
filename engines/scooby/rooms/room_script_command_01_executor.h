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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_01_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_01_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns the conditional interaction scan and cursor control selected by room-script command 0x01.

namespace Scooby {

class RoomScriptCommand01Executor {
public:
	// Binds command-record reads to the interaction state changed by every recovered branch.
	// rom: verified cartridge containing the current command record.
	// state: shared interaction flags, secondary selection, and script cursor.
	RoomScriptCommand01Executor(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Evaluates the interaction condition in scan mode or follows its signed record displacement.
	// mode: zero evaluates the condition; nonzero skips directly by the record displacement.
	// interactionMode: stable D7 interaction-mode snapshot retained by the scan dispatcher.
	//
	// Ghidra: executeRoomScriptCommand01 (0x0000246A).
	void executeRoomScriptCommand01(int mode, int16 interactionMode);

private:
	static const uint8 kAlternateInteractionMask = 0x01;
	static const uint8 kConditionEvaluationMask = 0x02;
	static const uint8 kRoomBehaviorModeMask = 0x01;
	static const uint8 kSecondaryInteractionLatchMask = 0x20;
	static const uint8 kStopDispatchMask = 0x01;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_01_EXECUTOR_H
