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

#ifndef SCOOBY_ROOM_SCRIPT_ACTION_1E_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_ACTION_1E_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns room-script action 0x1E's streamed reset presentation and handover.

namespace Scooby {

class RoomScriptAction1EExecutor {
public:
	// Binds the authored streams to their room presentation and process-reset owners.
	// rom: verified cartridge containing both Ring-LZ streams.
	// scene: logical tile scene receiving dynamic patterns and cyclic foreground rows.
	// state: shared scroll, countdown, display, and dynamic-tile state.
	// presenter: shared retrace and palette-transition owner.
	// updateActorAnimationFrames: recovered actor-animation update retained during the sequence.
	// handleRoomVBlank: installed room callback run at every recovered retrace boundary.
	// restartApplication: managed owner of the non-returning original reset handover.
	// requestSessionExit: publishes the managed result of the original non-returning tail branch.
	RoomScriptAction1EExecutor(const ScoobyDooRom &rom, TileScene &scene,
							   RuntimeState &state, FramePresenter &presenter,
							   Common::Functor0<void> &updateActorAnimationFrames,
							   Common::Functor0<void> &handleRoomVBlank,
							   Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication,
							   Common::Functor1<SessionExit, void> &requestSessionExit)
		: _rom(rom), _scene(scene), _state(state), _presenter(presenter),
		  _updateActorAnimationFrames(updateActorAnimationFrames), _handleRoomVBlank(handleRoomVBlank),
		  _restartApplication(restartApplication), _requestSessionExit(requestSessionExit) {
	}

	// Streams the authored vertical sequence and hands control to application restart.
	//
	// Ghidra: executeRoomScriptAction1E (0x000041B6). Ring-LZ source 0x000296FA-0x00029C70 decodes to 106
	// packed tiles; source 0x00029C72-0x0002AF8D decodes to 317 rows of 32 cells. The original shared
	// workspace at 0xFF7000-0xFFBF3F becomes operation-owned decoded arrays. Room callbacks remain installed
	// throughout both fades and every synchronous wait.
	void executeRoomScriptAction1E();

private:
	static const int kCellsPerRow = 0x20;
	static const int kCellStreamOffset = 0x29C72;
	static const int kInitialDestinationRow = 0x1C;
	static const int kInitialWaitCounter = 0x1E;
	static const int kLayerRowMask = 0x1F;
	static const int kRowByteCount = 0x40; // kCellsPerRow * sizeof(ushort)
	static const int kScrollRowInterval = 8;
	static const int kTileStreamOffset = 0x296FA;

	bool waitForRoomVerticalBlank();

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor1<const Common::Functor0<void> *, bool> &_restartApplication;
	Common::Functor1<SessionExit, void> &_requestSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_ACTION_1E_EXECUTOR_H
