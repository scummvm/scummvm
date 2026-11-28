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

#ifndef SCOOBY_ROOMS_VBLANK_HANDLER_H
#define SCOOBY_ROOMS_VBLANK_HANDLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/input/controller_input.h"
#include "scooby/interactions/action_menu_controller.h"
#include "scooby/rooms/actor_updater.h"
#include "scooby/rooms/object_tile_patch_renderer.h"
#include "scooby/rooms/sprite_renderer.h"
#include "scooby/rooms/tile_streamer.h"
#include "scooby/runtime/runtime_state.h"

// Runs the installed room retrace callback and preserves its ordered domain updates.
//
// Raw callback address 0x0000A65C is installed by Ghidra RunStartupSequence and CommitSelectedRoom. Logical
// scene and sprite owners replace the callback's VDP table-base writes; the callback retains its room-update
// gate, input sampling, script latch, and signed countdown lifecycles.

namespace Scooby {

class RoomVBlankHandler {
public:
	// Binds room retrace sequencing to the input and shared-state owners it updates.
	// rom: verified cartridge data containing the immutable action-prompt cell templates.
	// input: backend-neutral controller sampler invoked on every room retrace.
	// objectTilePatches: owner of pending room-object tile-map mutations and publication.
	// scene: logical scene containing the live palette published by room updates.
	// sprites: owner of actor tile and room sprite publication.
	// state: shared room flags and countdowns updated by the callback.
	// tileStreamer: owner of scrolling publication and newly exposed room-edge tiles.
	// actionMenu: owner of the staged action-prompt tile publication.
	// roomActorUpdater: owner of the per-retrace actor and interactive interface update.
	RoomVBlankHandler(const ScoobyDooRom &rom, ControllerInput &input,
					  RoomObjectTilePatchRenderer &objectTilePatches, TileScene &scene,
					  RoomSpriteRenderer &sprites, RuntimeState &state,
					  RoomTileStreamer &tileStreamer, ActionMenuController &actionMenu,
					  RoomActorUpdater &roomActorUpdater)
		: _actionMenu(actionMenu), _roomActorUpdater(roomActorUpdater),
		  _beforeRetraceCallback(this, &RoomVBlankHandler::handleRoomVBlank), _rom(rom),
		  _input(input), _objectTilePatches(objectTilePatches), _scene(scene), _sprites(sprites),
		  _state(state), _tileStreamer(tileStreamer), _walkAreaScene(nullptr), _walkAreaRoomId(-1),
		  _walkAreaCameraX(0), _walkAreaCameraY(0) {
	}

	// Services one room retrace in the original callback order.
	//
	// Ghidra: handleRoomVBlank (0x0000A65C).
	void handleRoomVBlank();

	// Returns handleRoomVBlank as the before-retrace callback taken by FramePresenter fades and waits.
	const Common::Functor0<void> *beforeRetraceCallback() const { return &_beforeRetraceCallback; }

private:
	static const uint8 kActionPromptPublicationPendingMask = 0x20;
	static const int kActionPromptColumnCount = 2;
	static const int kActionPromptRowCount = 3;
	static const int kActionPromptStartColumn = 59;
	static const int kActionPromptTemplateModeByteStride = 0x300;
	static const int kActionPromptTemplateRowByteStride = 0x80;
	static const int kLeftActionPromptStartRow = 21;
	static const int kLeftActionPromptTemplateOffset = 0x3008C;
	static const uint8 kObjectAnimationMask = 0x04;
	static const uint8 kRoomObjectAnimationExpiredMask = 0x04;
	static const uint8 kRoomObjectFixedActorMask = 0x02;
	static const uint16 kRoomObjectPatchClearAfterRestoreMask = 0x4000;
	static const uint16 kRoomObjectPatchDescriptorMask = 0xBFFF;
	static const uint8 kRoomObjectPatchPendingMask = 0x01;
	static const uint16 kRoomObjectPatchRestoreMask = 0x8000;
	static const int kRightActionPromptStartRow = 24;
	static const int kRightActionPromptTemplateOffset = 0x3020C;
	static const uint8 kRoomUpdateEnabledMask = 0x40;

	// Advances every enabled palette-range timer and rotates each range whose timer expires.
	//
	// Ghidra: rotateAnimatedPaletteRanges (0x0000A7CE). The original mutates inclusive subranges of
	// g_awCurrentPalette at 0xFF06F8-0xFF0777, then transfers only each changed range to CRAM. The logical
	// scene owns both the live colors and their immediate publication.
	void rotateAnimatedPaletteRanges();

	// Managed-only debug overlay (Console "showWalkArea"): publishes the visible part of the room's walk area
	// to the scene for the next frame.
	void updateWalkAreaOverlay();

	// Publishes a staged action-prompt update when its pending latch is set.
	//
	// Ghidra: commitPendingActionPromptDuringVBlank (0x00005C8E). Native scanlines at or before 0xE0 and
	// after 0xF6 defer the transfer without clearing display bit five. The logical scene has no unsafe DMA
	// interval, so execution inside this installed retrace callback is the managed accepted window. Its
	// direct call preserves the original fallthrough into CommitActionPromptTiles.
	void commitPendingActionPromptDuringVBlank();

	// Advances both prompt countdowns and publishes their current interface cells.
	//
	// Ghidra: updateActionPromptTimers (0x0000A704). ROM 0x00030016-0x00030915 contains three six-row,
	// 64-cell modes. Each prompt selects its mode with a wrapping signed-word 0x300-byte stride; the left
	// and right blocks publish columns 59-60 of rows 21-23 and 24-26 after adding the current prompt tile
	// attributes.
	void updateActionPromptTimers();

	void publishActionPromptCells(int16 promptIndex, int templateOffset, int startRow);

	// Advances every room-object countdown and services eligible pending tile patches.
	//
	// Ghidra: updateRoomObjectAnimations (0x00007C04). The decoded object array preserves the original
	// inclusive g_wLastRoomObjectIndex traversal. Patch publication receives the index after bit 14 is
	// cleared, while the record retains its original bit 14 for the post-publication clear branch.
	void updateRoomObjectAnimations();

	ActionMenuController &_actionMenu;
	RoomActorUpdater &_roomActorUpdater;
	Common::Functor0Mem<void, RoomVBlankHandler> _beforeRetraceCallback;
	const ScoobyDooRom &_rom;
	ControllerInput &_input;
	RoomObjectTilePatchRenderer &_objectTilePatches;
	TileScene &_scene;
	RoomSpriteRenderer &_sprites;
	RuntimeState &_state;
	RoomTileStreamer &_tileStreamer;

	// Managed-only debug overlay cache: the walk area over the whole room (rebuilt per room, or when Shaggy
	// leaves it) and its visible part (rebuilt when the camera moves).
	Common::Array<uint8> _walkAreaRoom;
	Common::Array<uint8> _walkAreaOverlay;
	const RoomSceneData *_walkAreaScene;
	int16 _walkAreaRoomId;
	int16 _walkAreaCameraX;
	int16 _walkAreaCameraY;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_VBLANK_HANDLER_H
