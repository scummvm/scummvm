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

#ifndef SCOOBY_ROOM_OBJECT_TILE_PATCH_RENDERER_H
#define SCOOBY_ROOM_OBJECT_TILE_PATCH_RENDERER_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns room-object tile-patch mutation and logical scene publication.

namespace Scooby {

class RoomObjectTilePatchRenderer {
public:
	// Binds tile-patch decoding and publication to the active room's logical resources.
	// rom: verified cartridge containing episode shape-offset tables and variable records.
	// scene: logical ring-map destination replacing direct name-table writes.
	// state: active episode, room map, camera, and video state.
	RoomObjectTilePatchRenderer(const ScoobyDooRom &rom, TileScene &scene,
								RuntimeState &state)
		: _rom(rom), _scene(scene), _state(state) {
	}

	// Mutates one room map patch or restores it, then republishes its visible cells.
	// tilePatchIndex: original D0 descriptor index after its caller clears bit 14.
	//
	// Ghidra: applyRoomObjectTilePatch (0x00007D54). Episode-zero shape records occupy
	// 0x0013E9B6-0x0014199B with 211 offsets at 0x00145EB8-0x00146203; episode-one records occupy
	// 0x001A8934-0x001AF33B with 214 offsets at 0x001B3508-0x001B385F. Positive indices copy authored cells
	// into a working map, while negative indices restore from its pristine snapshot. Logical ring-map writes
	// replace the clipped single-word VDP publications.
	void applyRoomObjectTilePatch(int16 tilePatchIndex);

private:
	static const int kDescriptorPlaneField = 0x0C;
	static const int kDescriptorTileCellsField = 0x0E;
	static const int kEpisodeRoomShapeDataField = 0x1C;
	static const int kEpisodeRoomShapeOffsetTableField = 0x24;
	static const int kExpandedViewportMaximumColumn = 33;
	static const int kExpandedViewportMaximumRow = 21;
	static const int kExpandedViewportMinimumCoordinate = -1;
	static const int kDirectTransferSnapshotColumnCount = 32;
	static const int kLayerColumnMask = 0x3F;
	static const int kLayerCellMask = 0x7FF;
	static const int kLayerRowBias = 2;
	static const int kLayerRowMask = 0x1F;
	static const uint16 kRestorePatchIndexMask = 0x7FFF;
	static const uint8 kTransferPatchDirectlyMask = 0x01;

	// Preserves the recovered fixed-record patch geometry shared by both original entry points.
	struct RoomShapePatch {
		int RecordOffset;
		int16 X;
		int16 Y;
		int16 Width;
		int16 Height;
		TileLayer Layer;

		RoomShapePatch(int recordOffset, int16 x, int16 y, int16 width,
					   int16 height,
					   TileLayer layer)
			: RecordOffset(recordOffset), X(x), Y(y), Width(width), Height(height), Layer(layer) {
		}
	};

	void copyAuthoredPatch(const RoomShapePatch &patch, Common::Array<uint8> &workingCells,
						   int destinationByteOffset, int16 rowGapByteCount);
	void restorePatch(Common::Array<uint8> &workingCells, const Common::Array<uint8> &snapshotCells,
					  int patchByteOffset, int16 rowGapByteCount, int16 width,
					  int16 height);
	void publishPatchCells(const RoomShapePatch &patch, const Common::Array<uint8> &workingCells,
						   int sourceByteOffset, int16 rowGapByteCount);
	RoomShapePatch readPatchDescriptor(int16 tilePatchIndex) const;

	// Publishes one room-object tile patch directly without changing its working room map.
	// tilePatchIndex: original D0 descriptor index selected by the tail edge at 0x00007D5C.
	//
	// Ghidra: transferRoomObjectTilePatch (0x00007C82). Positive indices stream contiguous authored cells
	// from the selected episode shape record. Negative indices stream pristine room cells with the original
	// fixed 32-cell source stride. Logical ring-map publication replaces DmaVramWordsWithSourceBankSplit; its
	// unsigned row extent, source advancement, destination order, 64-cell name-table stride, and two-row table
	// bias remain intact.
	void transferRoomObjectTilePatch(int16 tilePatchIndex);

	void publishDirectPatchRow(Span<const uint8> packedCells, TileLayer layer,
							   uint16 destinationRowByteOffset, uint16 columnCount);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_OBJECT_TILE_PATCH_RENDERER_H
