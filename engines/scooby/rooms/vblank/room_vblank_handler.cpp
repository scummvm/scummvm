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

#include "room_vblank_handler.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/contracts.h"
#include "scooby/common/span.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_cell.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/rooms/room_object.h"

namespace Scooby {

void RoomVBlankHandler::handleRoomVBlank() {
	// Ghidra 0x0000A65C-0x0000A673: restore foreground, background, and sprite-table VDP bases
	// 0xC000, 0xE000, and 0xD800. The managed logical layer and sprite owners bind those destinations.

	// Ghidra 0x0000A674-0x0000A677: publish scrolling and any newly exposed room-edge tiles.
	_tileStreamer.updateScrollingAndStreamTiles();

	// Ghidra 0x0000A678-0x0000A683: a clear room-update gate skips the complete conditional block and
	// continues with controller polling, script latching, and countdown service.
	if ((_state.DisplayFlags & kRoomUpdateEnabledMask) != 0) {
		// Ghidra 0x0000A684-0x0000A697: close the gate during palette, prompt, and sprite publication.
		_state.DisplayFlags &= static_cast<uint8>(~kRoomUpdateEnabledMask);
		rotateAnimatedPaletteRanges();
		commitPendingActionPromptDuringVBlank();
		_sprites.refreshRoomSprites();

		// Ghidra 0x0000A698-0x0000A69F: lower the original interrupt mask and update prompt timers.
		// Managed callback execution is already non-reentrant, so only the ordered timer call remains.
		updateActionPromptTimers();

		// Ghidra 0x0000A6A0-0x0000A6AF: preserve both object-animation branches.
		if ((_state.DisplayFlags & kObjectAnimationMask) != 0) {
			updateRoomObjectAnimations();
		}

		// Ghidra 0x0000A6B0-0x0000A6BF: update room actors and interface state, reopen the gate, and
		// replace the final interrupt-mask restoration with ordinary managed callback return semantics.
		SDM_ASSERT(_updateRoomActorsAndInterface != nullptr,
				   "The room VBlank actor-update owner has not been bound.");
		(*_updateRoomActorsAndInterface)();
		_state.DisplayFlags |= kRoomUpdateEnabledMask;
	}

	// Ghidra 0x0000A6C0-0x0000A6D7: sample active-low controls and latch room-script bit zero only
	// while Start is held; the released branch leaves every script flag unchanged.
	_input.pollControllers();
	if ((_state.ControllerOneInput & kStartButtonMask) == 0) {
		_state.RoomScriptFlags |= 0x01;
	}

	// Ghidra 0x0000A6D8-0x0000A6E9: decrement a nonnegative signed byte through -1; preserve a
	// previously negative countdown unchanged.
	if (_state.AuxiliaryRetraceCountdown >= 0) {
		_state.AuxiliaryRetraceCountdown--;
	}

	// Ghidra 0x0000A6EA-0x0000A6F1: release every WaitForVerticalBlank caller on this retrace.
	_state.DisplayFlags &= 0xFE;

	// Ghidra 0x0000A6F2-0x0000A703: decrement a nonnegative signed word through -1, preserve a
	// previously negative countdown unchanged, and return.
	if (_state.RetraceCountdown >= 0) {
		_state.RetraceCountdown--;
	}
}

void RoomVBlankHandler::rotateAnimatedPaletteRanges() {
	// Ghidra 0x0000A7CE-0x0000A7D3: initialize the descending five-slot traversal at slot four.
	for (int rangeIndex = static_cast<int>(_state.AnimatedPaletteRangeCountdowns.size()) - 1; rangeIndex >=
																							  0;
		 rangeIndex--) {
		// Ghidra 0x0000A7D4-0x0000A7DD: retain the independent disabled and enabled paths for every slot.
		uint8 rangeMask = static_cast<uint8>(1 << rangeIndex);
		if ((_state.AnimatedPaletteRangeFlags & rangeMask) == 0) {
			continue;
		}

		// Ghidra 0x0000A7DE-0x0000A7F1: decrement with word wrapping, skip nonnegative results, and
		// reload the matching interval only after the live countdown becomes negative.
		_state.AnimatedPaletteRangeCountdowns[static_cast<std::size_t>(rangeIndex)]--;
		if (_state.AnimatedPaletteRangeCountdowns[static_cast<std::size_t>(rangeIndex)] >= 0) {
			continue;
		}

		_state.AnimatedPaletteRangeCountdowns[static_cast<std::size_t>(rangeIndex)] =
			_state.AnimatedPaletteRangeIntervals[static_cast<std::size_t>(rangeIndex)];

		// Ghidra 0x0000A7F2-0x0000A81F: rotate the inclusive unsigned start/end range left by one color.
		// TileScene mutation replaces both the Work RAM copy loop and its exact-range CRAM publication.
		uint16 startIndex = _state.AnimatedPaletteRangeStartIndices[static_cast<std::size_t>(
			rangeIndex)];
		uint16 colorCount = static_cast<uint16>(
			_state.AnimatedPaletteRangeEndIndices[static_cast<std::size_t>(rangeIndex)] - startIndex + 1);
		_scene.rotatePaletteRangeLeft(startIndex, colorCount);
	}

	// Ghidra 0x0000A820-0x0000A827: the managed loop performs the same descending slot step and returns
	// after slot zero, including iterations skipped by either preceding branch.
}

void RoomVBlankHandler::commitPendingActionPromptDuringVBlank() {
	// Ghidra 0x00005C8E-0x00005C99: retain the clear-pending return and set-pending continuation.
	if ((_state.DisplayFlags & kActionPromptPublicationPendingMask) == 0) {
		return;
	}

	// Ghidra 0x00005C9A-0x00005CAD: the native VDP-counter branches defer at scanlines <=0xE0 and >0xF6.
	// Logical scene updates are atomic and have no unsafe scanline, so this installed managed retrace callback
	// is the high-level accepted publication window rather than a VDP counter model.

	// Ghidra 0x00005CAE-0x00005CB5: clear the pending latch and retain the deliberate fallthrough into the
	// separately recovered 0x00005CB6 prompt-tile publication body.
	_state.DisplayFlags &= static_cast<uint8>(~kActionPromptPublicationPendingMask);
	SDM_ASSERT(_commitActionPromptTiles != nullptr,
			   "The room VBlank prompt-tile owner has not been bound.");
	(*_commitActionPromptTiles)();
}

void RoomVBlankHandler::updateActionPromptTimers() {
	// Ghidra 0x0000A704-0x0000A71B: either independent suppression bit skips both countdowns and both
	// publications. Preserve the two tests rather than combining their state domains.
	if ((_state.InteractionFlags & 0x04) != 0) {
		return;
	}

	if ((_state.ProgressStateBytes[0] & 0x02) != 0) {
		return;
	}

	// Ghidra 0x0000A71C-0x0000A72D: decrement a nonnegative signed left countdown through -1, while a
	// previously negative value remains unchanged.
	if (_state.LeftActionPromptCountdown >= 0) {
		_state.LeftActionPromptCountdown--;
	}

	// Ghidra 0x0000A72E-0x0000A773: publish all six left cells even when its countdown was already negative.
	publishActionPromptCells(_state.LeftActionPromptIndex, kLeftActionPromptTemplateOffset,
							 kLeftActionPromptStartRow);

	// Ghidra 0x0000A774-0x0000A785: retain the separate signed right-countdown branch.
	if (_state.RightActionPromptCountdown >= 0) {
		_state.RightActionPromptCountdown--;
	}

	// Ghidra 0x0000A786-0x0000A7CB: publish the independent six-cell right block unconditionally.
	publishActionPromptCells(_state.RightActionPromptIndex, kRightActionPromptTemplateOffset,
							 kRightActionPromptStartRow);

	// Ghidra 0x0000A7CC: return after both prompt paths.
}

void RoomVBlankHandler::publishActionPromptCells(int16 promptIndex, int templateOffset, int startRow) {
	int16 modeByteOffset = static_cast<int16>(promptIndex *
											  kActionPromptTemplateModeByteStride);
	Common::Array<TileCell> cells(kActionPromptColumnCount);
	for (int row = 0; row < kActionPromptRowCount; row++) {
		int sourceOffset = templateOffset + modeByteOffset + row * kActionPromptTemplateRowByteStride;
		for (int column = 0; column < kActionPromptColumnCount; column++) {
			uint16 packedCell = static_cast<uint16>(
				_rom.readUInt16(sourceOffset + column * static_cast<int>(sizeof(uint16))) +
				_state.ActionPromptTileAttributes);
			cells[static_cast<std::size_t>(column)] = decodeTileCell(
				packedCell);
		}

		_scene.replaceLayerRow(MakeSpan(cells), TileLayer::Interface,
							   kActionPromptStartColumn,
							   startRow + row);
	}
}

void RoomVBlankHandler::updateRoomObjectAnimations() {
	// Ghidra 0x00007C04-0x00007C15: snapshot the room identity, first record, and inclusive final index.
	// The typed array snapshot carries the same fixed traversal cardinality without a packed RAM table.
	int16 roomIdSnapshot = _state.RoomId;
	Common::Array<RoomObject> &roomObjects = _state.RoomObjects;

	for (std::size_t objectIndex = 0; objectIndex < roomObjects.size(); objectIndex++) {
		RoomObject &roomObject = roomObjects[objectIndex];

		// Ghidra 0x00007C16-0x00007C31: leave a previously negative countdown and bit two unchanged.
		// Otherwise decrement with word wrapping and set bit two only when the result becomes negative.
		if (roomObject.AnimationCountdown >= 0) {
			roomObject.AnimationCountdown--;
			if (roomObject.AnimationCountdown < 0) {
				roomObject.Flags |= kRoomObjectAnimationExpiredMask;
			} else {
				roomObject.Flags &= static_cast<uint8>(~kRoomObjectAnimationExpiredMask);
			}
		}

		// Ghidra 0x00007C32-0x00007C49: retain the pending-bit, unchanged-room, and fixed-actor gates as
		// separate branches before touching the descriptor index or pending flag.
		if ((roomObject.Flags & kRoomObjectPatchPendingMask) == 0) {
			continue;
		}

		if (roomIdSnapshot != _state.RoomId) {
			continue;
		}

		if ((roomObject.Flags & kRoomObjectFixedActorMask) != 0) {
			continue;
		}

		// Ghidra 0x00007C4A-0x00007C6F: clear bit 14 only in the descriptor passed to the patch owner.
		// A set bit 15 selects the independent restore branch, whose original bit 14 can clear the whole
		// record word before bit 15 is removed.
		uint16 originalPatchIndex = static_cast<uint16>(roomObject.TilePatchIndex);
		int16 publishedPatchIndex =
			static_cast<int16>(originalPatchIndex & kRoomObjectPatchDescriptorMask);
		_objectTilePatches.applyRoomObjectTilePatch(publishedPatchIndex);
		if ((publishedPatchIndex & kRoomObjectPatchRestoreMask) != 0) {
			if ((originalPatchIndex & kRoomObjectPatchClearAfterRestoreMask) != 0) {
				roomObject.TilePatchIndex = 0;
			}

			roomObject.TilePatchIndex = static_cast<int16>(
				static_cast<uint16>(roomObject.TilePatchIndex) &
				static_cast<uint16>(~kRoomObjectPatchRestoreMask));
		}

		// Ghidra 0x00007C70-0x00007C75: every eligible call clears the pending bit after publication.
		roomObject.Flags &= static_cast<uint8>(~kRoomObjectPatchPendingMask);
	}

	// Ghidra 0x00007C76-0x00007C80: advance by one 0x1A-byte record through the inclusive DBF bound and return.
}

} // namespace Scooby