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

#include "room_script_action_1e_executor.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"
#include "scooby/compression/ring_lz_decoder.h"
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

void RoomScriptAction1EExecutor::executeRoomScriptAction1E() {
	// Ghidra 0x000041B6-0x000041BD: D0=0x001E drives 31 callback-aware retrace waits.
	for (int frameCounter = 0; frameCounter <= kInitialWaitCounter; frameCounter++) {
		if (!waitForRoomVerticalBlank()) {
			_requestSessionExit(SessionExit::HostClosed);
			return;
		}
	}

	// Ghidra 0x000041BE-0x000041C5: suppress normal scroll derivation and edge streaming.
	_state.VideoFlags |= 0x02;

	// Ghidra 0x000041C6-0x000041ED: decode 106 tiles and publish them at the packed base's tile index.
	Common::Array<uint8> packedTiles = decompressRingLz(
		_rom, kTileStreamOffset);
	_scene.loadTilesAt(MakeSpan(packedTiles), _state.RoomDynamicTileBaseAttribute & 0x07FF);

	// Ghidra 0x000041EE-0x0000420F: decode all 317 rows and add the packed base to every cell with original
	// word wrapping before any row is published.
	Common::Array<uint8> packedRows =
		decompressRingLz(_rom, kCellStreamOffset);
	uint16 tileBaseAttribute = _state.RoomDynamicTileBaseAttribute;
	for (std::size_t byteOffset = 0; byteOffset < packedRows.size(); byteOffset += sizeof(uint16)) {
		uint16 cell = ReadUInt16BigEndian(packedRows, static_cast<int>(byteOffset));
		WriteUInt16BigEndian(packedRows, static_cast<int>(byteOffset),
							 static_cast<uint16>(cell + tileBaseAttribute));
	}

	// Ghidra 0x00004210-0x0000424F: hold every signed vertical-scroll value for two retraces. After each
	// eighth increment, consume one 32-cell source row into the next cyclic foreground row; retain the
	// source-end branch after all 317 writes.
	int16 verticalScroll = 0;
	int destinationRow = kInitialDestinationRow;
	bool oddRetrace = false;
	std::size_t sourceByteOffset = 0;
	while (sourceByteOffset < packedRows.size()) {
		_state.ForegroundVerticalScroll = verticalScroll;
		_updateActorAnimationFrames();
		if (!waitForRoomVerticalBlank()) {
			_requestSessionExit(SessionExit::HostClosed);
			return;
		}

		oddRetrace = !oddRetrace;
		if (oddRetrace) {
			continue;
		}

		verticalScroll++;
		if ((verticalScroll & (kScrollRowInterval - 1)) != 0) {
			continue;
		}

		destinationRow = (destinationRow + 1) & kLayerRowMask;
		_scene.loadLayerRows(
			MakeSpan(packedRows).slice(sourceByteOffset, static_cast<std::size_t>(kRowByteCount)),
			TileLayer::Foreground, 0, destinationRow, kCellsPerRow, 1);
		sourceByteOffset += static_cast<std::size_t>(kRowByteCount);
	}

	// Ghidra 0x00004250-0x00004263: set the inclusive 300-retrace countdown, update animations before every
	// signed test, and retain the back edge until the installed callback decrements through -1.
	_state.RetraceCountdown = 0x012C;
	while (true) {
		_updateActorAnimationFrames();
		if (_state.RetraceCountdown < 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			_requestSessionExit(SessionExit::HostClosed);
			return;
		}
	}

	// Ghidra 0x00004264-0x0000426F: disable complete room updates and finish the action-owned fade.
	_state.DisplayFlags &= 0xBF;
	if (!_presenter.fadePaletteOut(&_handleRoomVBlank)) {
		_requestSessionExit(SessionExit::HostClosed);
		return;
	}

	// Ghidra 0x00004270-0x00004273: the original tail branch never resumes command dispatch; RestartApplication
	// performs its second fade and publishes the managed reset handover instead.
	_requestSessionExit(_restartApplication(&_handleRoomVBlank)
							? SessionExit::RestartRequested
							: SessionExit::HostClosed);
}

bool RoomScriptAction1EExecutor::waitForRoomVerticalBlank() {
	_handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}
} // namespace Scooby