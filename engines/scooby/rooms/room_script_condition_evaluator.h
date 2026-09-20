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

#ifndef SCOOBY_ROOM_SCRIPT_CONDITION_EVALUATOR_H
#define SCOOBY_ROOM_SCRIPT_CONDITION_EVALUATOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Evaluates the shared signed condition record used by room-script commands 03 and 04.

namespace Scooby {

class RoomScriptConditionEvaluator {
public:
	// Binds condition records to verified cartridge data and their typed mutable state sources.
	// rom: verified cartridge containing the current condition record.
	// state: current script cursor, progress bytes, room objects, and active room identity.
	RoomScriptConditionEvaluator(const ScoobyDooRom &rom, RuntimeState &state)
		: _rom(rom), _state(state) {
	}

	// Reads both selected signed operands and applies the record's prioritized comparison.
	// Returns true when the recovered condition sets a nonzero result.
	//
	// Ghidra: evaluateRoomScriptCondition (0x00002796). Record field +0x0E selects each operand from an
	// immediate word, one progress bit, or an aligned room-object word. The left object source alone maps
	// identity one and field six to g_wRoomId. Comparison bits retain equality, signed greater-than, signed
	// less-than, then inequality priority.
	bool evaluateRoomScriptCondition();

private:
	static const int kRoomObjectRecordSize = 0x1A;

	int16 readRoomObjectConditionWord(uint16 fieldOffset, uint16 objectIdentity) const;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_CONDITION_EVALUATOR_H
