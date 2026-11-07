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

#include "room_object_tile_patch_renderer.h"

#include "room_scene_data.h"
#include "scooby/common/contracts.h"

namespace Scooby {

void RoomObjectTilePatchRenderer::applyRoomObjectTilePatch(int16 tilePatchIndex) {
	// Ghidra 0x00007D54-0x00007D5F: preserve the video-mode tail edge as a distinct original boundary.
	if ((_state.videoFlags & kTransferPatchDirectlyMask) != 0) {
		transferRoomObjectTilePatch(tilePatchIndex);
		return;
	}

	// Ghidra 0x00007D60-0x00007D69: the normal path preserves registers, returns for zero, and selects
	// independent positive-apply and negative-restore branches.
	if (tilePatchIndex == 0) {
		return;
	}

	RoomShapePatch patch = readPatchDescriptor(tilePatchIndex);
	SCOOBY_ASSERT(_state.activeRoomScene, "ApplyRoomObjectTilePatch requires an active room scene.");
	RoomSceneData &room = *_state.activeRoomScene;
	Common::Array<uint8> &workingCells =
		patch.layer == TileLayer::Background ? room.backgroundCells : room.foregroundCells;
	int16 roomRowByteStride =
		static_cast<int16>(_state.roomWidthTiles * static_cast<int>(sizeof(uint16)));
	int16 patchByteOffset = static_cast<int16>(patch.y * roomRowByteStride +
											   patch.x * static_cast<int>(sizeof(uint16)));
	int16 rowGapByteCount =
		static_cast<int16>(roomRowByteStride - patch.width * static_cast<int>(sizeof(uint16)));

	if (tilePatchIndex > 0) {
		// Ghidra 0x00007D6A-0x00007DBD / 0x00007DBE-0x00007DDD: resolve the positive descriptor and copy every
		// authored word with the exact nested DBF lifecycles.
		copyAuthoredPatch(patch, workingCells, patchByteOffset, rowGapByteCount);
	} else {
		// Ghidra 0x00007E9A-0x00007EF9: clear the index sign bit, resolve the same descriptor, and select the
		// matching pristine snapshot independently for background and foreground records.
		const Common::Array<uint8> &snapshotCells = patch.layer == TileLayer::Background
														? room.backgroundCellSnapshot
														: room.foregroundCellSnapshot;

		// Ghidra 0x00007EFA-0x00007F1D: restore every word with equal source and destination row strides.
		restorePatch(workingCells, snapshotCells, patchByteOffset, rowGapByteCount, patch.width, patch.height);

		// Ghidra 0x00007F1E-0x00007F21: rejoin the common camera-relative publication path.
	}

	// Ghidra 0x00007DDE-0x00007E13: retain wrapped descriptor bounds, unsigned camera-tile origins, and the
	// working-map row gap used by the publication traversal.
	publishPatchCells(patch, workingCells, patchByteOffset, rowGapByteCount);

	// Ghidra 0x00007E94-0x00007E99: restore preserved registers and return after either the zero or completed
	// common path.
}

void RoomObjectTilePatchRenderer::copyAuthoredPatch(const RoomShapePatch &patch,
													Common::Array<uint8> &workingCells,
													int destinationByteOffset, int16 rowGapByteCount) {
	int sourceOffset = patch.recordOffset + kDescriptorTileCellsField;
	int16 remainingRows = static_cast<int16>(patch.height - 1);
	do {
		int16 remainingColumns = static_cast<int16>(patch.width - 1);
		do {
			uint16 cellValue = _rom.readUInt16(sourceOffset);
			workingCells[static_cast<size_t>(destinationByteOffset)] =
				static_cast<uint8>(cellValue >> 8);
			workingCells[static_cast<size_t>(destinationByteOffset + 1)] = static_cast<uint8>(
				cellValue);
			sourceOffset += static_cast<int>(sizeof(uint16));
			destinationByteOffset += static_cast<int>(sizeof(uint16));
			remainingColumns--;
		} while (remainingColumns != -1);

		destinationByteOffset += rowGapByteCount;
		remainingRows--;
	} while (remainingRows != -1);
}

void RoomObjectTilePatchRenderer::restorePatch(Common::Array<uint8> &workingCells,
											   const Common::Array<uint8> &snapshotCells,
											   int patchByteOffset, int16 rowGapByteCount,
											   int16 width, int16 height) {
	int sourceByteOffset = patchByteOffset;
	int destinationByteOffset = patchByteOffset;
	int16 remainingRows = static_cast<int16>(height - 1);
	do {
		int16 remainingColumns = static_cast<int16>(width - 1);
		do {
			workingCells[static_cast<size_t>(destinationByteOffset)] =
				snapshotCells[static_cast<size_t>(sourceByteOffset)];
			workingCells[static_cast<size_t>(destinationByteOffset + 1)] =
				snapshotCells[static_cast<size_t>(sourceByteOffset + 1)];
			sourceByteOffset += static_cast<int>(sizeof(uint16));
			destinationByteOffset += static_cast<int>(sizeof(uint16));
			remainingColumns--;
		} while (remainingColumns != -1);

		sourceByteOffset += rowGapByteCount;
		destinationByteOffset += rowGapByteCount;
		remainingRows--;
	} while (remainingRows != -1);
}

void RoomObjectTilePatchRenderer::publishPatchCells(const RoomShapePatch &patch,
													const Common::Array<uint8> &workingCells,
													int sourceByteOffset, int16 rowGapByteCount) {
	uint16 cameraTileX = static_cast<uint16>(_state.cameraX) >> 3;
	uint16 cameraTileY = static_cast<uint16>(_state.cameraY) >> 3;
	int16 startColumn = static_cast<int16>(patch.x - cameraTileX);
	int16 endColumn = static_cast<int16>(patch.x + patch.width - cameraTileX);
	int16 currentRow = static_cast<int16>(patch.y - cameraTileY);
	int16 endRow = static_cast<int16>(patch.y + patch.height - cameraTileY);

	do {
		int16 currentColumn = startColumn;
		do {
			Span<const uint8> packedCell = makeSpan(workingCells).slice(static_cast<size_t>(sourceByteOffset), sizeof(uint16));

			// Ghidra 0x00007E14-0x00007E7D: read every affected working-map cell and retain all four
			// independent signed clipping branches before replacing the single-word VDP write.
			if (currentColumn >= kExpandedViewportMinimumCoordinate) {
				if (currentRow >= kExpandedViewportMinimumCoordinate) {
					if (currentColumn < kExpandedViewportMaximumColumn) {
						if (currentRow < kExpandedViewportMaximumRow) {
							int destinationColumn =
								static_cast<uint16>(cameraTileX + currentColumn) & kLayerColumnMask;
							int destinationRow = (static_cast<uint16>(cameraTileY + currentRow) +
												  kLayerRowBias) &
												 kLayerRowMask;
							_scene.loadLayerRows(packedCell, patch.layer, destinationColumn, destinationRow, 1,
												 1);
						}
					}
				}
			}

			sourceByteOffset += static_cast<int>(sizeof(uint16));
			currentColumn++;
		} while (currentColumn != endColumn);

		// Ghidra 0x00007E7E-0x00007E93: advance through the complete width, apply the source row gap, reset
		// the horizontal coordinate, and retain the wrapped exclusive vertical bound.
		sourceByteOffset += rowGapByteCount;
		currentRow++;
	} while (currentRow != endRow);
}

RoomObjectTilePatchRenderer::RoomShapePatch RoomObjectTilePatchRenderer::readPatchDescriptor(
	int16 tilePatchIndex) const {
	uint16 descriptorIndex = static_cast<uint16>(tilePatchIndex);
	if (tilePatchIndex < 0) {
		descriptorIndex &= kRestorePatchIndexMask;
	}

	int episodeDescriptorOffset = _state.activeEpisodeDescriptorOffset;
	int shapeDataOffset = static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeDataField));
	int shapeOffsetTableOffset =
		static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeRoomShapeOffsetTableField));
	int16 tableByteOffset =
		static_cast<int16>((descriptorIndex - 1) * static_cast<int>(sizeof(uint32)));
	int recordOffset = shapeDataOffset + static_cast<int>(_rom.readUInt32(
											 shapeOffsetTableOffset + tableByteOffset));
	return RoomShapePatch(recordOffset, _rom.readInt16(recordOffset), _rom.readInt16(recordOffset + 2),
						  _rom.readInt16(recordOffset + 4), _rom.readInt16(recordOffset + 6),
						  _rom.readInt16(recordOffset + kDescriptorPlaneField) == 0
							  ? TileLayer::Background
							  : TileLayer::Foreground);
}

void RoomObjectTilePatchRenderer::transferRoomObjectTilePatch(int16 tilePatchIndex) {
	// Ghidra 0x00007C82-0x00007C8B: preserve registers and retain the independent zero, positive, and
	// negative descriptor branches.
	if (tilePatchIndex == 0) {
		// Zero rejoins the shared positive-path epilogue documented below without publishing cells.
		return;
	}

	RoomShapePatch patch = readPatchDescriptor(tilePatchIndex);
	uint16 destinationRowByteOffset = static_cast<uint16>(
		kLayerRowBias * (kLayerColumnMask + 1) * static_cast<int>(sizeof(uint16)) +
		patch.y * (kLayerColumnMask + 1) * static_cast<int>(sizeof(uint16)) +
		patch.x * static_cast<int>(sizeof(uint16)));
	uint16 rowByteCount =
		static_cast<uint16>(static_cast<uint16>(patch.width) * sizeof(uint16));
	int16 remainingRows = static_cast<int16>(patch.height - 1);

	if (tilePatchIndex > 0) {
		// Ghidra 0x00007C8C-0x00007CBF: resolve the positive record and its selected name-table address.
		int sourceByteOffset = patch.recordOffset + kDescriptorTileCellsField;
		do {
			// Ghidra 0x00007CC0-0x00007CD9: publish one unsigned-width authored row, advance both cursors,
			// and preserve the exact DBF height lifecycle.
			publishDirectPatchRow(_rom.readBytes(sourceByteOffset, rowByteCount), patch.layer,
								  destinationRowByteOffset, static_cast<uint16>(patch.width));
			sourceByteOffset += rowByteCount;
			destinationRowByteOffset += (kLayerColumnMask + 1) * static_cast<int>(sizeof(uint16));
			remainingRows--;
		} while (remainingRows != -1);

		// Ghidra 0x00007CDA-0x00007CDF: restore preserved registers after the completed positive transfer.
		return;
	}

	// Ghidra 0x00007CE0-0x00007D27: clear the descriptor sign bit, resolve the same record and plane, and
	// derive the pristine snapshot source from x plus y times the fixed 32-cell row stride.
	SCOOBY_ASSERT(_state.activeRoomScene, "TransferRoomObjectTilePatch requires an active room scene.");
	const RoomSceneData &room = *_state.activeRoomScene;
	const Common::Array<uint8> &snapshotCells =
		patch.layer == TileLayer::Background
			? room.backgroundCellSnapshot
			: room.foregroundCellSnapshot;
	int16 snapshotByteOffset = static_cast<int16>(
		patch.y * kDirectTransferSnapshotColumnCount * static_cast<int>(sizeof(uint16)) +
		patch.x * static_cast<int>(sizeof(uint16)));
	do {
		// Ghidra 0x00007D28-0x00007D4D: publish one unsigned-width pristine row, retain the helper's source
		// advancement plus explicit row gap as one 32-cell stride, and preserve the exact DBF lifecycle.
		publishDirectPatchRow(
			makeSpan(snapshotCells).slice(static_cast<size_t>(snapshotByteOffset), rowByteCount),
			patch.layer, destinationRowByteOffset, static_cast<uint16>(patch.width));
		snapshotByteOffset += kDirectTransferSnapshotColumnCount * static_cast<int>(sizeof(uint16));
		destinationRowByteOffset += (kLayerColumnMask + 1) * static_cast<int>(sizeof(uint16));
		remainingRows--;
	} while (remainingRows != -1);

	// Ghidra 0x00007D4E-0x00007D53: restore preserved registers after the completed negative transfer.
}

void RoomObjectTilePatchRenderer::publishDirectPatchRow(Span<const uint8> packedCells,
														TileLayer layer,
														uint16 destinationRowByteOffset,
														uint16 columnCount) {
	for (int column = 0; column < columnCount; ++column) {
		int destinationCellIndex =
			(destinationRowByteOffset / static_cast<int>(sizeof(uint16)) + column) & kLayerCellMask;
		_scene.loadLayerRows(
			packedCells.slice(static_cast<size_t>(column * static_cast<int>(sizeof(uint16))),
							  sizeof(uint16)),
			layer, destinationCellIndex & kLayerColumnMask, destinationCellIndex / (kLayerColumnMask + 1), 1,
			1);
	}
}
} // namespace Scooby
