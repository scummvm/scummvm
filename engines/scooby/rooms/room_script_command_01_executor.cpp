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

#include "room_script_command_01_executor.h"

namespace Scooby {

void RoomScriptCommand01Executor::executeRoomScriptCommand01(int mode, int16 interactionMode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x0000246A-0x0000246D: execution mode bypasses every condition branch.
	if (mode != 0) {
		goto AdvanceCursor;
	}

	// Ghidra 0x0000246E-0x00002479: a clear condition-evaluation gate takes the shared cursor branch.
	if ((_state.AutomaticInteractionFlags & kConditionEvaluationMask) == 0) {
		goto AdvanceCursor;
	}

	// Ghidra 0x0000247A-0x00002481: a different interaction mode also takes the shared cursor branch.
	if (interactionMode != _rom.readInt16(commandOffset + 2)) {
		goto AdvanceCursor;
	}

	// Ghidra 0x00002482-0x00002491: mode five clears room-behavior bit zero and branches past the independent
	// mode-six test to the secondary-selector field.
	if (interactionMode == 5) {
		_state.RoomBehaviorFlags &= static_cast<uint8>(~kRoomBehaviorModeMask);
		goto TestSecondarySelector;
	}

	// Ghidra 0x00002492-0x000024A1: mode six sets room-behavior bit zero; every other matching mode latches
	// the dispatch stop immediately without consulting the secondary-selector field.
	if (interactionMode != 6) {
		goto StopDispatch;
	}

	_state.RoomBehaviorFlags |= kRoomBehaviorModeMask;

TestSecondarySelector:
	// Ghidra 0x000024A2-0x000024A9: a zero secondary-selector field latches the dispatch stop.
	if (_rom.readInt16(commandOffset + 4) == 0) {
		goto StopDispatch;
	}

	// Ghidra 0x000024AA-0x000024B1: a nonzero current secondary interaction skips its zero-only setup.
	if (_state.SecondaryInteraction != 0) {
		goto RecheckInteractionMode;
	}

	// Ghidra 0x000024B2-0x000024C5: latch the zero-secondary path, then preserve both independent
	// interaction-bit-zero outcomes instead of assuming the second secondary read remains zero.
	_state.AutomaticInteractionFlags |= kSecondaryInteractionLatchMask;
	if ((_state.InteractionFlags & kAlternateInteractionMask) != 0) {
		goto RetestSecondaryInteraction;
	}

	// Ghidra 0x000024C6-0x000024D7: set alternate targeting, clear the complete secondary word, and take the
	// same signed cursor branch used by every failed condition.
	_state.InteractionFlags |= kAlternateInteractionMask;
	_state.SecondaryInteraction = 0;
	goto AdvanceCursor;

RetestSecondaryInteraction:
	// Ghidra 0x000024D8-0x000024E1: re-read the secondary word; zero advances, while nonzero continues.
	if (_state.SecondaryInteraction == 0) {
		goto AdvanceCursor;
	}

RecheckInteractionMode:
	// Ghidra 0x000024E2-0x000024E9: independently repeat the interaction-mode comparison.
	if (interactionMode != _rom.readInt16(commandOffset + 2)) {
		goto AdvanceCursor;
	}

	// Ghidra 0x000024EA-0x000024F7: independently re-read and compare the secondary interaction with record
	// field +4; a mismatch advances and an exact match falls through to the stop latch.
	if (_state.SecondaryInteraction != _rom.readInt16(commandOffset + 4)) {
		goto AdvanceCursor;
	}

StopDispatch:
	// Ghidra 0x000024F8-0x00002501: latch bit zero and return without advancing the command cursor.
	_state.AutomaticInteractionFlags |= kStopDispatchMask;
	return;

AdvanceCursor:
	// Ghidra 0x00002502-0x0000250B: sign-extend record field +6, add it to the original command start, and
	// return without adding a defensive progress check for zero or backward authored displacements.
	_state.RoomScriptStartOffset = commandOffset + _rom.readInt16(commandOffset + 6);
}
} // namespace Scooby