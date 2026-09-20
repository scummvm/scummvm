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

#include "room_script_foreground_override_executor.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_layer.h"

namespace Scooby {
namespace {
uint16 ReadUInt16BigEndian(const Common::Array<uint8> &bytes, int offset) {
	return static_cast<uint16>((bytes[static_cast<std::size_t>(offset)] << 8) |
							   bytes[static_cast<std::size_t>(offset + 1)]);
}

void WriteUInt16BigEndian(Common::Array<uint8> &bytes, int offset, uint16 value) {
	bytes[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 8);
	bytes[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value);
}
} // namespace

void RoomScriptForegroundOverrideExecutor::executeRoomScriptAction07() {
	// Ghidra 0x000044BA-0x000044C1: preserve the full-byte busy branch. Original interrupts keep publishing
	// actor tiles during the spin, so each continuation services one callback-aware clocked frame.
	while (_state.ActorTileUploadPendingFlags != 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x000044C2-0x000044C9: select actor-zero-relative foreground scrolling and suppress all normal
	// foreground edge-stream transfers until action 0x08, action 0x32, or room loading clears it.
	_state.InteractionFlags |= kForegroundOverrideMask;

	// Ghidra 0x000044CA-0x000044F5: decode all 70 authored tiles and publish them at the packed base's low
	// 11-bit tile index. The operation-owned array replaces the FF3000/FF4000 shared workspace.
	Common::Array<uint8> packedTiles = decompressRingLz(
		_rom, kTileStreamOffset);
	uint16 tileBaseAttribute = _state.RoomDynamicTileBaseAttribute;
	_scene.loadTilesAt(MakeSpan(packedTiles), tileBaseAttribute & kTileIndexMask);

	// Ghidra 0x000044F6-0x00004509: replace all 64x32 plane-A cells with dynamic base plus one. The original
	// FillVramWords helper preserves D0=0xC000 for the following upper-left map overlay.
	_scene.fillLayer(
		decodeTileCell(static_cast<uint16>(tileBaseAttribute + 1)),
		TileLayer::Foreground);

	// Ghidra 0x0000450A-0x00004531: retain both 11-iteration DBF branches. Add the packed base with 16-bit
	// wrapping, then place the complete 11x11 map at row zero, column zero with a 64-cell stride.
	Common::Array<uint8> packedForeground =
		_rom.readBytes(kForegroundMapOffset, kForegroundMapColumnCount * kForegroundMapRowCount *
												 static_cast<int>(sizeof(uint16)))
			.toArray();
	for (std::size_t byteOffset = 0; byteOffset < packedForeground.size(); byteOffset += sizeof(uint16)) {
		WriteUInt16BigEndian(
			packedForeground, static_cast<int>(byteOffset),
			static_cast<uint16>(ReadUInt16BigEndian(packedForeground, static_cast<int>(byteOffset)) +
								tileBaseAttribute));
	}

	_scene.loadLayerRows(MakeSpan(packedForeground), TileLayer::Foreground, 0, 0,
						 kForegroundMapColumnCount, kForegroundMapRowCount);
	// Ghidra 0x00004532-0x00004533: return with the foreground override installed.
}

void RoomScriptForegroundOverrideExecutor::executeRoomScriptAction08() {
	// Ghidra 0x00004534-0x0000453B: clear only the authored foreground-override mode.
	_state.InteractionFlags &= static_cast<uint8>(~kForegroundOverrideMask);
	// Ghidra 0x0000453C-0x0000453D: return with every other interaction flag preserved.
}
} // namespace Scooby