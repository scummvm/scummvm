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

#include "room_interface_transition_executor.h"

#include "scooby/common/span.h"
#include "scooby/graphics/binary_mask_tile_generator.h"
#include "scooby/graphics/genesis_asset_decoder.h"

namespace Scooby {

void RoomInterfaceTransitionExecutor::executeRoomScriptAction1D() {
	// Ghidra 0x0000482E-0x00004835: set only progress bit one and preserve every other authored bit.
	_state.ProgressStateBytes[0] |= 0x02;

	// Ghidra 0x00004836-0x0000483F: retain the signed nonnegative back edge. The original interrupt advances
	// this countdown asynchronously, so service one callback-aware clocked frame per continuation.
	while (_state.RoomInterfaceTransitionCountdown >= 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00004840-0x00004843: preserve the separate save-and-clear original-function boundary.
	bool savedTileBlockOnRight = saveAndClearInteractionTileBlock();

	// Ghidra 0x00004844-0x0000484B: begin the inclusive room/interface transition at signed count 16.
	_state.RoomInterfaceTransitionCountdown = 0x10;

	// Ghidra 0x0000484C-0x00004859: update every actor stream before each signed test and retain the complete
	// nonnegative back edge while clocked room retraces advance the transition.
	while (true) {
		_updateActorAnimationFrames();
		if (_state.RoomInterfaceTransitionCountdown < 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x0000485A-0x00004861: retain the selected block side for FinishRoomStartup and return.
	_state.SavedInteractionTileBlockOnRight = savedTileBlockOnRight;
}

void RoomInterfaceTransitionExecutor::finishRoomStartup() {
	// Ghidra 0x00004862-0x00004867: capture all three proved destination identities. Managed null preserves
	// reset word zero; Action1D replaces it with the left or right lower-interface selection.
	Optional<bool> savedBlockOnRight = _state.SavedInteractionTileBlockOnRight;

	// Ghidra 0x00004868-0x0000487D: start at signed count 16, update animations before every test, and
	// preserve the nonnegative back edge while callback-aware clocked frames advance asynchronous state.
	_state.RoomInterfaceTransitionCountdown = 0x10;
	while (true) {
		_updateActorAnimationFrames();
		if (_state.RoomInterfaceTransitionCountdown < 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x0000487E-0x00004897: preserve all six 32-word transfers and route their raw destination to its
	// logical domain. Zero selects six sparse pattern ranges; Action1D's values select one interface half.
	if (savedBlockOnRight.hasValue()) {
		_scene.loadLayerRows(_graphicsWorkspace.interfaceTileBlock(), TileLayer::Interface,
							 savedBlockOnRight.value() ? kRightInterfaceBlockStartColumn : 0,
							 kInterfaceBlockStartRow, kInterfaceBlockColumnCount, kInterfaceBlockRowCount);
	} else {
		for (int row = 0; row < kInterfaceBlockRowCount; ++row) {
			_scene.overwritePackedPatternBytes(
				_graphicsWorkspace.interfaceTileBlock().slice(row * kInterfaceBlockRowByteCount,
															  kInterfaceBlockRowByteCount),
				row * kPatternDestinationRowByteStride);
		}
	}

	// Ghidra 0x00004898-0x000048AB: regenerate the complete 95-tile binary mask with palette indices 15 and 1
	// at the tile base selected from the interface attributes plus the original 0x20-tile delta.
	int firstMaskTileIndex = (_state.InterfaceTileAttributes & kInterfaceTileIndexMask) +
							 kInterfaceMaskTileBaseDelta;
	Common::Array<uint8> binaryMaskTiles =
		generate(_rom, kMaskNonzeroPaletteIndex, kMaskZeroPaletteIndex);
	_scene.loadTilesAt(MakeSpan(binaryMaskTiles), firstMaskTileIndex);

	// Ghidra 0x000048AC-0x000048B5: clear only progress byte-zero bit one after every restoration side effect.
	_state.ProgressStateBytes[0] &= 0xFD;
}

bool RoomInterfaceTransitionExecutor::saveAndClearInteractionTileBlock() {
	// Ghidra 0x000047AC-0x000047CB: clear display bit three and retain both display-bit-four side branches.
	_state.DisplayFlags &= static_cast<uint8>(~kInteractionDisplayMask);
	bool savedBlockOnRight = (_state.DisplayFlags & kDisplaySideMask) == 0;
	int startColumn = savedBlockOnRight ? kRightInterfaceBlockStartColumn : 0;

	// Ghidra 0x000047CC-0x000047ED: save all 32 cells from each of six rows through the original 0x80-byte
	// row stride into the transition-owned lifetime of the shared workspace.
	_scene.copyPackedLayerRows(_graphicsWorkspace.interfaceTileBlock(), TileLayer::Interface,
							   startColumn,
							   kInterfaceBlockStartRow, kInterfaceBlockColumnCount, kInterfaceBlockRowCount);

	// Ghidra 0x000047EE-0x00004819: remove palette and priority bits, add the exact tile-index offset, and
	// replace every saved cell through the second complete six-row by 32-cell loop.
	uint16 replacementCellWord = static_cast<uint16>(
		(_state.InterfaceTileAttributes & kInterfaceTileIndexMask) + kInterfaceMaskTileBaseDelta);
	TileCell replacementCell = decodeTileCell(replacementCellWord);
	_scene.fillLayerRows(replacementCell, TileLayer::Interface, startColumn, kInterfaceBlockStartRow,
						 kInterfaceBlockColumnCount, kInterfaceBlockRowCount);
	return savedBlockOnRight;
}
} // namespace Scooby