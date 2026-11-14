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

#include "scooby/rooms/scene_loader.h"

#include "common/array.h"

#include "room_object.h"
#include "scooby/rooms/scene_data.h"
#include "scooby/common/make_unique.h"
#include "scooby/common/span.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/types.h"

namespace Scooby {
void RoomSceneLoader::loadRoomScene() {
	// Ghidra 0x0000758A-0x000075AB: clear the complete pattern/layer destination and all four scroll words.
	_scene.clearContent();
	_state.cameraX = 0;
	_state.cameraY = 0;
	_state.foregroundHorizontalScroll = 0;
	_state.backgroundHorizontalScroll = 0;
	_state.foregroundVerticalScroll = 0;
	_state.backgroundVerticalScroll = 0;

	// Ghidra 0x000075AC-0x0000761B: restore shared colors, classify every object against the selected room,
	// and copy that room's second 32-color palette through the signed room-index path.
	Common::Array<PaletteColor> sharedPalette =
		readPalette(_rom, kDefaultSharedPaletteOffset, 32);
	Common::copy(sharedPalette.begin(), sharedPalette.end(), _state.targetPalette.begin());

	for (RoomObject &roomObject : _state.roomObjects) {
		roomObject.flags = roomObject.roomId == _state.roomId
							   ? static_cast<uint8>(roomObject.flags | 0x01)
							   : static_cast<uint8>(roomObject.flags & 0xFE);
	}

	int16 roomIndex = static_cast<int16>(_state.roomId - 2);
	int32 descriptorOffset = _state.activeEpisodeDescriptorOffset;
	int paletteOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodePaletteBaseField)) +
						roomIndex * kRoomPaletteByteStride + kRoomPaletteOffsetWithinRecord;
	Common::Array<PaletteColor> roomPalette =
		readPalette(_rom, paletteOffset, 32);
	Common::copy(roomPalette.begin(), roomPalette.end(), _state.targetPalette.begin() + 32);

	// Ghidra 0x0000761C-0x0000762B: every room load clears the foreground override and all palette ranges.
	_state.interactionFlags &= 0xBF;
	_state.animatedPaletteRangeFlags = 0;

	// Ghidra 0x0000762C-0x00007675: select the 20-byte room record with unsigned word multiplication,
	// decode its pattern stream, and retain the distinct under-0x400 minimum-allocation branch.
	int roomRecordOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeRoomRecordTableField)) +
						   static_cast<uint16>(roomIndex) * kRoomRecordSize;
	int roomTileStreamOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeAssetBaseField)) +
							   static_cast<int>(_rom.readUInt32(roomRecordOffset + 4));
	Common::Array<uint8> roomTiles =
		decompressRingLz(
			_rom, roomTileStreamOffset + static_cast<int>(sizeof(uint32)));
	int reservedRoomTileByteCount = MAX(static_cast<int>(roomTiles.size()), kMinimumRoomTileByteCount);
	int transferredRoomTileByteCount =
		(static_cast<uint16>(reservedRoomTileByteCount) >> 1) * static_cast<int>(sizeof(uint16));
	Common::Array<uint8> publishedRoomTiles(static_cast<size_t>(transferredRoomTileByteCount), 0);
	int copyByteCount = MIN(static_cast<int>(roomTiles.size()),
								 static_cast<int>(publishedRoomTiles.size()));
	Common::copy(roomTiles.begin(), roomTiles.begin() + copyByteCount, publishedRoomTiles.begin());
	_scene.overwritePackedPatternBytes(makeSpan(publishedRoomTiles), 0);

	// Ghidra 0x00007676-0x000076B1: derive the interface base, generate all 95 mask tiles, discard the
	// write-only end-address value after accounting for it, and derive the following prompt destination.
	uint16 patternByteOffset = static_cast<uint16>(reservedRoomTileByteCount);
	_state.interfaceTileAttributes = static_cast<uint16>(((patternByteOffset >> 5) - 0x20) |
														 kPriorityAttribute);
	uint16 maskTileByteOffset =
		static_cast<uint16>(((_state.interfaceTileAttributes & 0x07FF) + 0x20) << 5);
	Common::Array<uint8> dialogueMaskTiles = colorise4bppMaskTiles(_rom, 0x0F, 0x01);
	_scene.overwritePackedPatternBytes(makeSpan(dialogueMaskTiles), maskTileByteOffset);
	patternByteOffset += kMaskTileByteCount;
	_state.actionPromptTileAttributes = static_cast<uint16>((patternByteOffset >> 5) |
															kPriorityAttribute);

	// Ghidra 0x000076B2-0x0000773B: publish the prompt stream and the deliberately overlapping cursor,
	// tall-interface, and short-interface pattern assets while retaining each derived tile base.
	Common::Array<uint8> actionPromptTiles =
		decompressRingLz(_rom, kActionPromptTilesOffset);
	uint16 actionPromptWordCount =
		static_cast<uint16>(static_cast<int>(actionPromptTiles.size()) >> 1);
	int actionPromptByteCount = actionPromptWordCount * static_cast<int>(sizeof(uint16));
	_scene.overwritePackedPatternBytes(
		makeSpan(actionPromptTiles).slice(0, static_cast<size_t>(actionPromptByteCount)),
		patternByteOffset);
	patternByteOffset += actionPromptWordCount + actionPromptWordCount;
	_state.cursorSpriteTileAttributes = static_cast<uint16>((patternByteOffset >> 5) |
															kPriorityAttribute);
	_scene.overwritePackedPatternBytes(_rom.readBytes(kCursorSpriteTilesOffset, kCursorSpriteTileByteCount),
									   patternByteOffset);
	patternByteOffset += kCursorSpriteTileByteCount;
	_state.tallInterfaceSpriteTileIndex = static_cast<uint16>(patternByteOffset >> 5);
	_scene.overwritePackedPatternBytes(
		_rom.readBytes(kTallInterfaceSpriteTilesOffset, kTallInterfaceSpriteTileByteCount), patternByteOffset);
	patternByteOffset += kTallInterfaceSpriteAdvanceByteCount;
	_state.shortInterfaceSpriteTileIndex = static_cast<uint16>(patternByteOffset >> 5);
	_scene.overwritePackedPatternBytes(
		_rom.readBytes(kShortInterfaceSpriteTilesOffset, kShortInterfaceSpriteTileByteCount),
		patternByteOffset);
	patternByteOffset += kShortInterfaceSpriteTileByteCount;
	_state.blankTileCell = static_cast<uint16>((patternByteOffset >> 5) | kPriorityAttribute);

	// Ghidra 0x0000773C-0x00007759: materialize the all-one blank tile and reserve the following tile as
	// the dynamic room-script pattern base.
	Common::Array<uint8> blankTile(static_cast<size_t>(kBlankTileByteCount),
								   static_cast<uint8>(0x11));
	_scene.overwritePackedPatternBytes(makeSpan(blankTile), patternByteOffset);
	_state.roomDynamicTileBaseAttribute = static_cast<uint16>(((patternByteOffset >> 5) + 1) |
															  kPriorityAttribute);

	// Ghidra 0x0000775A-0x000077AB: the cleared logical scene subsumes the 0xD000 name-table fill; build
	// all six 64-cell lower-interface rows, including each row's 61-copy, 3-word skip, and 3-cell fill branch.
	uint16 repeatedInterfaceCell =
		static_cast<uint16>(_rom.readUInt16(kInterfaceTemplateOffset) + _state.actionPromptTileAttributes);
	Common::Array<uint8> packedInterfaceCells(
		static_cast<size_t>(kInterfaceTemplateRowCount * kInterfaceTemplateColumnCount *
								 static_cast<int>(sizeof(uint16))),
		0);
	Span<uint8> packedInterfaceCellsSpan = makeSpan(packedInterfaceCells);
	for (int row = 0; row < kInterfaceTemplateRowCount; row++) {
		for (int column = 0; column < kInterfaceTemplateExplicitColumnCount; column++) {
			int sourceOffset =
				kInterfaceTemplateOffset + (row * kInterfaceTemplateColumnCount + column) * static_cast<int>(
																								sizeof(
																									uint16));
			uint16 cell =
				static_cast<uint16>(_rom.readUInt16(sourceOffset) + _state.actionPromptTileAttributes);
			writeUInt16BigEndian(packedInterfaceCellsSpan,
								 (row * kInterfaceTemplateColumnCount + column) * static_cast<int>(sizeof(
																					  uint16)),
								 cell);
		}

		for (int column = kInterfaceTemplateExplicitColumnCount; column < kInterfaceTemplateColumnCount; column++) {
			writeUInt16BigEndian(packedInterfaceCellsSpan,
								 (row * kInterfaceTemplateColumnCount + column) * static_cast<int>(sizeof(
																					  uint16)),
								 repeatedInterfaceCell);
		}
	}

	_scene.loadLayerRows(makeSpan(packedInterfaceCells), TileLayer::Interface, 0,
						 kInterfaceTemplateStartRow, kInterfaceTemplateColumnCount, kInterfaceTemplateRowCount);

	// Ghidra 0x000077AC-0x000077E3: preserve both unconditional child calls and both independent condition
	// terms around the temporary display-bit-four inversion and interface save/clear boundary.
	_actionMenu.composeInventoryGrid();
	_actionMenu.commitActionPromptTiles();
	if ((_state.progressStateBytes[0] & 0x02) != 0 || (_state.interactionFlags & 0x04) != 0) {
		_state.displayFlags ^= 0x10;
		_roomInterfaceTransitions.saveAndClearInteractionTileBlock();
		_state.displayFlags ^= 0x10;
	}

	// Ghidra 0x000077E4-0x00007805: publish the room-name text and resolve the selected layout dimensions.
	_state.roomNameTextOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeRoomNameBaseField)) +
								static_cast<int>(_rom.readUInt32(roomRecordOffset));
	int roomLayoutOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeLayoutBaseField)) +
						   static_cast<int>(_rom.readUInt32(roomRecordOffset + 8));
	_state.roomWidthTiles = _rom.readUInt16(roomLayoutOffset);
	_state.roomHeightTiles = _rom.readUInt16(roomLayoutOffset + static_cast<int>(sizeof(uint16)));

	// Ghidra 0x00007806-0x00007869: decode foreground, background, and collision sections through both
	// relative-prefix jumps, then retain independent working maps and pristine snapshots.
	int roomSectionBaseOffset = roomLayoutOffset + 8;
	int foregroundSectionOffset = roomSectionBaseOffset + static_cast<int>(_rom.readUInt32(roomLayoutOffset + 4));
	Common::Array<uint8> foregroundCells = decompressRingLz(
		_rom, foregroundSectionOffset + static_cast<int>(sizeof(uint32)));
	Common::Array<uint8> backgroundCells =
		decompressRingLz(_rom, roomSectionBaseOffset);
	int collisionSectionOffset = foregroundSectionOffset + static_cast<int>(sizeof(uint32)) +
								 static_cast<int>(_rom.readUInt32(foregroundSectionOffset));
	Common::Array<uint8> collisionCells = decompressRingLz(
		_rom, collisionSectionOffset + static_cast<int>(sizeof(uint32)));
	_state.activeRoomScene = makeUnique<RoomSceneData>(
		makeSpan(backgroundCells), makeSpan(foregroundCells), makeSpan(collisionCells));
	_state.roomCoordinateDataOffset = collisionSectionOffset + static_cast<int>(sizeof(uint32)) +
									  static_cast<int>(_rom.readUInt32(collisionSectionOffset));

	// Ghidra 0x0000786A-0x0000789F: locate the eight initial coordinate pairs and count every wander entry
	// through the first all-one sentinel, retaining count eight when no earlier sentinel occurs.
	_state.roomPositionCoordinateTableOffset =
		_state.roomCoordinateDataOffset +
		static_cast<int16>(_state.roomHeightTiles * static_cast<int>(sizeof(uint16)));
	_state.roomMovementDataOffset = _state.roomPositionCoordinateTableOffset +
									kPositionCoordinateTableByteCount;
	int16 randomMoveCount = 0;
	while (randomMoveCount != 8 &&
		   _rom.readUInt32(
			   _state.roomMovementDataOffset + randomMoveCount * static_cast<int>(sizeof(uint32))) !=
			   0xFFFFFFFFu) {
		randomMoveCount++;
	}

	_state.randomMoveCount = randomMoveCount;

	// Ghidra 0x000078A0-0x000078D9: the interaction-bit-three branch skips every coordinate read and write;
	// otherwise replace only the integer halves of actor slots zero and one, retaining fractional halves.
	if ((_state.interactionFlags & 0x08) == 0) {
		int initialPositionOffset = _state.roomPositionCoordinateTableOffset + _state.initialRoomPositionCoordinateOffset;
		int16 initialX = static_cast<int16>(_rom.readInt16(initialPositionOffset) << 3);
		int16 initialY = static_cast<int16>(
			_rom.readInt16(initialPositionOffset + static_cast<int>(sizeof(uint16))) << 3);
		_state.actorXFixedCoordinates[kShaggyActorSlot] = (_state.actorXFixedCoordinates[kShaggyActorSlot] & 0xFFFF) | (initialX << 16);
		_state.actorXFixedCoordinates[kScoobyActorSlot] = (_state.actorXFixedCoordinates[kScoobyActorSlot] & 0xFFFF) | (initialX << 16);
		_state.actorYFixedCoordinates[kShaggyActorSlot] = (_state.actorYFixedCoordinates[kShaggyActorSlot] & 0xFFFF) | (initialY << 16);
		_state.actorYFixedCoordinates[kScoobyActorSlot] =
			(_state.actorYFixedCoordinates[kScoobyActorSlot] & 0xFFFF) | (static_cast<int16>(initialY - 1) << 16);
	}

	// Ghidra 0x000078DA-0x00007945: preserve logical word shifts, signed lower clamps, signed viewport-bound
	// branches, and 16-bit scaling while selecting the initial camera from actor zero.
	int16 actorX = static_cast<int16>(_state.actorXFixedCoordinates[kShaggyActorSlot] >> 16);
	int16 actorY = static_cast<int16>(_state.actorYFixedCoordinates[kShaggyActorSlot] >> 16);
	int16 cameraTileX = static_cast<int16>((static_cast<uint16>(actorX) >> 3) - 16);
	int16 cameraTileY = static_cast<int16>((static_cast<uint16>(actorY) >> 3) - 10);
	if (cameraTileX < 0) {
		cameraTileX = 0;
	}

	if (cameraTileY < 0) {
		cameraTileY = 0;
	}

	int16 horizontalOverflow =
		static_cast<int16>(cameraTileX + kViewportWidthTiles - _state.roomWidthTiles);
	if (horizontalOverflow > 0) {
		cameraTileX -= horizontalOverflow;
	}

	int16 verticalOverflow =
		static_cast<int16>(cameraTileY + kViewportHeightTiles - _state.roomHeightTiles);
	if (verticalOverflow > 0) {
		cameraTileY -= verticalOverflow;
	}

	_state.cameraX = static_cast<int16>(cameraTileX << 3);
	_state.cameraY = static_cast<int16>(cameraTileY << 3);

	// Ghidra 0x00007946-0x0000794F: draw the initial room viewport, initialize room actors, and return.
	_tileStreamer.drawRoomViewport();
	_actorInitializer.initializeRoomActors();
}
} // namespace Scooby
