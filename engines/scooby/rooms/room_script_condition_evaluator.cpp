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

#include "room_script_condition_evaluator.h"

namespace Scooby {

bool RoomScriptConditionEvaluator::evaluateRoomScriptCondition() {
	int commandOffset = _state.RoomScriptStartOffset;
	uint16 flags = _rom.readUInt16(commandOffset + 14);
	int16 left;

	// Ghidra 0x00002796-0x000027A9: load the selector and give left progress-bit and object-word sources
	// priority over the immediate field.
	if ((flags & 0x01) != 0) {
		// Ghidra 0x000027B2-0x000027CF: read the signed progress byte offset, test the selected bit modulo
		// eight, and normalize both bit outcomes to signed word zero or one.
		int16 byteOffset = _rom.readInt16(commandOffset + 6);
		int bitMask = 1 << (_rom.readUInt16(commandOffset + 8) & 0x07);
		left = 0;
		if ((_state.ProgressStateBytes[static_cast<std::size_t>(byteOffset)] & bitMask) != 0) {
			left = 1;
		}
	} else if ((flags & 0x02) != 0) {
		// Ghidra 0x000027D0-0x00002807: preserve both RoomId-special-case branches; every other
		// identity/field pair uses the wrapped and aligned typed room-object projection.
		uint16 fieldOffset = _rom.readUInt16(commandOffset + 6);
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 8);
		if (objectIdentity == 1) {
			if (fieldOffset == 6) {
				left = _state.RoomId;
			} else {
				left = readRoomObjectConditionWord(fieldOffset, objectIdentity);
			}
		} else {
			left = readRoomObjectConditionWord(fieldOffset, objectIdentity);
		}
	} else {
		// Ghidra 0x000027AA-0x000027B1: the default left source is the signed immediate at +6.
		left = _rom.readInt16(commandOffset + 6);
	}

	int16 right;

	// Ghidra 0x00002808-0x00002817: give right progress-bit and object-word sources priority over the
	// immediate field independently from the left selector.
	if ((flags & 0x04) != 0) {
		// Ghidra 0x00002820-0x0000283D: normalize both outcomes of the selected progress bit.
		int16 byteOffset = _rom.readInt16(commandOffset + 10);
		int bitMask = 1 << (_rom.readUInt16(commandOffset + 12) & 0x07);
		right = 0;
		if ((_state.ProgressStateBytes[static_cast<std::size_t>(byteOffset)] & bitMask) != 0) {
			right = 1;
		}
	} else if ((flags & 0x08) != 0) {
		// Ghidra 0x0000283E-0x0000285B: the right object source always uses the wrapped table projection and
		// has no RoomId special case.
		right = readRoomObjectConditionWord(_rom.readUInt16(commandOffset + 10),
											_rom.readUInt16(commandOffset + 12));
	} else {
		// Ghidra 0x00002818-0x0000281F: the default right source is the signed immediate at +0x0A.
		right = _rom.readInt16(commandOffset + 10);
	}

	// Ghidra 0x0000285C-0x0000286F: comparison bit four has first priority and tests equality.
	if ((flags & 0x10) != 0) {
		if (right != left) {
			return false;
		}

		return true;
	}

	// Ghidra 0x00002870-0x00002881: comparison bit five has second priority and tests signed left > right.
	if ((flags & 0x20) != 0) {
		if (left > right) {
			return true;
		}

		return false;
	}

	// Ghidra 0x00002882-0x00002893: comparison bit six has third priority and tests signed left < right.
	if ((flags & 0x40) != 0) {
		if (left < right) {
			return true;
		}

		return false;
	}

	// Ghidra 0x00002894-0x0000289F: no comparison bit selects inequality; direct managed returns replace the
	// original shared D3 result, terminal TST.W, and RTS for every comparison branch.
	if (right == left) {
		return false;
	}

	return true;
}

int16 RoomScriptConditionEvaluator::readRoomObjectConditionWord(uint16 fieldOffset,
																uint16 objectIdentity) const {
	uint16 tableByteOffset =
		static_cast<uint16>(fieldOffset + (objectIdentity - 3) * kRoomObjectRecordSize);
	tableByteOffset &= 0xFFFE;
	return _state.RoomObjects[tableByteOffset / kRoomObjectRecordSize].readScriptConditionWord(
		tableByteOffset % kRoomObjectRecordSize);
}
} // namespace Scooby