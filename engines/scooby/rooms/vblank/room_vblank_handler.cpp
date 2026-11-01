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
#include "scooby/rooms/room_collision_probe.h"
#include "scooby/rooms/room_object.h"
#include "scooby/scooby.h"

namespace Scooby {

void RoomVBlankHandler::handleRoomVBlank() {
	// Ghidra 0x0000A65C-0x0000A673: restore foreground, background, and sprite-table VDP bases
	// 0xC000, 0xE000, and 0xD800. The managed logical layer and sprite owners bind those destinations.

	// Ghidra 0x0000A674-0x0000A677: publish scrolling and any newly exposed room-edge tiles.
	_tileStreamer.updateScrollingAndStreamTiles();

	// Managed-only debug overlay (Console "showWalkArea").
	updateWalkAreaOverlay();

	// Ghidra 0x0000A678-0x0000A683: a clear room-update gate skips the complete conditional block and
	// continues with controller polling, script latching, and countdown service.
	if ((_state.displayFlags & kRoomUpdateEnabledMask) != 0) {
		// Ghidra 0x0000A684-0x0000A697: close the gate during palette, prompt, and sprite publication.
		_state.displayFlags &= static_cast<uint8>(~kRoomUpdateEnabledMask);
		rotateAnimatedPaletteRanges();
		commitPendingActionPromptDuringVBlank();
		_sprites.refreshRoomSprites();

		// Ghidra 0x0000A698-0x0000A69F: lower the original interrupt mask and update prompt timers.
		// Managed callback execution is already non-reentrant, so only the ordered timer call remains.
		updateActionPromptTimers();

		// Ghidra 0x0000A6A0-0x0000A6AF: preserve both object-animation branches.
		if ((_state.displayFlags & kObjectAnimationMask) != 0) {
			updateRoomObjectAnimations();
		}

		// Ghidra 0x0000A6B0-0x0000A6BF: update room actors and interface state, reopen the gate, and
		// replace the final interrupt-mask restoration with ordinary managed callback return semantics.
		SDM_ASSERT(_updateRoomActorsAndInterface != nullptr,
				   "The room VBlank actor-update owner has not been bound.");
		(*_updateRoomActorsAndInterface)();
		_state.displayFlags |= kRoomUpdateEnabledMask;
	}

	// Ghidra 0x0000A6C0-0x0000A6D7: sample active-low controls and latch room-script bit zero only
	// while Start is held; the released branch leaves every script flag unchanged.
	_input.pollControllers(_state, InputScreen::Room);
	if (_state.isButtonDown(ControllerButtons::Start)) {
		_state.roomScriptFlags |= 0x01;
	}

	// Ghidra 0x0000A6D8-0x0000A6E9: decrement a nonnegative signed byte through -1; preserve a
	// previously negative countdown unchanged.
	if (_state.auxiliaryRetraceCountdown >= 0) {
		_state.auxiliaryRetraceCountdown--;
	}

	// Ghidra 0x0000A6EA-0x0000A6F1: release every WaitForVerticalBlank caller on this retrace.
	_state.displayFlags &= 0xFE;

	// Ghidra 0x0000A6F2-0x0000A703: decrement a nonnegative signed word through -1, preserve a
	// previously negative countdown unchanged, and return.
	if (_state.retraceCountdown >= 0) {
		_state.retraceCountdown--;
	}
}

void RoomVBlankHandler::updateWalkAreaOverlay() {
	// Managed-only debug overlay (Console "showWalkArea"): the points Shaggy may walk to. The original
	// lead-actor movement tests one point at a time (RoomCollisionProbe::probeLeadActorCollision), and the
	// collision map only blocks walls, so the area shown is the clear region connected to where Shaggy stands.
	if (!g_engine->isWalkAreaShown() || !_state.activeRoomScene || _state.roomWidthTiles == 0 ||
		_state.roomHeightTiles == 0) {
		_walkAreaScene = nullptr;
		return;
	}

	const int roomWidth = _state.roomWidthTiles * 8;
	const int roomHeight = _state.roomHeightTiles * 8;
	const int shaggyX = static_cast<int16>(_state.actorXFixedCoordinates[0] >> 16);
	const int shaggyY = static_cast<int16>(_state.actorYFixedCoordinates[0] >> 16);
	const bool shaggyInRoom = shaggyX >= 0 && shaggyY >= 0 && shaggyX < roomWidth && shaggyY < roomHeight;
	bool rebuildRoom = _walkAreaScene != _state.activeRoomScene.get() || _walkAreaRoomId != _state.roomId ||
					   _walkAreaRoom.size() != static_cast<size_t>(roomWidth * roomHeight);
	// Shaggy moved to another region (scripts can place him anywhere): flood from his new position.
	if (!rebuildRoom && shaggyInRoom && _walkAreaRoom[static_cast<size_t>(shaggyY * roomWidth + shaggyX)] == 0) {
		rebuildRoom = true;
	}

	if (rebuildRoom) {
		_walkAreaScene = _state.activeRoomScene.get();
		_walkAreaRoomId = _state.roomId;
		const int16 maximumX = static_cast<int16>(roomWidth - 1);
		const int16 maximumY = static_cast<int16>(roomHeight - 1);
		RoomCollisionProbe probe(_rom, _state);
		Common::Array<uint8> clear(static_cast<size_t>(roomWidth * roomHeight));
		for (int y = 0; y < roomHeight; y++) {
			for (int x = 0; x < roomWidth; x++) {
				clear[static_cast<size_t>(y * roomWidth + x)] =
					probe.probeShaggyActorCollision(x << 16, y << 16, maximumX, maximumY).isClear ? 1 : 0;
			}
		}

		// Flood the clear points connected to Shaggy; with no clear starting point show every clear point.
		if (!shaggyInRoom || clear[static_cast<size_t>(shaggyY * roomWidth + shaggyX)] == 0) {
			_walkAreaRoom = clear;
		} else {
			_walkAreaRoom = Common::Array<uint8>(static_cast<size_t>(roomWidth * roomHeight), 0);
			Common::Array<int32> pending;
			pending.push_back(shaggyY * roomWidth + shaggyX);
			_walkAreaRoom[static_cast<size_t>(shaggyY * roomWidth + shaggyX)] = 1;
			while (!pending.empty()) {
				int32 point = pending.back();
				pending.pop_back();
				int x = point % roomWidth;
				int y = point / roomWidth;
				const int32 neighbours[4] = {x > 0 ? point - 1 : -1, x + 1 < roomWidth ? point + 1 : -1,
											 y > 0 ? point - roomWidth : -1,
											 y + 1 < roomHeight ? point + roomWidth : -1};
				for (int i = 0; i < 4; i++) {
					int32 next = neighbours[i];
					if (next >= 0 && clear[static_cast<size_t>(next)] != 0 && _walkAreaRoom[static_cast<size_t>(next)] == 0) {
						_walkAreaRoom[static_cast<size_t>(next)] = 1;
						pending.push_back(next);
					}
				}
			}
		}
	}

	const int width = _scene.walkAreaOverlayWidth();
	const int height = TileScene::walkAreaOverlayHeight();
	if (rebuildRoom || _walkAreaCameraX != _state.cameraX || _walkAreaCameraY != _state.cameraY ||
		_walkAreaOverlay.size() != static_cast<size_t>(width * height)) {
		_walkAreaCameraX = _state.cameraX;
		_walkAreaCameraY = _state.cameraY;
		_walkAreaOverlay.resize(static_cast<size_t>(width * height));
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int worldX = x + _state.cameraX;
				int worldY = y + _state.cameraY;
				bool walkable = worldX >= 0 && worldY >= 0 && worldX < roomWidth && worldY < roomHeight &&
								_walkAreaRoom[static_cast<size_t>(worldY * roomWidth + worldX)] != 0;
				_walkAreaOverlay[static_cast<size_t>(y * width + x)] = walkable ? 1 : 0;
			}
		}
	}

	_scene.setWalkAreaOverlay(makeSpan(_walkAreaOverlay));
}

void RoomVBlankHandler::rotateAnimatedPaletteRanges() {
	// Ghidra 0x0000A7CE-0x0000A7D3: initialize the descending five-slot traversal at slot four.
	for (int rangeIndex = static_cast<int>(_state.animatedPaletteRangeCountdowns.size()) - 1; rangeIndex >=
																							  0;
		 rangeIndex--) {
		// Ghidra 0x0000A7D4-0x0000A7DD: retain the independent disabled and enabled paths for every slot.
		uint8 rangeMask = static_cast<uint8>(1 << rangeIndex);
		if ((_state.animatedPaletteRangeFlags & rangeMask) == 0) {
			continue;
		}

		// Ghidra 0x0000A7DE-0x0000A7F1: decrement with word wrapping, skip nonnegative results, and
		// reload the matching interval only after the live countdown becomes negative.
		_state.animatedPaletteRangeCountdowns[static_cast<size_t>(rangeIndex)]--;
		if (_state.animatedPaletteRangeCountdowns[static_cast<size_t>(rangeIndex)] >= 0) {
			continue;
		}

		_state.animatedPaletteRangeCountdowns[static_cast<size_t>(rangeIndex)] =
			_state.animatedPaletteRangeIntervals[static_cast<size_t>(rangeIndex)];

		// Ghidra 0x0000A7F2-0x0000A81F: rotate the inclusive unsigned start/end range left by one color.
		// TileScene mutation replaces both the Work RAM copy loop and its exact-range CRAM publication.
		uint16 startIndex = _state.animatedPaletteRangeStartIndices[static_cast<size_t>(
			rangeIndex)];
		uint16 colorCount = static_cast<uint16>(
			_state.animatedPaletteRangeEndIndices[static_cast<size_t>(rangeIndex)] - startIndex + 1);
		_scene.rotatePaletteRangeLeft(startIndex, colorCount);
	}

	// Ghidra 0x0000A820-0x0000A827: the managed loop performs the same descending slot step and returns
	// after slot zero, including iterations skipped by either preceding branch.
}

void RoomVBlankHandler::commitPendingActionPromptDuringVBlank() {
	// Ghidra 0x00005C8E-0x00005C99: retain the clear-pending return and set-pending continuation.
	if ((_state.displayFlags & kActionPromptPublicationPendingMask) == 0) {
		return;
	}

	// Ghidra 0x00005C9A-0x00005CAD: the native VDP-counter branches defer at scanlines <=0xE0 and >0xF6.
	// Logical scene updates are atomic and have no unsafe scanline, so this installed managed retrace callback
	// is the high-level accepted publication window rather than a VDP counter model.

	// Ghidra 0x00005CAE-0x00005CB5: clear the pending latch and retain the deliberate fallthrough into the
	// separately recovered 0x00005CB6 prompt-tile publication body.
	_state.displayFlags &= static_cast<uint8>(~kActionPromptPublicationPendingMask);
	SDM_ASSERT(_commitActionPromptTiles != nullptr,
			   "The room VBlank prompt-tile owner has not been bound.");
	(*_commitActionPromptTiles)();
}

void RoomVBlankHandler::updateActionPromptTimers() {
	// Ghidra 0x0000A704-0x0000A71B: either independent suppression bit skips both countdowns and both
	// publications. Preserve the two tests rather than combining their state domains.
	if ((_state.interactionFlags & 0x04) != 0) {
		return;
	}

	if ((_state.progressStateBytes[0] & 0x02) != 0) {
		return;
	}

	// Ghidra 0x0000A71C-0x0000A72D: decrement a nonnegative signed left countdown through -1, while a
	// previously negative value remains unchanged.
	if (_state.leftActionPromptCountdown >= 0) {
		_state.leftActionPromptCountdown--;
	}

	// Ghidra 0x0000A72E-0x0000A773: publish all six left cells even when its countdown was already negative.
	publishActionPromptCells(_state.leftActionPromptIndex, kLeftActionPromptTemplateOffset,
							 kLeftActionPromptStartRow);

	// Ghidra 0x0000A774-0x0000A785: retain the separate signed right-countdown branch.
	if (_state.rightActionPromptCountdown >= 0) {
		_state.rightActionPromptCountdown--;
	}

	// Ghidra 0x0000A786-0x0000A7CB: publish the independent six-cell right block unconditionally.
	publishActionPromptCells(_state.rightActionPromptIndex, kRightActionPromptTemplateOffset,
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
				_state.actionPromptTileAttributes);
			cells[static_cast<size_t>(column)] = decodeTileCell(
				packedCell);
		}

		_scene.replaceLayerRow(makeSpan(cells), TileLayer::Interface,
							   kActionPromptStartColumn,
							   startRow + row);
	}
}

void RoomVBlankHandler::updateRoomObjectAnimations() {
	// Ghidra 0x00007C04-0x00007C15: snapshot the room identity, first record, and inclusive final index.
	// The typed array snapshot carries the same fixed traversal cardinality without a packed RAM table.
	int16 roomIdSnapshot = _state.roomId;
	Common::Array<RoomObject> &roomObjects = _state.roomObjects;

	for (size_t objectIndex = 0; objectIndex < roomObjects.size(); objectIndex++) {
		RoomObject &roomObject = roomObjects[objectIndex];

		// Ghidra 0x00007C16-0x00007C31: leave a previously negative countdown and bit two unchanged.
		// Otherwise decrement with word wrapping and set bit two only when the result becomes negative.
		if (roomObject.animationCountdown >= 0) {
			roomObject.animationCountdown--;
			if (roomObject.animationCountdown < 0) {
				roomObject.flags |= kRoomObjectAnimationExpiredMask;
			} else {
				roomObject.flags &= static_cast<uint8>(~kRoomObjectAnimationExpiredMask);
			}
		}

		// Ghidra 0x00007C32-0x00007C49: retain the pending-bit, unchanged-room, and fixed-actor gates as
		// separate branches before touching the descriptor index or pending flag.
		if ((roomObject.flags & kRoomObjectPatchPendingMask) == 0) {
			continue;
		}

		if (roomIdSnapshot != _state.roomId) {
			continue;
		}

		if ((roomObject.flags & kRoomObjectFixedActorMask) != 0) {
			continue;
		}

		// Ghidra 0x00007C4A-0x00007C6F: clear bit 14 only in the descriptor passed to the patch owner.
		// A set bit 15 selects the independent restore branch, whose original bit 14 can clear the whole
		// record word before bit 15 is removed.
		uint16 originalPatchIndex = static_cast<uint16>(roomObject.tilePatchIndex);
		int16 publishedPatchIndex =
			static_cast<int16>(originalPatchIndex & kRoomObjectPatchDescriptorMask);
		_objectTilePatches.applyRoomObjectTilePatch(publishedPatchIndex);
		if ((publishedPatchIndex & kRoomObjectPatchRestoreMask) != 0) {
			if ((originalPatchIndex & kRoomObjectPatchClearAfterRestoreMask) != 0) {
				roomObject.tilePatchIndex = 0;
			}

			roomObject.tilePatchIndex = static_cast<int16>(
				static_cast<uint16>(roomObject.tilePatchIndex) &
				static_cast<uint16>(~kRoomObjectPatchRestoreMask));
		}

		// Ghidra 0x00007C70-0x00007C75: every eligible call clears the pending bit after publication.
		roomObject.flags &= static_cast<uint8>(~kRoomObjectPatchPendingMask);
	}

	// Ghidra 0x00007C76-0x00007C80: advance by one 0x1A-byte record through the inclusive DBF bound and return.
}

} // namespace Scooby
