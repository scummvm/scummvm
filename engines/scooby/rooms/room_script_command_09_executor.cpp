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

#include "room_script_command_09_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand09Executor::executeRoomScriptCommand09(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002D5E-0x00002D61: inherited Z from dispatcher mode zero skips every state mutation.
	if (mode != 0) {
		// Ghidra 0x00002D62-0x00002D7F: retain the unsigned word identity multiply, low-word wrapping add,
		// signed field +5 displacement, and the independent +0x16 base before resolving the typed byte.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		RoomObjectByteLocation location =
			resolveRoomObjectByte(objectIdentity, static_cast<int8>(_rom.readByte(commandOffset + 5)));

		// Ghidra 0x00002D80-0x00002D99: both identity outcomes retain the same resolved byte. A match
		// sign-extends its old value into the pending icon and draws mode zero before the replacement.
		if (objectIdentity == static_cast<uint16>(_state.CurrentInteraction)) {
			_state.PendingActionIcon =
				static_cast<int8>(location.TargetRoomObject.readCommandByte(location.ByteOffset));
			_drawSelectedActionIcon(0);
		}

		// Ghidra 0x00002D9A-0x00002DA7: replace the resolved byte, then retain both outcomes of the
		// active-icon test rather than narrowing the displacement to the authored action-icon field.
		location.TargetRoomObject.writeCommandByte(location.ByteOffset, _rom.readByte(commandOffset + 4));
		if (_state.ActiveActionIcon != 0) {
			// Ghidra 0x00002DA8-0x00002DB9: queue the complete active word and clear it without drawing.
			_state.PendingActionIcon = _state.ActiveActionIcon;
			_state.ActiveActionIcon = 0;
		}
	}

	// Ghidra 0x00002DBA-0x00002DBF: every mode and state branch consumes the six-byte command.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

RoomScriptCommand09Executor::RoomObjectByteLocation RoomScriptCommand09Executor::resolveRoomObjectByte(
	uint16 objectIdentity, int8 displacement) {
	uint16 recordOffset = static_cast<uint16>((objectIdentity - 3) * kRoomObjectRecordSize);
	int16 wrappedRecordOffset = static_cast<int16>(recordOffset + displacement);
	int tableByteOffset = kActionIconByteOffset + wrappedRecordOffset;
	int objectIndex = tableByteOffset / kRoomObjectRecordSize;
	int byteOffset = tableByteOffset % kRoomObjectRecordSize;
	if (byteOffset < 0) {
		objectIndex--;
		byteOffset += kRoomObjectRecordSize;
	}

	return RoomObjectByteLocation(_state.RoomObjects[static_cast<std::size_t>(objectIndex)], byteOffset);
}
} // namespace Scooby