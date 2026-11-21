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

#ifndef SCOOBY_ROOMS_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H
#define SCOOBY_ROOMS_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "scooby/rooms/tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/controller.h"
#include "scooby/rooms/actor_animator.h"
#include "scooby/rooms/interface_transition_executor.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/types.h"

// Owns the shared Action05/0C progress gate and Action05's temporary room-presentation lifecycle.

namespace Scooby {

class RoomScriptAction05Or0CExecutor {
public:
	// Binds the shared handler to its temporary graphics, input, interaction, and transition owners.
	// rom: verified cartridge containing the command record and temporary presentation assets.
	// scene: logical scene that replaces the original VRAM, CRAM, and sprite-table operations.
	// state: shared progress, camera, input, display, actor, and interaction state.
	// actorAnimator: owner of the recovered actor-animation update called after room restoration.
	// waitForRoomVerticalBlank: callback-aware clocked frame used by every room wait.
	// interactions: owner of the canonical interaction-display refresh called after restoration.
	// roomInterfaceTransitions: owner of the separate Action1D transition reached by both action branches.
	// tileStreamer: owner of the explicit room viewport redraw after content restoration.
	RoomScriptAction05Or0CExecutor(const ScoobyDooRom &rom, TileScene &scene,
								   RuntimeState &state,
								   RoomActorAnimator &actorAnimator,
								   Common::Functor0<bool> &waitForRoomVerticalBlank,
								   InteractionController &interactions,
								   RoomInterfaceTransitionExecutor &roomInterfaceTransitions,
								   RoomTileStreamer &tileStreamer)
		: _rom(rom), _scene(scene), _state(state), _actorAnimator(actorAnimator),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank),
		  _interactions(interactions), _roomInterfaceTransitions(roomInterfaceTransitions),
		  _tileStreamer(tileStreamer) {
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
	RoomActorAnimator &_actorAnimator;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	InteractionController &_interactions;
	RoomInterfaceTransitionExecutor &_roomInterfaceTransitions;
	RoomTileStreamer &_tileStreamer;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_SCRIPT_ACTION_05_OR_0C_EXECUTOR_H
