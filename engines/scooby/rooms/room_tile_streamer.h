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

#ifndef SCOOBY_ROOM_TILE_STREAMER_H
#define SCOOBY_ROOM_TILE_STREAMER_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns camera-relative room-plane scrolling, publication, and the four bordering tile streams.

namespace Scooby {

class RoomTileStreamer {
public:
	// Binds active decoded room cells to their logical plane destination and camera state.
	// rom: verified cartridge containing the authored room-scroll phase tables.
	// scene: logical background and foreground planes replacing the two hardware name tables.
	// state: active room dimensions, decoded cells, camera origin, and streaming flags.
	RoomTileStreamer(const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state)
		: _rom(rom), _scene(scene), _state(state) {
	}

	// Publishes the camera-selected room viewport and refreshes its four bordering edges.
	//
	// Ghidra: drawRoomViewport (0x00007950). The decoded maps remain width-strided while each plane
	// destination retains the original 64-column, 32-row name-table ring. Plane B receives the background
	// workspace and plane A receives the foreground workspace before all edge streamers run.
	void drawRoomViewport();

	// Advances room scrolling, streams every requested edge, and publishes all plane offsets.
	//
	// Ghidra: updateScrollingAndStreamTiles (0x0000FA78). The 23-word authored phase sequence
	// g_awRoomScrollPhaseOffsets at 0x0003244E-0x0003247B supplies 22 adjacent foreground and background
	// offset pairs. Video bit one bypasses all derivation and edge consumption while retaining final
	// publication of the four existing scroll words.
	void updateScrollingAndStreamTiles();

private:
	static const uint8 kBottomEdgeMask = 0x08;
	static const uint8 kCameraPanActiveMask = 0x20;
	static const int kEdgeColumnHeightTiles = 21;
	static const int kEdgeRowWidthTiles = 34;
	static const uint8 kForegroundOverrideMask = 0x40;
	static const uint8 kLeftEdgeMask = 0x02;
	static const int kNameTableColumnCount = 64;
	static const int kNameTableRowCount = 32;
	static const uint8 kRightEdgeMask = 0x01;
	static const uint8 kRoomScrollAnimationMask = 0x80;
	static const int kRoomScrollPhaseDelayCount = 4;
	static const int kRoomScrollPhaseDelaysOffset = 0x3247C;
	static const int kRoomScrollPhaseOffsetsOffset = 0x3244E;
	static const int16 kRoomScrollPhaseTerminalOffset = 0x2C;
	static const uint8 kScrollDerivationBypassMask = 0x02;
	static const uint8 kTopEdgeMask = 0x04;
	static const int kViewportHeightTiles = 19;
	static const int kViewportWidthTiles = 32;

	void streamRoomRightEdgeColumns();
	void streamRoomLeftEdgeColumns();
	void streamRoomTopEdgeRows();
	void streamRoomBottomEdgeRows();

	void publishViewport(Span<const uint8> packedCells, TileLayer layer,
						 int sourceByteOffset, int sourceRowByteStride, int destinationColumn,
						 int destinationStartRow);
	void publishColumn(Span<const uint8> packedCells, TileLayer layer,
					   int sourceByteOffset, int sourceRowByteStride, int destinationColumn,
					   int destinationStartRow, int rowCount);
	void publishHorizontalEdgeRow(uint16 sourceX, int sourceY, int destinationRow);
	void publishMapCell(Span<const uint8> packedCells, TileLayer layer,
						int sourceByteOffset, int destinationColumn, int destinationRow);
	void publishRow(Span<const uint8> packedCells, TileLayer layer,
					int sourceByteOffset,
					int destinationColumn, int destinationRow, int columnCount);
	void publishVerticalEdgeColumn(uint16 sourceX, int cameraTileY);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_TILE_STREAMER_H
