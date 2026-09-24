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

#include "room_tile_streamer.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "room_scene_data.h"
#include "scooby/common/contracts.h"

namespace Scooby {
namespace {
const Common::Array<uint8> kEmptyPackedCell = {0, 0};
} // namespace

void RoomTileStreamer::drawRoomViewport() {
	// Ghidra 0x00007950-0x000079A1: derive the width-strided source and camera-relative ring destination.
	SDM_ASSERT(_state.ActiveRoomScene, "DrawRoomViewport requires an active room scene.");
	const RoomSceneData &room = *_state.ActiveRoomScene;
	uint16 cameraTileX = static_cast<uint16>(_state.CameraX) >> 3;
	uint16 cameraTileY = static_cast<uint16>(_state.CameraY) >> 3;
	uint16 sourceByteOffset = static_cast<uint16>(
		(cameraTileY * _state.RoomWidthTiles + cameraTileX) * static_cast<int>(sizeof(uint16)));
	uint16 sourceRowByteStride =
		static_cast<uint16>(_state.RoomWidthTiles * static_cast<int>(sizeof(uint16)));
	int destinationColumn = cameraTileX & (kNameTableColumnCount - 1);
	int destinationStartRow = (cameraTileY + 2) & (kNameTableRowCount - 1);

	// Ghidra 0x000079A2-0x000079D9: publish all 19 rows and 32 columns from the background workspace.
	publishViewport(MakeSpan(room.BackgroundCells), TileLayer::Background, sourceByteOffset,
					sourceRowByteStride, destinationColumn, destinationStartRow);

	// Ghidra 0x000079DA-0x00007A0F: repeat the same source and destination traversal for the foreground.
	publishViewport(MakeSpan(room.ForegroundCells), TileLayer::Foreground, sourceByteOffset,
					sourceRowByteStride, destinationColumn, destinationStartRow);

	// Ghidra 0x00007A10-0x00007A29: refresh every bordering edge in the original unconditional order.
	streamRoomRightEdgeColumns();
	streamRoomLeftEdgeColumns();
	streamRoomTopEdgeRows();
	streamRoomBottomEdgeRows();
}

void RoomTileStreamer::updateScrollingAndStreamTiles() {
	// Ghidra 0x0000FA78-0x0000FA83: video bit one bypasses every update through edge-mask clearing but still
	// reaches both final logical scroll publications.
	if ((_state.VideoFlags & kScrollDerivationBypassMask) == 0) {
		// Ghidra 0x0000FA84-0x0000FAE3: retain both inactive-pan and active-pan paths, word wrapping, crossing
		// detection, signed direction selection, and equality-only completion.
		if ((_state.TransitionFlags & kCameraPanActiveMask) != 0) {
			_state.CameraX += _state.CameraPanStep;
			int cameraRemainder = _state.CameraX & 0x07;
			int crossingRemainder = static_cast<int16>(8 + _state.CameraPanStep) & 0x07;
			if (cameraRemainder == crossingRemainder) {
				if (_state.CameraPanStep < 0) {
					_state.TileStreamingEdgeFlags |= kLeftEdgeMask;
				} else {
					_state.TileStreamingEdgeFlags |= kRightEdgeMask;
				}
			}

			if (_state.CameraX == _state.CameraPanTargetX) {
				_state.TransitionFlags &= static_cast<uint8>(~kCameraPanActiveMask);
			}
		}

		// Ghidra 0x0000FAE4-0x0000FB17: derive the wrapped background X displacement, then retain both
		// normal-copy and actor-zero-relative foreground branches selected by interaction bit six.
		_state.BackgroundHorizontalScroll = static_cast<int16>(-(_state.CameraX & 0x03FF));
		_state.ForegroundHorizontalScroll = _state.BackgroundHorizontalScroll;
		if ((_state.InteractionFlags & kForegroundOverrideMask) != 0) {
			int16 actorX = static_cast<int16>(_state.ActorXFixedCoordinates[0] >> 16);
			_state.ForegroundHorizontalScroll = static_cast<int16>(actorX - _state.CameraX - 0x2C);
		}

		// Ghidra 0x0000FB18-0x0000FB49: derive the wrapped background Y displacement, then retain both
		// normal-copy and negated actor-zero-relative foreground branches.
		_state.BackgroundVerticalScroll = static_cast<int16>(_state.CameraY & 0x00FF);
		_state.ForegroundVerticalScroll = _state.BackgroundVerticalScroll;
		if ((_state.InteractionFlags & kForegroundOverrideMask) != 0) {
			int16 actorY = static_cast<int16>(_state.ActorYFixedCoordinates[0] >> 16);
			_state.ForegroundVerticalScroll = static_cast<int16>(-(actorY - _state.CameraY - 0x30));
		}

		// Ghidra 0x0000FB4A-0x0000FBCB: retain the disabled branch, signed delay branch, four-entry high-byte
		// cursor cycle, all 22 adjacent phase pairs, and equality-only terminal offset.
		if ((_state.RoomBehaviorFlags & kRoomScrollAnimationMask) != 0) {
			bool applyPhaseOffsets = true;
			if (_state.ScrollAnimationPhaseOffset < 0) {
				_state.ScrollAnimationDelay--;
				if (_state.ScrollAnimationDelay >= 0) {
					applyPhaseOffsets = false;
				} else {
					int8 delayIndex = static_cast<int8>(
						static_cast<uint16>(_state.ScrollAnimationDelayCursor) >> 8);
					_state.ScrollAnimationDelay =
						static_cast<int8>(_rom.readByte(kRoomScrollPhaseDelaysOffset + delayIndex));
					int16 nextDelayIndex = static_cast<int16>(delayIndex + 1);
					if (nextDelayIndex == kRoomScrollPhaseDelayCount) {
						nextDelayIndex = 0;
					}

					_state.ScrollAnimationDelayCursor = static_cast<int16>(
						(static_cast<uint16>(_state.ScrollAnimationDelayCursor) & 0x00FF) |
						((static_cast<uint16>(nextDelayIndex) & 0x00FF) << 8));
					_state.ScrollAnimationPhaseOffset = 0;
				}
			}

			if (applyPhaseOffsets) {
				int16 phaseOffset = _state.ScrollAnimationPhaseOffset;
				_state.ScrollAnimationPhaseOffset =
					static_cast<int16>(phaseOffset + static_cast<int>(sizeof(uint16)));
				if (_state.ScrollAnimationPhaseOffset == kRoomScrollPhaseTerminalOffset) {
					_state.ScrollAnimationPhaseOffset = -1;
				}

				_state.ForegroundVerticalScroll += _rom.readInt16(kRoomScrollPhaseOffsetsOffset + phaseOffset);
				_state.BackgroundVerticalScroll += _rom.readInt16(kRoomScrollPhaseOffsetsOffset + phaseOffset + static_cast<int>(sizeof(uint16)));
			}
		}

		// Ghidra 0x0000FBCC-0x0000FC09: test every edge independently in right, left, top, bottom order, then
		// clear the complete byte including unconsumed high bits.
		if ((_state.TileStreamingEdgeFlags & kRightEdgeMask) != 0) {
			streamRoomRightEdgeColumns();
		}

		if ((_state.TileStreamingEdgeFlags & kLeftEdgeMask) != 0) {
			streamRoomLeftEdgeColumns();
		}

		if ((_state.TileStreamingEdgeFlags & kTopEdgeMask) != 0) {
			streamRoomTopEdgeRows();
		}

		if ((_state.TileStreamingEdgeFlags & kBottomEdgeMask) != 0) {
			streamRoomBottomEdgeRows();
		}

		_state.TileStreamingEdgeFlags = 0;
	}

	// Ghidra 0x0000FC0A-0x0000FC23: replace both WriteSingleVramWord horizontal-scroll transfers with logical
	// foreground/background publication in the same order.
	_scene.setHorizontalOffsets(_state.ForegroundHorizontalScroll, _state.BackgroundHorizontalScroll);

	// Ghidra 0x0000FC24-0x0000FC39: replace the paired VSRAM longword write with logical vertical-scroll
	// publication and return without retaining VDP command state.
	_scene.setVerticalOffsets(_state.ForegroundVerticalScroll, _state.BackgroundVerticalScroll);
}

void RoomTileStreamer::streamRoomRightEdgeColumns() {
	// Ghidra 0x0000FC3A-0x0000FC51: equality with width minus 32 alone skips the complete right edge.
	uint16 cameraTileX = static_cast<uint16>(_state.CameraX) >> 3;
	uint16 rightmostCameraTileX = static_cast<uint16>(_state.RoomWidthTiles -
													  kViewportWidthTiles);
	if (cameraTileX != rightmostCameraTileX) {
		// Ghidra 0x0000FC52-0x0000FCB1: select column X+32 from row Y-1 and derive its first segment.
		uint16 cameraTileY = static_cast<uint16>(_state.CameraY) >> 3;
		uint16 sourceX = static_cast<uint16>(cameraTileX + kViewportWidthTiles);

		// Ghidra 0x0000FCB2-0x0000FCFB / 0x0000FCFC-0x0000FD05 / 0x0000FD06-0x0000FD53: publish the first
		// segment and any wrapped remainder through the shared column publisher.
		publishVerticalEdgeColumn(sourceX, cameraTileY);
	}

	// Ghidra 0x0000FD54-0x0000FD5D: restore two-byte VDP auto-increment and return. The logical scene has no
	// persistent transfer-increment state to restore.
}

void RoomTileStreamer::streamRoomLeftEdgeColumns() {
	// Ghidra 0x0000FD5E-0x0000FD69: column zero alone skips the complete left edge.
	uint16 cameraTileX = static_cast<uint16>(_state.CameraX) >> 3;
	if (cameraTileX != 0) {
		// Ghidra 0x0000FD6A-0x0000FDC7: select column X-1 from row Y-1 and derive its first segment.
		uint16 sourceX = static_cast<uint16>(cameraTileX - 1);
		uint16 cameraTileY = static_cast<uint16>(_state.CameraY) >> 3;

		// Ghidra 0x0000FDC8-0x0000FE11 / 0x0000FE12-0x0000FE1B / 0x0000FE1C-0x0000FE69: publish the first
		// segment and any wrapped remainder through the shared column publisher.
		publishVerticalEdgeColumn(sourceX, cameraTileY);
	}

	// Ghidra 0x0000FE6A-0x0000FE73: restore two-byte VDP auto-increment and return. The logical scene has no
	// persistent transfer-increment state to restore.
}

void RoomTileStreamer::streamRoomTopEdgeRows() {
	// Ghidra 0x0000FE74-0x0000FE7F: row zero alone skips the complete top edge.
	uint16 cameraTileY = static_cast<uint16>(_state.CameraY) >> 3;
	if (cameraTileY != 0) {
		// Ghidra 0x0000FE80-0x0000FEDF: select row Y-1 from column X-1 and derive its first segment.
		uint16 cameraTileX = static_cast<uint16>(_state.CameraX) >> 3;
		uint16 sourceX = static_cast<uint16>(cameraTileX - 1);
		uint16 sourceY = static_cast<uint16>(cameraTileY - 1);
		int destinationRow = (cameraTileY + 1) & (kNameTableRowCount - 1);

		// Ghidra 0x0000FEE0-0x0000FF17 / 0x0000FF18-0x0000FF21 / 0x0000FF22-0x0000FF65: publish the first
		// segment and any wrapped remainder through the shared row publisher.
		publishHorizontalEdgeRow(sourceX, sourceY, destinationRow);
	}

	// Ghidra 0x0000FF66-0x0000FF67: return with no persistent hardware-transfer state.
}

void RoomTileStreamer::streamRoomBottomEdgeRows() {
	// Ghidra 0x0000FF68-0x0000FF7F: equality with height minus 19 alone skips the complete bottom edge.
	uint16 cameraTileY = static_cast<uint16>(_state.CameraY) >> 3;
	uint16 bottommostCameraTileY = static_cast<uint16>(_state.RoomHeightTiles -
													   kViewportHeightTiles);
	if (cameraTileY != bottommostCameraTileY) {
		// Ghidra 0x0000FF80-0x0000FFE1: select row Y+19 from column X-1 and derive its first segment.
		uint16 sourceY = static_cast<uint16>(cameraTileY + kViewportHeightTiles);
		uint16 cameraTileX = static_cast<uint16>(_state.CameraX) >> 3;
		uint16 sourceX = static_cast<uint16>(cameraTileX - 1);
		int destinationRow = (sourceY + 2) & (kNameTableRowCount - 1);

		// Ghidra 0x0000FFE2-0x00010019 / 0x0001001A-0x00010023 / 0x00010024-0x00010067: publish the first
		// segment and any wrapped remainder through the shared row publisher.
		publishHorizontalEdgeRow(sourceX, sourceY, destinationRow);
	}

	// Ghidra 0x00010068-0x00010069: return with no persistent hardware-transfer state.
}

void RoomTileStreamer::publishViewport(Span<const uint8> packedCells, TileLayer layer,
									   int sourceByteOffset, int sourceRowByteStride, int destinationColumn,
									   int destinationStartRow) {
	for (int row = 0; row < kViewportHeightTiles; ++row) {
		_scene.loadLayerRows(
			packedCells.slice(static_cast<std::size_t>(sourceByteOffset + row * sourceRowByteStride),
							  static_cast<std::size_t>(kViewportWidthTiles *
													   static_cast<int>(sizeof(uint16)))),
			layer, destinationColumn, (destinationStartRow + row) & (kNameTableRowCount - 1),
			kViewportWidthTiles,
			1);
	}
}

void RoomTileStreamer::publishColumn(Span<const uint8> packedCells, TileLayer layer,
									 int sourceByteOffset, int sourceRowByteStride, int destinationColumn,
									 int destinationStartRow, int rowCount) {
	for (int row = 0; row < rowCount; ++row) {
		publishMapCell(packedCells, layer, sourceByteOffset + row * sourceRowByteStride, destinationColumn,
					   destinationStartRow + row);
	}
}

void RoomTileStreamer::publishHorizontalEdgeRow(uint16 sourceX, int sourceY, int destinationRow) {
	uint16 sourceRowByteStride =
		static_cast<uint16>(_state.RoomWidthTiles * static_cast<int>(sizeof(uint16)));
	int16 sourceByteOffset = static_cast<int16>(sourceY * sourceRowByteStride +
												sourceX * static_cast<int>(sizeof(uint16)));
	int destinationColumn = sourceX & (kNameTableColumnCount - 1);
	int firstSegmentColumnCount = kNameTableColumnCount - destinationColumn;
	if (firstSegmentColumnCount > kEdgeRowWidthTiles) {
		firstSegmentColumnCount = kEdgeRowWidthTiles;
	}

	SDM_ASSERT(_state.ActiveRoomScene, "PublishHorizontalEdgeRow requires an active room scene.");
	const RoomSceneData &room = *_state.ActiveRoomScene;
	publishRow(MakeSpan(room.BackgroundCells), TileLayer::Background, sourceByteOffset,
			   destinationColumn, destinationRow, firstSegmentColumnCount);
	if ((_state.InteractionFlags & kForegroundOverrideMask) == 0) {
		publishRow(MakeSpan(room.ForegroundCells), TileLayer::Foreground, sourceByteOffset,
				   destinationColumn, destinationRow, firstSegmentColumnCount);
	}

	int16 remainingBalance = static_cast<int16>(firstSegmentColumnCount - kEdgeRowWidthTiles);
	if (remainingBalance >= 0) {
		return;
	}

	int secondSegmentColumnCount = -remainingBalance;
	int secondSourceByteOffset =
		sourceByteOffset + firstSegmentColumnCount * static_cast<int>(sizeof(uint16));
	publishRow(MakeSpan(room.BackgroundCells), TileLayer::Background, secondSourceByteOffset,
			   0,
			   destinationRow, secondSegmentColumnCount);
	if ((_state.InteractionFlags & kForegroundOverrideMask) == 0) {
		publishRow(MakeSpan(room.ForegroundCells), TileLayer::Foreground,
				   secondSourceByteOffset,
				   0, destinationRow, secondSegmentColumnCount);
	}
}

void RoomTileStreamer::publishMapCell(Span<const uint8> packedCells, TileLayer layer,
									  int sourceByteOffset, int destinationColumn, int destinationRow) {
	uint32 boundary = static_cast<uint32>(static_cast<int>(packedCells.size()) -
										  static_cast<int>(sizeof(uint16)));
	Span<const uint8> sourceCell =
		static_cast<uint32>(sourceByteOffset) <= boundary
			? packedCells.slice(static_cast<std::size_t>(sourceByteOffset), sizeof(uint16))
			: MakeSpan(kEmptyPackedCell);
	_scene.loadLayerRows(sourceCell, layer, destinationColumn, destinationRow, 1, 1);
}

void RoomTileStreamer::publishRow(Span<const uint8> packedCells, TileLayer layer,
								  int sourceByteOffset, int destinationColumn, int destinationRow,
								  int columnCount) {
	for (int column = 0; column < columnCount; ++column) {
		publishMapCell(packedCells, layer, sourceByteOffset + column * static_cast<int>(sizeof(uint16)),
					   destinationColumn + column, destinationRow);
	}
}

void RoomTileStreamer::publishVerticalEdgeColumn(uint16 sourceX, int cameraTileY) {
	uint16 sourceRowByteStride =
		static_cast<uint16>(_state.RoomWidthTiles * static_cast<int>(sizeof(uint16)));
	int16 sourceByteOffset =
		static_cast<int16>(sourceX * static_cast<int>(sizeof(uint16)) +
						   cameraTileY * sourceRowByteStride - sourceRowByteStride);
	int destinationColumn = sourceX & (kNameTableColumnCount - 1);
	int destinationStartRow = (cameraTileY + 1) & (kNameTableRowCount - 1);
	int firstSegmentRowCount = kNameTableRowCount - destinationStartRow;
	if (firstSegmentRowCount > kEdgeColumnHeightTiles) {
		firstSegmentRowCount = kEdgeColumnHeightTiles;
	}

	SDM_ASSERT(_state.ActiveRoomScene, "PublishVerticalEdgeColumn requires an active room scene.");
	const RoomSceneData &room = *_state.ActiveRoomScene;
	publishColumn(MakeSpan(room.BackgroundCells), TileLayer::Background, sourceByteOffset,
				  sourceRowByteStride, destinationColumn, destinationStartRow, firstSegmentRowCount);
	if ((_state.InteractionFlags & kForegroundOverrideMask) == 0) {
		publishColumn(MakeSpan(room.ForegroundCells), TileLayer::Foreground, sourceByteOffset,
					  sourceRowByteStride, destinationColumn, destinationStartRow, firstSegmentRowCount);
	}

	int16 remainingBalance = static_cast<int16>(firstSegmentRowCount - kEdgeColumnHeightTiles);
	if (remainingBalance >= 0) {
		return;
	}

	int secondSegmentRowCount = -remainingBalance;
	int secondSourceByteOffset = sourceByteOffset + firstSegmentRowCount * sourceRowByteStride;
	publishColumn(MakeSpan(room.BackgroundCells), TileLayer::Background,
				  secondSourceByteOffset,
				  sourceRowByteStride, destinationColumn, 0, secondSegmentRowCount);
	if ((_state.InteractionFlags & kForegroundOverrideMask) == 0) {
		publishColumn(MakeSpan(room.ForegroundCells), TileLayer::Foreground,
					  secondSourceByteOffset, sourceRowByteStride, destinationColumn, 0, secondSegmentRowCount);
	}
}
} // namespace Scooby