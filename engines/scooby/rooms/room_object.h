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

#ifndef SCOOBY_ROOM_OBJECT_H
#define SCOOBY_ROOM_OBJECT_H

#include "common/scummsys.h"
#include "common/algorithm.h"

#include "common/array.h"

// Carries one mutable 0x1A-byte record from Ghidra g_aRoomObjectStates at
// 0xFF1200. Authored fields are decoded from ROM while countdown, script,
// and actor slots begin with the values written by LoadRoomObjectTable.

namespace Scooby {

class RoomObject {
public:
	RoomObject()
		: TilePatchIndex(0), ShapeIndex(0), InventoryGraphicIndex(0), RoomId(0), PositionIndex(0),
		  AnimationCountdown(0), ScriptStateWords(3), FixedX(0), FixedY(0), ActionIcon(0),
		  ActionArgument(0), Flags(0), FixedPositionIndex(0) {
		Common::fill(ScriptStateWords.begin(), ScriptStateWords.end(), 0);
	}

	// Record offset +0x00. Command 0x05 sets bit 15 when a visible portable
	// object leaves the active room; command 0x0B replaces it or sets both
	// high bits before queued tile-patch publication.
	int16 TilePatchIndex;

	// Record offset +0x02. One-based collision or actor-shape index, or zero for no shape.
	int16 ShapeIndex;

	// Record offset +0x04. Inventory graphic index.
	int16 InventoryGraphicIndex;

	// Record offset +0x06 (g_wRoomObjectRoomIdStart). Room in which the object participates.
	int16 RoomId;

	// Record offset +0x08. One-based room-position selector.
	int16 PositionIndex;

	// Record offset +0x0A. Signed animation countdown.
	int16 AnimationCountdown;

	// Record offsets +0x0C-+0x11. Three indexed script-state words.
	Common::Array<int16> ScriptStateWords;

	// Record offset +0x12. Signed fixed world X coordinate.
	int16 FixedX;

	// Record offset +0x14. Signed fixed world Y coordinate.
	int16 FixedY;

	// Record offset +0x16 high byte. Signed action icon.
	int8 ActionIcon;

	// Record offset +0x17. Adjacent authored action byte.
	uint8 ActionArgument;

	// Record offset +0x18 high byte (g_bRoomObjectFlagsStart). Object behavior flags.
	uint8 Flags;

	// Record offset +0x19 (g_sbRoomObjectFixedPositionIndexStart). Signed dynamic-actor index.
	int8 FixedPositionIndex;

	// Reads one byte through the complete original record layout (offsets +0x00 through +0x19).
	uint8 readCommandByte(int byteOffset) const {
		uint16 word = readCommandWord(byteOffset & ~1);
		return (byteOffset & 1) == 0 ? static_cast<uint8>(word >> 8) : static_cast<uint8>(word);
	}

	// Reads one aligned word through the complete original record layout (even offsets +0x00 through +0x18).
	uint16 readCommandWord(int byteOffset) const {
		switch (byteOffset) {
		case 0x00:
			return static_cast<uint16>(TilePatchIndex);
		case 0x02:
			return static_cast<uint16>(ShapeIndex);
		case 0x04:
			return static_cast<uint16>(InventoryGraphicIndex);
		case 0x06:
			return static_cast<uint16>(RoomId);
		case 0x08:
			return static_cast<uint16>(PositionIndex);
		case 0x0A:
			return static_cast<uint16>(AnimationCountdown);
		case 0x0C:
			return static_cast<uint16>(ScriptStateWords[0]);
		case 0x0E:
			return static_cast<uint16>(ScriptStateWords[1]);
		case 0x10:
			return static_cast<uint16>(ScriptStateWords[2]);
		case 0x12:
			return static_cast<uint16>(FixedX);
		case 0x14:
			return static_cast<uint16>(FixedY);
		case 0x16:
			return static_cast<uint16>((static_cast<uint8>(ActionIcon) << 8) | ActionArgument);
		case 0x18:
			return static_cast<uint16>((Flags << 8) | static_cast<uint8>(FixedPositionIndex));
		default:
			return static_cast<uint16>(ScriptStateWords[static_cast<std::size_t>((byteOffset - 0x0C) /
																				 static_cast<int>(sizeof(int16)))]);
		}
	}

	// Projects one aligned word from the typed record for room-script condition evaluation.
	int16 readScriptConditionWord(int byteOffset) const {
		return static_cast<int16>(readCommandWord(byteOffset));
	}

	// Writes one aligned word through the complete original record layout.
	void writeCommandWord(int byteOffset, uint16 value) {
		switch (byteOffset) {
		case 0x00:
			TilePatchIndex = static_cast<int16>(value);
			break;
		case 0x02:
			ShapeIndex = static_cast<int16>(value);
			break;
		case 0x04:
			InventoryGraphicIndex = static_cast<int16>(value);
			break;
		case 0x06:
			RoomId = static_cast<int16>(value);
			break;
		case 0x08:
			PositionIndex = static_cast<int16>(value);
			break;
		case 0x0A:
			AnimationCountdown = static_cast<int16>(value);
			break;
		case 0x0C:
			ScriptStateWords[0] = static_cast<int16>(value);
			break;
		case 0x0E:
			ScriptStateWords[1] = static_cast<int16>(value);
			break;
		case 0x10:
			ScriptStateWords[2] = static_cast<int16>(value);
			break;
		case 0x12:
			FixedX = static_cast<int16>(value);
			break;
		case 0x14:
			FixedY = static_cast<int16>(value);
			break;
		case 0x16:
			ActionIcon = static_cast<int8>(value >> 8);
			ActionArgument = static_cast<uint8>(value);
			break;
		case 0x18:
			Flags = static_cast<uint8>(value >> 8);
			FixedPositionIndex = static_cast<int8>(value);
			break;
		default:
			ScriptStateWords[static_cast<std::size_t>((byteOffset - 0x0C) / static_cast<int>(sizeof(
																				int16)))] =
				static_cast<int16>(value);
			break;
		}
	}

	// Writes one byte while retaining the adjacent byte in the same original big-endian word.
	void writeCommandByte(int byteOffset, uint8 value) {
		int alignedOffset = byteOffset & ~1;
		uint16 word = readCommandWord(alignedOffset);
		writeCommandWord(alignedOffset, (byteOffset & 1) == 0
											? static_cast<uint16>((word & 0x00FF) | (value << 8))
											: static_cast<uint16>((word & 0xFF00) | value));
	}
};
} // namespace Scooby

#endif // SCOOBY_ROOM_OBJECT_H
