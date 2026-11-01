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

#include "interaction_resolver.h"

#include "common/scummsys.h"

namespace Scooby {

int16 InteractionResolver::findAdjustedCursorInteraction() {
	int16 worldX = static_cast<int16>(_state.cursorX + _state.cameraX);
	int16 worldY = static_cast<int16>(_state.cursorY + _state.cameraY);
	int16 tileX = static_cast<int16>(
		static_cast<uint16>(static_cast<int16>(worldX + 8)) >> 3);
	int16 tileY = static_cast<int16>(
		static_cast<uint16>(static_cast<int16>(worldY - 8)) >> 3);
	int16 actorX = static_cast<int16>(_state.actorXFixedCoordinates[1] >> 16);
	int16 actorY = static_cast<int16>(_state.actorYFixedCoordinates[1] >> 16);
	int32 episodeDescriptorOffset = _state.activeEpisodeDescriptorOffset;
	int shapeOffsetTableOffset = static_cast<int>(
		_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeOffsetTableField));
	int shapeDataOffset = static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeDataField));
	int16 selectedInteraction = 0;
	if ((_state.actorVisibilityBlockFlags & 0x02) == 0 && (_state.activeActorFlags & 0x02) != 0 &&
		worldX <= static_cast<int16>(actorX + _rom.readInt16(0x3259A)) &&
		worldX >= static_cast<int16>(actorX - _rom.readInt16(0x32552)) &&
		worldY <= static_cast<int16>(actorY + _rom.readInt16(0x3250A)) &&
		worldY >= static_cast<int16>(actorY - _rom.readInt16(0x324C2))) {
		selectedInteraction = 2;
	}

	for (size_t index = 0; index < _state.roomObjects.size(); index++) {
		const RoomObject &roomObject = _state.roomObjects[index];
		if (roomObject.roomId != _state.roomId) {
			continue;
		}

		bool containsCursor = (roomObject.flags & 0x02) == 0
								  ? containsDynamicObject(roomObject, tileX, tileY, shapeOffsetTableOffset,
														  shapeDataOffset)
								  : containsFixedObject(
										roomObject, static_cast<int16>(_state.cursorX + _state.cameraX),
										static_cast<int16>(_state.cursorY + _state.cameraY));
		if (containsCursor) {
			selectedInteraction = static_cast<int16>(index + 3);
		}
	}

	return selectedInteraction;
}

int16 InteractionResolver::findAutomaticInteraction() {
	int16 tileX = static_cast<int16>(
		static_cast<uint16>(_state.actorXFixedCoordinates[0] >> 16) >> 3);
	int16 tileY = static_cast<int16>(
		static_cast<uint16>(_state.actorYFixedCoordinates[0] >> 16) >> 3);
	int32 episodeDescriptorOffset = _state.activeEpisodeDescriptorOffset;
	int shapeOffsetTableOffset = static_cast<int>(
		_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeOffsetTableField));
	int shapeDataOffset = static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeDataField));
	int16 selectedInteraction = 0;
	for (size_t index = 0; index < _state.roomObjects.size(); index++) {
		const RoomObject &roomObject = _state.roomObjects[index];
		if (roomObject.roomId == _state.roomId && (roomObject.flags & 0x02) == 0 &&
			containsDynamicObject(roomObject, tileX, tileY, shapeOffsetTableOffset, shapeDataOffset)) {
			selectedInteraction = static_cast<int16>(index + 3);
		}
	}

	return selectedInteraction;
}

bool InteractionResolver::containsDynamicObject(const RoomObject &roomObject, int16 x,
												int16 y,
												int shapeOffsetTableOffset, int shapeDataOffset) const {
	if (roomObject.shapeIndex == 0) {
		return false;
	}

	int16 tableByteOffset = static_cast<int16>(
		(roomObject.shapeIndex - 1) * static_cast<int>(sizeof(uint32)));
	int shapeOffset = static_cast<int>(_rom.readUInt32(shapeOffsetTableOffset + tableByteOffset));
	int shapeAddress = shapeDataOffset + shapeOffset;
	int16 left = _rom.readInt16(shapeAddress);
	int16 top = _rom.readInt16(shapeAddress + 2);
	int16 right = static_cast<int16>(left + _rom.readInt16(shapeAddress + 4));
	int16 bottom = static_cast<int16>(top + _rom.readInt16(shapeAddress + 6));
	return x >= left && y >= top && x < right && y < bottom;
}

bool InteractionResolver::getDynamicObjectTileBounds(const RoomObject &roomObject,
													 Common::Rect &tileBounds) const {
	// Managed-only mouse support: the same shape lookup as containsDynamicObject.
	if (roomObject.shapeIndex == 0) {
		return false;
	}

	int32 episodeDescriptorOffset = _state.activeEpisodeDescriptorOffset;
	int shapeOffsetTableOffset = static_cast<int>(
		_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeOffsetTableField));
	int shapeDataOffset = static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeDataField));
	int shapeAddress = shapeDataOffset + static_cast<int>(_rom.readUInt32(
		shapeOffsetTableOffset + (roomObject.shapeIndex - 1) * static_cast<int>(sizeof(uint32))));
	int16 left = _rom.readInt16(shapeAddress);
	int16 top = _rom.readInt16(shapeAddress + 2);
	tileBounds = Common::Rect(left, top, static_cast<int16>(left + _rom.readInt16(shapeAddress + 4)),
							  static_cast<int16>(top + _rom.readInt16(shapeAddress + 6)));
	return true;
}

bool InteractionResolver::containsFixedObject(const RoomObject &roomObject, int16 worldX,
											  int16 worldY) const {
	int shapeIndex = roomObject.shapeIndex - 1;
	int positionIndex = roomObject.fixedPositionIndex + 2;
	int16 actorX = static_cast<int16>(_state.actorXFixedCoordinates[positionIndex] >> 16);
	int16 actorY = static_cast<int16>(_state.actorYFixedCoordinates[positionIndex] >> 16);
	int16 left = static_cast<int16>(actorX - _rom.readInt16(0x32550 + shapeIndex * 2));
	int16 right = static_cast<int16>(actorX + _rom.readInt16(0x32598 + shapeIndex * 2));
	int16 top = static_cast<int16>(actorY - _rom.readInt16(0x324C0 + shapeIndex * 2));
	int16 bottom = static_cast<int16>(actorY + _rom.readInt16(0x32508 + shapeIndex * 2));
	return worldX >= left && worldX <= right && worldY >= top && worldY <= bottom;
}
} // namespace Scooby
