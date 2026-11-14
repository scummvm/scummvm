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

#include "scooby/rooms/script/action_1e_executor.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"
#include "scooby/compression/ring_lz_decoder.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/types.h"

namespace Scooby {
void RoomScriptAction1EExecutor::executeRoomScriptAction1E() {
	// Ghidra 0x000041B6-0x000041BD: D0=0x001E drives 31 callback-aware retrace waits.
	for (int frameCounter = 0; frameCounter <= kInitialWaitCounter; frameCounter++) {
		if (!waitForRoomVerticalBlank()) {
			_requestSessionExit(SessionExit::HostClosed);
			return;
		}
	}

	// Ghidra 0x000041BE-0x000041C5: suppress normal scroll derivation and edge streaming.
	_state.videoFlags |= 0x02;

	// Ghidra 0x000041C6-0x000041ED: decode 106 tiles and publish them at the packed base's tile index.
	Common::Array<uint8> packedTiles = decompressRingLz(
		_rom, kTileStreamOffset);
	_scene.loadTilesAt(makeSpan(packedTiles), _state.roomDynamicTileBaseAttribute & 0x07FF);

	// Ghidra 0x000041EE-0x0000420F: decode all 317 rows and add the packed base to every cell with original
	// word wrapping before any row is published.
	Common::Array<uint8> packedRows =
		decompressRingLz(_rom, kCellStreamOffset);
	uint16 tileBaseAttribute = _state.roomDynamicTileBaseAttribute;
	for (size_t byteOffset = 0; byteOffset < packedRows.size(); byteOffset += sizeof(uint16)) {
		uint16 cell = readUInt16BigEndian(packedRows, static_cast<int>(byteOffset));
		writeUInt16BigEndian(packedRows, static_cast<int>(byteOffset),
							 static_cast<uint16>(cell + tileBaseAttribute));
	}

	// Ghidra 0x00004210-0x0000424F: hold every signed vertical-scroll value for two retraces. After each
	// eighth increment, consume one 32-cell source row into the next cyclic foreground row; retain the
	// source-end branch after all 317 writes.
	int16 verticalScroll = 0;
	int destinationRow = kInitialDestinationRow;
	bool oddRetrace = false;
	size_t sourceByteOffset = 0;
	while (sourceByteOffset < packedRows.size()) {
		_state.foregroundVerticalScroll = verticalScroll;
		_actorAnimator.updateActorAnimationFrames();
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
			makeSpan(packedRows).slice(sourceByteOffset, static_cast<size_t>(kRowByteCount)),
			TileLayer::Foreground, 0, destinationRow, kCellsPerRow, 1);
		sourceByteOffset += static_cast<size_t>(kRowByteCount);
	}

	// Ghidra 0x00004250-0x00004263: set the inclusive 300-retrace countdown, update animations before every
	// signed test, and retain the back edge until the installed callback decrements through -1.
	_state.retraceCountdown = 0x012C;
	while (true) {
		_actorAnimator.updateActorAnimationFrames();
		if (_state.retraceCountdown < 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			_requestSessionExit(SessionExit::HostClosed);
			return;
		}
	}

	// Ghidra 0x00004264-0x0000426F: disable complete room updates and finish the action-owned fade.
	_state.displayFlags &= 0xBF;
	if (!_presenter.fadePaletteOut(_roomVBlank.beforeRetraceCallback())) {
		_requestSessionExit(SessionExit::HostClosed);
		return;
	}

	// Ghidra 0x00004270-0x00004273: the original tail branch never resumes command dispatch; RestartApplication
	// performs its second fade and publishes the managed reset handover instead.
	_requestSessionExit(_restartApplication(_roomVBlank.beforeRetraceCallback())
							? SessionExit::RestartRequested
							: SessionExit::HostClosed);
}

bool RoomScriptAction1EExecutor::waitForRoomVerticalBlank() {
	_roomVBlank.handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}
} // namespace Scooby
