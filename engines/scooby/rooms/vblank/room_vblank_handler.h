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

#ifndef SCOOBY_ROOM_VBLANK_HANDLER_H
#define SCOOBY_ROOM_VBLANK_HANDLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/input/controller_input.h"
#include "scooby/rooms/room_object_tile_patch_renderer.h"
#include "scooby/rooms/room_sprite_renderer.h"
#include "scooby/rooms/room_tile_streamer.h"
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
	RoomVBlankHandler(const ScoobyDooRom &rom, ControllerInput &input,
					  RoomObjectTilePatchRenderer &objectTilePatches, TileScene &scene,
					  RoomSpriteRenderer &sprites, RuntimeState &state,
					  RoomTileStreamer &tileStreamer)
		: _commitActionPromptTiles(nullptr), _updateRoomActorsAndInterface(nullptr), _rom(rom),
		  _input(input), _objectTilePatches(objectTilePatches), _scene(scene), _sprites(sprites),
		  _state(state), _tileStreamer(tileStreamer) {
	}

	// Binds the prompt-tile owner after the mutually dependent menu and VBlank owners exist.
	// commitActionPromptTiles: publishes the fully staged prompt patterns and interface cells.
	void bindActionPromptTileCommit(Common::Functor0<void> &commitActionPromptTiles) {
		_commitActionPromptTiles = &commitActionPromptTiles;
	}

	// Binds the distinct actor and interface updater after its action-menu dependency exists.
	// updateRoomActorsAndInterface: services all actor slots and interactive interface state.
	void bindRoomActorUpdate(Common::Functor0<void> &updateRoomActorsAndInterface) {
		_updateRoomActorsAndInterface = &updateRoomActorsAndInterface;
	}

	// Services one room retrace in the original callback order.
	//
	// Ghidra: handleRoomVBlank (0x0000A65C).
	void handleRoomVBlank();

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
	static const uint8 kStartButtonMask = 0x80;

	// Advances every enabled palette-range timer and rotates each range whose timer expires.
	//
	// Ghidra: rotateAnimatedPaletteRanges (0x0000A7CE). The original mutates inclusive subranges of
	// g_awCurrentPalette at 0xFF06F8-0xFF0777, then transfers only each changed range to CRAM. The logical
	// scene owns both the live colors and their immediate publication.
	void rotateAnimatedPaletteRanges();

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

	Common::Functor0<void> *_commitActionPromptTiles;
	Common::Functor0<void> *_updateRoomActorsAndInterface;
	const ScoobyDooRom &_rom;
	ControllerInput &_input;
	RoomObjectTilePatchRenderer &_objectTilePatches;
	TileScene &_scene;
	RoomSpriteRenderer &_sprites;
	RuntimeState &_state;
	RoomTileStreamer &_tileStreamer;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_VBLANK_HANDLER_H
