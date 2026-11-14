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

#ifndef SCOOBY_ROOMS_SCRIPT_CONTROLLER_H
#define SCOOBY_ROOMS_SCRIPT_CONTROLLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/rooms/collision_probe.h"
#include "scooby/rooms/script/command_executor.h"
#include "scooby/rooms/tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/dialogue_controller.h"
#include "scooby/interactions/controller.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/rooms/actor_animator.h"
#include "scooby/rooms/interface_transition_executor.h"
#include "scooby/rooms/vblank/handler.h"
#include "scooby/runtime/runtime_state.h"

// Owns shared room-script cursor traversal and command dispatch.

namespace Scooby {

class RoomScriptController {
public:
	// Binds shared script traversal to the verified ROM and runtime state.
	// rom: verified cartridge containing the current authored script window.
	// scene: logical tile scene updated by nested presentation actions.
	// state: shared cursor, exclusive bound, and script control flags.
	// collisionProbe: shared owner of decoded room collision queries.
	// movement: session-owned movement boundaries shared with ambient movement.
	// random: shared deterministic sequence consumed by randomizing script commands.
	// presenter: shared retrace and palette-transition owner used by dedicated actions.
	// actorAnimator: owner of the recovered actor-animation update invoked by nested actions.
	// roomVBlank: owner of the installed room callback that advances state during action busy waits.
	// presentRoom: canonical room-presentation owner called by nested Action02.
	// resetRoomStateForReload: canonical room-state reset owner called by command 0x18.
	// reloadAndPresentRoom: canonical room reload and presentation owner called by command 0x11.
	// interactions: interaction owner called by nested Action04 and Action05.
	// roomInterfaceTransitions: owner of the nested action-table targets at indices 0x1C and 0x1D.
	// dialogue: shared dialogue owner used by room-script dialogue commands.
	// actionMenu: owner of the action-prompt commit (commands 0x05 and 0x07), action-prompt refresh
	//     (command 0x05), and action-icon renderer (command 0x09).
	// tileStreamer: owner of room-plane restoration called by nested Action05.
	// restartApplication: managed owner of Ghidra RestartApplication.
	RoomScriptController(const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
						 const RoomCollisionProbe &collisionProbe, ScriptedRoomMovementExecutor &movement,
						 DeterministicRandom &random, FramePresenter &presenter,
						 RoomActorAnimator &actorAnimator,
						 RoomVBlankHandler &roomVBlank,
						 Common::Functor0<Optional<SessionExit>> &presentRoom,
						 Common::Functor0<Optional<SessionExit>> &resetRoomStateForReload,
						 Common::Functor0<Optional<SessionExit>> &reloadAndPresentRoom,
						 InteractionController &interactions,
						 RoomInterfaceTransitionExecutor &roomInterfaceTransitions, DialogueController &dialogue,
						 ActionMenuController &actionMenu, RoomTileStreamer &tileStreamer,
						 Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication);

	// Scans the current script for initialization actions without consuming its window.
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Ghidra: runRoomScriptInitializationCommands (0x00002320). The complete body is
	// 0x00002320-0x00002353. The explicit mode dispatcher replaces the dynamically saved callback;
	// typed script offsets preserve the native A5/A6 save and restore around the complete scan.
	Optional<SessionExit> runRoomScriptInitializationCommands();

	// Selects and runs the current room's exit-script window under execution-mode dispatch.
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Ghidra: runRoomExitScript (0x0000221A). The complete body is
	// 0x0000221A-0x00002277. g_pActiveEpisodeDescriptor at 0xFF068E-0xFF0691
	// selects the room-record table at descriptor field +0x08 and script base at +0x2C.
	// Room-record field +0x0C is relative to that base; its script header contains a signed length
	// followed by an ignored word. The managed dispatch mode replaces dynamically writing executable
	// RunRoomScriptCommands address 0x00002406 to g_pRoomScriptDispatchCallback at
	// 0xFF0878-0xFF087B.
	Optional<SessionExit> runRoomExitScript();

	// Selects and runs the current room's entry-script window under execution-mode dispatch.
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Ghidra: runRoomEntryScript (0x00002278). The complete body is
	// 0x00002278-0x000022D5. g_pActiveEpisodeDescriptor at 0xFF068E-0xFF0691
	// selects the room-record table at descriptor field +0x08 and script base at +0x2C.
	// Room-record field +0x10 is relative to that base; its script header contains a signed length
	// followed by an ignored word. Explicit managed execution mode replaces the dynamically scoped native
	// RunRoomScriptCommands callback address at 0xFF0878-0xFF087B.
	Optional<SessionExit> runRoomEntryScript();

	// Runs the current room-script window until a command handler or state gate yields.
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Ghidra: runRoomScriptCommands (0x00002406). Managed script offsets preserve the
	// original A5 cursor and exclusive A6 bound. The 34-entry executable table at
	// 0x00002354-0x000023DB is represented by RoomScriptCommandExecutor rather than
	// read through the ROM asset owner; command slots 0x0E and 0x1B retain their shared target.
	Optional<SessionExit> runRoomScriptCommands();

	// Runs the current script window in scan mode (mode zero) or execution mode (any other value).
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Replaces the original g_pRoomScriptDispatchCallback indirection at 0xFF0878-0xFF087B.
	Optional<SessionExit> invokeRoomScriptDispatchCallback(int mode);

private:
	static const int kEpisodeRoomRecordTableField = 0x08;
	static const int kEpisodeRoomScriptBaseField = 0x2C;
	static const int kRoomEntryScriptField = 0x10;
	static const int kRoomExitScriptField = 0x0C;
	static const int kRoomRecordByteCount = 0x14;
	static const int kRoomScriptHeaderByteCount = 4;

	// Scans the current room-script window until its bound or automatic marker yields.
	// Returns the managed handover from an original non-returning command path, otherwise no value.
	//
	// Ghidra: dispatchRoomScriptCommands (0x000023DC). The complete body is
	// 0x000023DC-0x00002405. The native 34-entry executable table at
	// 0x00002354-0x000023DB is the existing RoomScriptCommandExecutor mapping;
	// slots 0x0E and 0x1B retain their shared target. Handlers own cursor movement, so this
	// scan deliberately adds no progress requirement before rechecking its exclusive bound.
	Optional<SessionExit> dispatchRoomScriptCommands();

	const ScoobyDooRom &_rom;
	RuntimeState &_state;

	Common::Functor1Mem<int, Optional<SessionExit>, RoomScriptController> _invokeRoomScriptDispatchCallbackFunctor;

	RoomScriptCommandExecutor _commands;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_SCRIPT_CONTROLLER_H
