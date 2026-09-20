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

#ifndef SCOOBY_ROOM_INTERFACE_TRANSITION_EXECUTOR_H
#define SCOOBY_ROOM_INTERFACE_TRANSITION_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/shared_graphics_workspace.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns the room/interface tile-block transition, its clocked timing, and its later restoration.

namespace Scooby {

class RoomInterfaceTransitionExecutor {
public:
	// Binds the transition lifecycle to shared room state and its asynchronous update owners.
	// rom: verified cartridge containing the immutable binary-mask source tiles.
	// scene: logical lower-interface layer saved, cleared, and restored by the transition.
	// graphicsWorkspace: aliased packed bytes shared with inventory-pattern composition.
	// state: progress, countdown, and saved tile-block state shared across the transition.
	// updateActorAnimationFrames: recovered actor-animation update retained during the wait.
	// waitForRoomVerticalBlank: callback-aware room frame boundary advancing the countdown.
	RoomInterfaceTransitionExecutor(const ScoobyDooRom &rom, TileScene &scene,
									SharedGraphicsWorkspace &graphicsWorkspace,
									RuntimeState &state,
									Common::Functor0<void> &updateActorAnimationFrames,
									Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _scene(scene), _graphicsWorkspace(graphicsWorkspace), _state(state),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Starts the authored interface tile-block transition and retains its restoration side.
	//
	// Ghidra: executeRoomScriptAction1D (0x0000482E). The original encoded destinations 0xAA80 and 0xAAC0
	// identify the left and right halves of a 6-row by 32-cell interface block; managed state retains that
	// binary identity without exposing a VRAM address.
	void executeRoomScriptAction1D();

	// Completes the room/interface transition and publishes its shared-workspace transfer.
	//
	// Ghidra: finishRoomStartup (0x00004862). The complete body is 0x00004862-0x000048B5. Reset leaves the
	// destination word at zero, so the six transfers target sparse pattern-memory rows until Action 0x1D
	// selects an interface half. Both domains consume the current aliased workspace bytes. Binary mask tiles
	// retain their immutable source, palette indices 15/1, 95-tile extent, and destination derived from
	// g_wInterfaceTileAttributes. Signed countdown 16 consumes 17 callback-aware clocked frames before
	// becoming negative.
	void finishRoomStartup();

	// Saves and clears the display-selected half of the lower-interface tile block.
	// Returns true when the right half was selected; otherwise false.
	//
	// Ghidra: saveAndClearInteractionTileBlock (0x000047AC). Display bit four clear selects VRAM address
	// 0xAAC0, the right half; set selects 0xAA80, the left half. The managed return value carries the selected
	// address retained in original register D4 to its caller.
	bool saveAndClearInteractionTileBlock();

private:
	static const uint8 kDisplaySideMask = 0x10;
	static const uint8 kInteractionDisplayMask = 0x08;
	static const int kInterfaceBlockColumnCount = 32;
	static const int kInterfaceBlockRowByteCount = kInterfaceBlockColumnCount * 2;
	static const int kInterfaceBlockRowCount = 6;
	static const int kInterfaceBlockStartRow = 21;
	static const uint16 kInterfaceMaskTileBaseDelta = 0x20;
	static const uint16 kInterfaceTileIndexMask = 0x07FF;
	static const uint8 kMaskNonzeroPaletteIndex = 0x0F;
	static const uint8 kMaskZeroPaletteIndex = 0x01;
	static const int kPatternDestinationRowByteStride = 0x80;
	static const int kRightInterfaceBlockStartColumn = 32;

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	SharedGraphicsWorkspace &_graphicsWorkspace;
	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_INTERFACE_TRANSITION_EXECUTOR_H
