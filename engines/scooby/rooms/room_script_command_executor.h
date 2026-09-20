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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "actor_animation_descriptor_catalog.h"
#include "room_collision_probe.h"
#include "room_script_action_executor.h"
#include "room_script_command_01_executor.h"
#include "room_script_command_02_executor.h"
#include "room_script_command_05_executor.h"
#include "room_script_command_08_executor.h"
#include "room_script_command_09_executor.h"
#include "room_script_command_0b_executor.h"
#include "room_script_command_0e_or_1b_executor.h"
#include "room_script_command_0f_executor.h"
#include "room_script_command_10_executor.h"
#include "room_script_command_19_executor.h"
#include "room_script_command_1a_executor.h"
#include "room_script_command_1c_executor.h"
#include "room_script_command_1d_executor.h"
#include "room_script_command_1e_executor.h"
#include "room_script_command_1f_executor.h"
#include "room_script_command_20_executor.h"
#include "room_script_command_21_executor.h"
#include "room_script_condition_evaluator.h"
#include "room_tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/dialogue_controller.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns the executable room-script command mapping and its recovered handler boundaries.
//
// Ghidra g_apRoomScriptCommandHandlers at ROM 0x00002354-0x000023DB contains 34
// pointer entries. The managed table preserves every command index and the shared target used by commands
// 0x0E and 0x1B without treating executable lookup data as a ROM asset.

namespace Scooby {

class RoomScriptCommandExecutor {
public:
	// Builds the exact recovered command-index mapping once for this runtime session.
	// rom: verified cartridge containing the authored command records.
	// scene: logical tile scene updated by nested presentation actions.
	// state: shared script cursor and mutable runtime state.
	// collisionProbe: shared owner of decoded room collision queries.
	// movement: session-owned movement boundaries shared with ambient movement.
	// random: shared deterministic sequence advanced by commands that request a scaled value.
	// presenter: shared retrace and palette-transition owner used by dedicated actions.
	// updateActorAnimationFrames: recovered actor-animation update invoked by nested actions.
	// handleRoomVBlank: installed room callback that advances state during action busy waits.
	// presentRoom: canonical room-presentation owner called by nested Action02.
	// resetRoomStateForReload: canonical room-state reset owner called by command 0x18.
	// reloadAndPresentRoom: canonical room reload and presentation owner called by command 0x11.
	// processAutomaticInteractions: canonical automatic-interaction update called by nested Action04.
	// refreshInteractionDisplay: canonical interaction-display refresh called by nested Action05.
	// finishRoomStartup: existing owner of the action-table target at index 0x1C.
	// executeRoomScriptAction1D: transition owner selected by action-table index 0x1D.
	// dialogue: shared dialogue owner used by command 0x10.
	// invokeRoomScriptDispatchCallback: runtime owner of the scan or execution dispatcher installed by the
	//     current script caller.
	// commitActionPromptUpdate: recovered action-prompt commit boundary used by commands 0x05 and 0x07.
	// refreshActionPrompts: recovered action-prompt refresh boundary used by command 0x05.
	// drawSelectedActionIcon: recovered action-icon renderer used by command 0x09.
	// tileStreamer: owner of room-plane restoration called by nested Action05.
	// restartApplication: managed owner of Ghidra RestartApplication.
	RoomScriptCommandExecutor(
		const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
		const RoomCollisionProbe &collisionProbe, ScriptedRoomMovementExecutor &movement,
		DeterministicRandom &random, FramePresenter &presenter,
		Common::Functor0<void> &updateActorAnimationFrames, Common::Functor0<void> &handleRoomVBlank,
		Common::Functor0<Optional<SessionExit>> &presentRoom,
		Common::Functor0<Optional<SessionExit>> &resetRoomStateForReload,
		Common::Functor0<Optional<SessionExit>> &reloadAndPresentRoom,
		Common::Functor0<Optional<SessionExit>> &processAutomaticInteractions,
		Common::Functor0<void> &refreshInteractionDisplay, Common::Functor0<void> &finishRoomStartup,
		Common::Functor0<void> &executeRoomScriptAction1D, DialogueController &dialogue,
		Common::Functor1<int, Optional<SessionExit>> &invokeRoomScriptDispatchCallback,
		Common::Functor0<void> &commitActionPromptUpdate, Common::Functor0<void> &refreshActionPrompts,
		Common::Functor1<int, void> &drawSelectedActionIcon, RoomTileStreamer &tileStreamer,
		Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication);

	// Dispatches one command through its original table index.
	// command: unsigned command word read from the current script cursor.
	// mode: original D0 mode selected by the calling script loop.
	// interactionModeSnapshot: original D7 word retained across scan-mode command dispatch.
	// Returns the managed handover from an original non-returning nested action, otherwise no value.
	Optional<SessionExit> execute(uint16 command, int mode,
								  int16 interactionModeSnapshot);

private:
	static const int kHandlerCount = 34;

	// Consumes the one-word command record without changing command-specific state.
	// mode: ignored because scan and execution modes consume the same record.
	//
	// Ghidra: executeRoomScriptCommand00 (0x0000250C).
	void executeRoomScriptCommand00(int mode);

	// Forwards to the recovered command 0x01 owner with the retained interaction-mode snapshot.
	// mode: original D0 mode selected by the calling script loop.
	void executeRoomScriptCommand01(int mode) {
		_command01Executor.executeRoomScriptCommand01(mode, _interactionModeSnapshot);
	}

	// Chooses the fixed record end or its authored signed failure displacement.
	// mode: ignored because the shared condition record has the same meaning in either mode.
	//
	// Ghidra: executeRoomScriptCommand03 (0x00002732).
	void executeRoomScriptCommand03(int mode);

	// Runs a conditional nested script window and merges its resulting cursor into the parent window.
	// mode: selects the scan or execution dispatcher installed by the current script owner.
	//
	// Ghidra: executeRoomScriptCommand04 (0x0000274A). The original dynamically scoped
	// g_pRoomScriptDispatchCallback is represented by the runtime-owned callback rather than a mutable
	// executable pointer. Signed script displacements and the auto-flag early return retain their original
	// cursor and bound ownership.
	void executeRoomScriptCommand04(int mode);

	// Selects an actor animation directly or through its fixed-position room object.
	// mode: zero scans past the command; nonzero applies its animation selection.
	//
	// Ghidra: executeRoomScriptCommand06 (0x00002B5A). Each managed continuation services
	// the installed room callback and one clocked host frame because the original tight loops allow
	// asynchronous interrupt publication.
	void executeRoomScriptCommand06(int mode);

	// Marks an active object pending or aligns the inventory page for an inventory object.
	// mode: zero scans past the command; nonzero applies its object update.
	//
	// Ghidra: executeRoomScriptCommand07 (0x00002BF8). The typed inventory list replaces
	// the original contiguous words and their trailing -1 sentinel without removing its count path.
	void executeRoomScriptCommand07(int mode);

	// Replaces an object's inventory graphic and refreshes inventory prompts when required.
	// mode: zero scans past the command; nonzero applies its object update.
	//
	// Ghidra: executeRoomScriptCommand0A (0x00002DC0). The installed room callback and
	// host presenter replace the original vertical-blank handshake before the canonical prompt commit.
	void executeRoomScriptCommand0A(int mode);

	// Dispatches one authored nested room-script action and advances past its command record.
	// mode: zero scans past the command; nonzero executes its nested action.
	//
	// Ghidra: executeRoomScriptCommand0C (0x00002E9E). The executable 62-entry table at
	// 0x00002EB4-0x00002FAB is represented by RoomScriptActionExecutor rather than
	// read as cartridge asset data.
	void executeRoomScriptCommand0C(int mode);

	// Routes one encoded audio command to its play or stop boundary.
	// mode: zero scans past the command; nonzero dispatches its audio request.
	//
	// Ghidra: executeRoomScriptCommand0D (0x000048B6). Audio playback remains an explicitly
	// deferred nonthrowing domain, while the encoded sign branch and command ordering remain intact.
	void executeRoomScriptCommand0D(int mode);

	static void playAudioCommand(int command);
	static void stopAudioPlayback(int command);

	// Reloads a selected room from one explicit room, coordinate, and actor-position record.
	// mode: zero scans the eight-byte record; nonzero performs the fade and room reload.
	//
	// Ghidra: executeRoomScriptCommand11 (0x00004F0E). The coordinate index is shifted
	// within a word, preserving 16-bit wrap before LoadRoomScene consumes it as a byte offset.
	void executeRoomScriptCommand11(int mode);

	// Replaces one room object's complete shape index word.
	// mode: zero scans the six-byte record; nonzero applies its shape index.
	//
	// Ghidra: executeRoomScriptCommand12 (0x00004F50). The object identity is converted to
	// the original signed low-word byte offset before selecting the typed 0x1A-byte record.
	void executeRoomScriptCommand12(int mode);

	// Stops automatic-interaction scanning at its marker or follows the marker's signed displacement.
	// mode: zero recognizes an active automatic-interaction scan; nonzero follows the displacement.
	//
	// Ghidra: executeRoomScriptCommand13 (0x00002446). The automatic scan stop leaves the
	// cursor on this command so its owner can derive the nested execution window from the same record.
	void executeRoomScriptCommand13(int mode);

	// Replaces one room object's complete shape index word through command slot 0x14.
	// mode: zero scans the six-byte record; nonzero applies its shape index.
	//
	// Ghidra: executeRoomScriptCommand14 (0x00002E7E). This remains a distinct original
	// boundary even though its complete 32-byte body is identical to command 0x12.
	void executeRoomScriptCommand14(int mode);

	// Advances actor animations for an inclusive authored retrace countdown.
	// mode: zero scans the four-byte record; nonzero performs the timed animation wait.
	//
	// Ghidra: executeRoomScriptCommand15 (0x000048D8). The room callback and host retrace
	// replace the asynchronous interrupt that decrements the original signed countdown.
	void executeRoomScriptCommand15(int mode);

	// Blocks visibility for a direct actor or a room object's assigned actor slot.
	// mode: zero scans the four-byte record; nonzero applies its actor visibility block.
	//
	// Ghidra: executeRoomScriptCommand16 (0x00002AD0). Dynamic byte-bit selection remains
	// modulo eight, including the room object's -1 fixed-position sentinel branch.
	void executeRoomScriptCommand16(int mode);

	// Advances animations until a direct actor or room object's assigned actor reaches a hold command.
	// mode: zero scans the four-byte record; nonzero waits for the selected actor hold.
	//
	// Ghidra: executeRoomScriptCommand17 (0x00004FBE). The selected actor is updated at
	// least once, and dynamic byte-bit selection retains the fixed-position -1 sentinel branch.
	// The installed room callback and host presenter replace interrupt-driven delay progress on each
	// continuation of the native tight loop.
	void executeRoomScriptCommand17(int mode);

	bool waitForRoomVerticalBlank();

	// Selects a room and opening actor coordinate before resetting its runtime state.
	// mode: zero scans the eight-byte record; nonzero applies the room selection and reset.
	//
	// Ghidra: executeRoomScriptCommand18 (0x00004EE4). The coordinate index shifts within
	// a word before the separate room-state reset boundary consumes the selected room state.
	void executeRoomScriptCommand18(int mode);

	// Records the non-returning handover from a nested movement command.
	void requestMovementHostClose() {
		_requestedSessionExit = SessionExit::HostClosed;
	}

	// Records an explicit session-exit request from a nested command owner.
	// exit: the requested session-exit value.
	void requestSessionExit(SessionExit exit) {
		_requestedSessionExit = exit;
	}

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<Optional<SessionExit>> &_reloadAndPresentRoom;
	Common::Functor0<Optional<SessionExit>> &_resetRoomStateForReload;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor0<void> &_commitActionPromptUpdate;
	Common::Functor1<int, Optional<SessionExit>> &_invokeRoomScriptDispatchCallback;

	Common::Functor0Mem<bool, RoomScriptCommandExecutor> _waitForRoomVerticalBlankFunctor;
	Common::Functor0Mem<void, RoomScriptCommandExecutor> _requestMovementHostCloseFunctor;
	Common::Functor1Mem<SessionExit, void, RoomScriptCommandExecutor> _requestSessionExitFunctor;

	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command00Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command01Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command03Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command04Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command06Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command07Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command0AFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command0CFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command0DFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command11Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command12Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command13Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command14Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command15Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command16Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command17Functor;
	Common::Functor1Mem<int, void, RoomScriptCommandExecutor> _command18Functor;

	Common::Functor1Mem<int, void, RoomScriptCommand02Executor> _command02Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand05Executor> _command05Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand08Executor> _command08Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand09Executor> _command09Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand0BExecutor> _command0BFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand0EOr1BExecutor> _command0EFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand0FExecutor> _command0FFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand10Executor> _command10Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand19Executor> _command19Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand1AExecutor> _command1AFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand1CExecutor> _command1CFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand1DExecutor> _command1DFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand1EExecutor> _command1EFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand1FExecutor> _command1FFunctor;
	Common::Functor1Mem<int, void, RoomScriptCommand20Executor> _command20Functor;
	Common::Functor1Mem<int, void, RoomScriptCommand21Executor> _command21Functor;

	ActorAnimationDescriptorCatalog _actorDescriptorCatalog;
	RoomScriptCommand01Executor _command01Executor;
	RoomScriptCommand02Executor _command02Executor;
	RoomScriptCommand05Executor _command05Executor;
	RoomScriptCommand08Executor _command08Executor;
	RoomScriptCommand09Executor _command09Executor;
	RoomScriptCommand0BExecutor _command0BExecutor;
	RoomScriptCommand0EOr1BExecutor _command0EOr1BExecutor;
	RoomScriptCommand0FExecutor _command0FExecutor;
	RoomScriptCommand10Executor _command10Executor;
	RoomScriptCommand19Executor _command19Executor;
	RoomScriptCommand1AExecutor _command1AExecutor;
	RoomScriptCommand1CExecutor _command1CExecutor;
	RoomScriptCommand1DExecutor _command1DExecutor;
	RoomScriptCommand1EExecutor _command1EExecutor;
	RoomScriptCommand1FExecutor _command1FExecutor;
	RoomScriptCommand20Executor _command20Executor;
	RoomScriptCommand21Executor _command21Executor;
	RoomScriptActionExecutor _actions;
	RoomScriptConditionEvaluator _conditionEvaluator;

	Common::Array<Common::Functor1<int, void> *> _handlers;
	int16 _interactionModeSnapshot;
	Optional<SessionExit> _requestedSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_EXECUTOR_H
