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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_10_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_10_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/dialogue_controller.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns room-script command 0x10's dialogue and optional lead-actor transitions.

namespace Scooby {

class RoomScriptCommand10Executor {
public:
	// Binds authored dialogue records to shared animation, text, and logical tile owners.
	// rom: verified cartridge containing command records and episode dialogue.
	// scene: logical tile scene receiving regenerated dialogue mask patterns.
	// state: shared command cursor, actor state, interaction flags, and tile attributes.
	// dialogue: shared dialogue composition and dismissal owner.
	// updateActorAnimationFrames: recovered actor-animation interpreter used by transition waits.
	// waitForRoomVerticalBlank: callback-aware clocked frame advancing transition state.
	// requestSessionExit: publishes a managed non-returning dialogue handover.
	RoomScriptCommand10Executor(const ScoobyDooRom &rom, TileScene &scene,
								RuntimeState &state, DialogueController &dialogue,
								Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<bool> &waitForRoomVerticalBlank,
								Common::Functor1<SessionExit, void> &requestSessionExit)
		: _rom(rom), _scene(scene), _state(state), _dialogue(dialogue),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank), _requestSessionExit(requestSessionExit) {
	}

	// Presents one authored dialogue with optional hold setup and reversible actor movement.
	// mode: zero scans the eight-byte record; nonzero executes every encoded branch.
	//
	// Ghidra: executeRoomScriptCommand10 (0x00004D80). Flag bit 12 primes actor-zero hold
	// and interaction state. Bit 13 independently gates the transition before and after dialogue, bits
	// 14-15 select its temporary position, bits 4-5 replace tile palette bits 13-14, and bits 0-3 select
	// the mask's nonzero palette index. The managed scene receives the same 95 generated patterns without
	// retaining the original VRAM transport.
	void executeRoomScriptCommand10(int mode);

private:
	static const uint8 kLeadActorMask = 0x01;
	static const int kDialogueBaseField = 0x20;
	static const int kMaskTileBaseDelta = 0x20;
	static const uint16 kPaletteAttributePreservationMask = 0x9FFF;
	static const int kRecordSize = 8;

	void generateDialogueMaskTiles(uint8 nonzeroPaletteIndex);
	bool playLeadActorPositionTransition(int16 previousPositionIndex, int16 nextPositionIndex);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	DialogueController &_dialogue;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
	Common::Functor1<SessionExit, void> &_requestSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_10_EXECUTOR_H
