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
	// Size in bytes of one original room-object record (offsets +0x00 through +0x19).
	static const int kRecordSize = 0x1A;

	RoomObject()
		: tilePatchIndex(0), shapeIndex(0), inventoryGraphicIndex(0), roomId(0), positionIndex(0),
		  animationCountdown(0), scriptStateWords(3), fixedX(0), fixedY(0), suggestedVerb(0),
		  actionArgument(0), flags(0), fixedPositionIndex(0) {
		Common::fill(scriptStateWords.begin(), scriptStateWords.end(), 0);
	}

	// Record offset +0x00. Command 0x05 sets bit 15 when a visible portable
	// object leaves the active room; command 0x0B replaces it or sets both
	// high bits before queued tile-patch publication.
	int16 tilePatchIndex;

	// Record offset +0x02. One-based collision or actor-shape index, or zero for no shape.
	int16 shapeIndex;

	// Record offset +0x04. Inventory graphic index.
	int16 inventoryGraphicIndex;

	// Record offset +0x06 (g_wRoomObjectRoomIdStart). Room in which the object participates.
	int16 roomId;

	// Record offset +0x08. One-based room-position selector.
	int16 positionIndex;

	// Record offset +0x0A. Signed animation countdown.
	int16 animationCountdown;

	// Record offsets +0x0C-+0x11. Three indexed script-state words.
	Common::Array<int16> scriptStateWords;

	// Record offset +0x12. Signed fixed world X coordinate.
	int16 fixedX;

	// Record offset +0x14. Signed fixed world Y coordinate.
	int16 fixedY;

	// Record offset +0x16 high byte. Signed action icon.
	int8 suggestedVerb;

	// Record offset +0x17. Adjacent authored action byte.
	uint8 actionArgument;

	// Record offset +0x18 high byte (g_bRoomObjectFlagsStart). Object behavior flags.
	uint8 flags;

	// Record offset +0x19 (g_sbRoomObjectFixedPositionIndexStart). Signed dynamic-actor index.
	int8 fixedPositionIndex;

	// Reads one byte through the complete original record layout (offsets +0x00 through +0x19).
	uint8 readCommandByte(int byteOffset) const {
		uint16 word = readCommandWord(byteOffset & ~1);
		return (byteOffset & 1) == 0 ? static_cast<uint8>(word >> 8) : static_cast<uint8>(word);
	}

	// Reads one aligned word through the complete original record layout (even offsets +0x00 through +0x18).
	uint16 readCommandWord(int byteOffset) const {
		switch (byteOffset) {
		case 0x00:
			return static_cast<uint16>(tilePatchIndex);
		case 0x02:
			return static_cast<uint16>(shapeIndex);
		case 0x04:
			return static_cast<uint16>(inventoryGraphicIndex);
		case 0x06:
			return static_cast<uint16>(roomId);
		case 0x08:
			return static_cast<uint16>(positionIndex);
		case 0x0A:
			return static_cast<uint16>(animationCountdown);
		case 0x0C:
			return static_cast<uint16>(scriptStateWords[0]);
		case 0x0E:
			return static_cast<uint16>(scriptStateWords[1]);
		case 0x10:
			return static_cast<uint16>(scriptStateWords[2]);
		case 0x12:
			return static_cast<uint16>(fixedX);
		case 0x14:
			return static_cast<uint16>(fixedY);
		case 0x16:
			return static_cast<uint16>((static_cast<uint8>(suggestedVerb) << 8) | actionArgument);
		case 0x18:
			return static_cast<uint16>((flags << 8) | static_cast<uint8>(fixedPositionIndex));
		default:
			return static_cast<uint16>(scriptStateWords[static_cast<size_t>((byteOffset - 0x0C) /
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
			tilePatchIndex = static_cast<int16>(value);
			break;
		case 0x02:
			shapeIndex = static_cast<int16>(value);
			break;
		case 0x04:
			inventoryGraphicIndex = static_cast<int16>(value);
			break;
		case 0x06:
			roomId = static_cast<int16>(value);
			break;
		case 0x08:
			positionIndex = static_cast<int16>(value);
			break;
		case 0x0A:
			animationCountdown = static_cast<int16>(value);
			break;
		case 0x0C:
			scriptStateWords[0] = static_cast<int16>(value);
			break;
		case 0x0E:
			scriptStateWords[1] = static_cast<int16>(value);
			break;
		case 0x10:
			scriptStateWords[2] = static_cast<int16>(value);
			break;
		case 0x12:
			fixedX = static_cast<int16>(value);
			break;
		case 0x14:
			fixedY = static_cast<int16>(value);
			break;
		case 0x16:
			suggestedVerb = static_cast<int8>(value >> 8);
			actionArgument = static_cast<uint8>(value);
			break;
		case 0x18:
			flags = static_cast<uint8>(value >> 8);
			fixedPositionIndex = static_cast<int8>(value);
			break;
		default:
			scriptStateWords[static_cast<size_t>((byteOffset - 0x0C) / static_cast<int>(sizeof(
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
