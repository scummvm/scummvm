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

#ifndef SCOOBY_ROOM_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "room_tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns the shared Action05/0C progress gate and Action05's temporary room-presentation lifecycle.

namespace Scooby {

class RoomScriptAction05Or0CExecutor {
public:
	// Binds the shared handler to its temporary graphics, input, interaction, and transition owners.
	// rom: verified cartridge containing the command record and temporary presentation assets.
	// scene: logical scene that replaces the original VRAM, CRAM, and sprite-table operations.
	// state: shared progress, camera, input, display, actor, and interaction state.
	// updateActorAnimationFrames: recovered actor-animation update called after room restoration.
	// waitForRoomVerticalBlank: callback-aware clocked frame used by every room wait.
	// refreshInteractionDisplay: canonical interaction-display refresh called after restoration.
	// executeRoomScriptAction1D: separate transition owner reached by both action branches.
	// tileStreamer: owner of the explicit room viewport redraw after content restoration.
	RoomScriptAction05Or0CExecutor(const ScoobyDooRom &rom, TileScene &scene,
								   RuntimeState &state,
								   Common::Functor0<void> &updateActorAnimationFrames,
								   Common::Functor0<bool> &waitForRoomVerticalBlank,
								   Common::Functor0<void> &refreshInteractionDisplay,
								   Common::Functor0<void> &executeRoomScriptAction1D,
								   RoomTileStreamer &tileStreamer)
		: _rom(rom), _scene(scene), _state(state), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank),
		  _refreshInteractionDisplay(refreshInteractionDisplay),
		  _executeRoomScriptAction1D(executeRoomScriptAction1D), _tileStreamer(tileStreamer) {
	}

	// Runs the shared progress gate, the complete Action05 presentation, or Action0C's direct transition.
	//
	// Ghidra: executeRoomScriptAction05Or0C (0x000045A4). The current command record remains the source of the
	// original (2,A5) action-word comparison. Restoring the temporary content retains the separate
	// DrawRoomViewport call after both camera words and the pattern snapshot. The function never writes VDP
	// register 12, so its 64x32 replacement planes remain behind the inherited H32 viewport while the
	// every-fourth-frame camera toggle selects between their 256-pixel halves.
	void executeRoomScriptAction05Or0C();

private:
	static const uint8 kAButtonMask = 0x40;
	static const uint8 kBButtonMask = 0x10;
	static const int kBackgroundHalfMapOffset = 0x2F97A;
	static const int kBackgroundHalfRowBytes = 64; // 32 * sizeof(ushort)
	static const int kBackgroundMapRows = 19;
	static const uint8 kInteractionRefreshMask = 0x02;
	static const int kPaletteColorCount = 64;
	static const int kPaletteOffset = 0x2F8FA;
	static const int kPatternByteCount = 0x3C3E; // 0x1E1F * sizeof(ushort)
	static const int kPatternBytesOffset = 0x2B33A;
	static const int kPlaneColumns = 64;
	static const int kPlaneRowBytes = 128; // kPlaneColumns * sizeof(ushort)
	static const int kPlaneRows = 32;
	static const uint8 kRoomUpdateEnabledMask = 0x40;
	static const uint16 kShortcutAction = 0x000C;
	static const int kForegroundMapOffset = 0x2EF7A;
	static const int kForegroundMapRows = 19;

	static const Common::Array<PaletteColor> kBlankPalette;

	bool waitForFrames(int frameCounter);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	Common::Functor0<void> &_refreshInteractionDisplay;
	Common::Functor0<void> &_executeRoomScriptAction1D;
	RoomTileStreamer &_tileStreamer;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H
