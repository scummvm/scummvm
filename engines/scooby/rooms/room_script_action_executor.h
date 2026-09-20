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

#ifndef SCOOBY_ROOM_SCRIPT_ACTION_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_ACTION_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "duel_room_script_action_executor.h"
#include "room_collision_probe.h"
#include "room_script_action_05_or_0c_executor.h"
#include "room_script_action_1e_executor.h"
#include "room_script_action_27_executor.h"
#include "room_script_camera_pan_executor.h"
#include "room_script_coordinate_sprite_executor.h"
#include "room_script_foreground_override_executor.h"
#include "room_script_paired_actor_transition_executor.h"
#include "room_tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns the executable nested room-script action mapping and its recovered function boundaries.
//
// Ghidra g_apRoomScriptActionHandlers at ROM 0x00002EB4-0x00002FAB contains 62 pointer entries. The managed
// table preserves all indices, the shared target selected by 0x05 and 0x0C, and the existing
// FinishRoomStartup owner at 0x1C.

namespace Scooby {

class RoomScriptActionExecutor {
public:
	// Builds the exact recovered action-index mapping once for this runtime session.
	// rom: verified cartridge containing authored room-action assets.
	// scene: logical tile scene receiving nested action presentation updates.
	// state: shared runtime flags mutated by nested actions.
	// collisionProbe: shared owner of decoded room collision queries.
	// random: shared deterministic sequence advanced by random selections and duel impacts.
	// presenter: shared retrace and palette-transition owner used by dedicated actions.
	// updateActorAnimationFrames: recovered actor-animation update called by action handlers.
	// handleRoomVBlank: installed room callback that advances state during busy waits.
	// presentRoom: canonical room-presentation owner called by Action02.
	// processAutomaticInteractions: canonical automatic-interaction update called by Action04.
	// refreshInteractionDisplay: canonical interaction-display refresh called by Action05.
	// finishRoomStartup: existing owner of Ghidra FinishRoomStartup at 0x00004862, selected by action 0x1C
	//     and also called directly during room startup.
	// executeRoomScriptAction1D: transition owner selected by action-table index 0x1D.
	// tileStreamer: owner of room-plane restoration called by Action05.
	// restartApplication: managed owner of Ghidra RestartApplication.
	RoomScriptActionExecutor(const ScoobyDooRom &rom, TileScene &scene,
							 RuntimeState &state,
							 const RoomCollisionProbe &collisionProbe, DeterministicRandom &random,
							 FramePresenter &presenter,
							 Common::Functor0<void> &updateActorAnimationFrames,
							 Common::Functor0<void> &handleRoomVBlank,
							 Common::Functor0<Optional<SessionExit>> &presentRoom,
							 Common::Functor0<Optional<SessionExit>>
								 &processAutomaticInteractions,
							 Common::Functor0<void> &refreshInteractionDisplay,
							 Common::Functor0<void> &finishRoomStartup,
							 Common::Functor0<void> &executeRoomScriptAction1D, RoomTileStreamer &tileStreamer,
							 Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication);

	// Dispatches one nested action through its original table index.
	// action: unsigned action word read from the current command record.
	// Returns the managed handover from an original non-returning action, otherwise no value.
	Optional<SessionExit> execute(uint16 action);

private:
	static const int kHandlerCount = 62;

	void executeRoomScriptAction00();
	void executeRoomScriptAction01();
	void executeRoomScriptAction02();
	bool tryExecuteRoomScriptAction02();
	void executeRoomScriptAction03();
	void executeRoomScriptAction04();
	void executeRoomScriptAction06();
	void executeRoomScriptAction09();
	void executeRoomScriptAction0A();
	void executeRoomScriptAction0B();
	void executeRoomScriptAction11();
	void executeRoomScriptAction12();
	void executeRoomScriptAction13();
	void executeRoomScriptAction14();
	void executeRoomScriptAction1A();
	void executeRoomScriptAction1B();
	void executeRoomScriptAction1F();
	void executeRoomScriptAction20();
	void executeRoomScriptAction21();
	void executeRoomScriptAction22();
	void executeRoomScriptAction23();
	void executeRoomScriptAction24();
	void executeRoomScriptAction25();
	void executeRoomScriptAction26();
	void executeRoomScriptAction28();
	void executeRoomScriptAction29();
	void executeRoomScriptAction2A();
	void executeRoomScriptAction2B();
	void executeRoomScriptAction2C();
	void executeRoomScriptAction2D();
	void executeRoomScriptAction2E();
	void executeRoomScriptAction2F();
	void executeRoomScriptAction30();
	void executeRoomScriptAction31();
	void executeRoomScriptAction33();
	void executeRoomScriptAction34();
	void executeRoomScriptAction35();
	void executeRoomScriptAction36();
	void executeRoomScriptAction37();
	void executeRoomScriptAction38();
	void executeRoomScriptAction39();
	void executeRoomScriptAction3A();
	void executeRoomScriptAction3B();
	void executeRoomScriptAction3C();
	void executeRoomScriptAction3D();
	bool waitForRoomVerticalBlank();
	void requestSessionExit(SessionExit exit) { _requestedSessionExit = exit; }
	static void playAudioCommand(int command);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	DeterministicRandom &_random;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor0<Optional<SessionExit>> &_presentRoom;
	Common::Functor0<Optional<SessionExit>> &_processAutomaticInteractions;
	Common::Functor0<void> &_finishRoomStartup;

	Common::Functor0Mem<bool, RoomScriptActionExecutor> _waitForRoomVerticalBlankFunctor;
	Common::Functor1Mem<SessionExit, void, RoomScriptActionExecutor> _requestSessionExitFunctor;
	Common::Functor0Mem<bool, RoomScriptActionExecutor> _tryExecuteRoomScriptAction02Functor;

	Common::Functor0Mem<void, RoomScriptActionExecutor> _action00Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action01Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action02Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action03Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action04Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action06Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action09Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action0AFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action0BFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action11Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action12Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action13Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action14Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action1AFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action1BFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action1FFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action20Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action21Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action22Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action23Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action24Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action25Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action26Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action28Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action29Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2AFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2BFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2CFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2DFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2EFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action2FFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action30Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action31Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action33Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action34Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action35Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action36Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action37Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action38Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action39Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action3AFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action3BFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action3CFunctor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _action3DFunctor;

	Common::Functor0Mem<void, RoomScriptAction05Or0CExecutor> _action05Or0CFunctor;
	Common::Functor0Mem<void, RoomScriptForegroundOverrideExecutor> _action07Functor;
	Common::Functor0Mem<void, RoomScriptForegroundOverrideExecutor> _action08Functor;
	Common::Functor0Mem<void, RoomScriptCameraPanExecutor> _action0DFunctor;
	Common::Functor0Mem<void, RoomScriptCameraPanExecutor> _action0EFunctor;
	Common::Functor0Mem<void, RoomScriptCameraPanExecutor> _action0FFunctor;
	Common::Functor0Mem<void, RoomScriptCameraPanExecutor> _action10Functor;
	Common::Functor0Mem<void, RoomScriptPairedActorTransitionExecutor> _action15Functor;
	Common::Functor0Mem<void, RoomScriptPairedActorTransitionExecutor> _action16Functor;
	Common::Functor0Mem<void, RoomScriptCoordinateSpriteExecutor> _action17Functor;
	Common::Functor0Mem<void, RoomScriptCoordinateSpriteExecutor> _action18Functor;
	Common::Functor0Mem<void, RoomScriptCoordinateSpriteExecutor> _action19Functor;
	Common::Functor0Mem<void, RoomScriptAction1EExecutor> _action1EFunctor;
	Common::Functor0Mem<void, RoomScriptAction27Executor> _action27Functor;
	Common::Functor0Mem<void, DuelRoomScriptActionExecutor> _action32Functor;

	RoomScriptAction05Or0CExecutor _action05Or0CExecutor;
	RoomScriptAction1EExecutor _action1EExecutor;
	RoomScriptAction27Executor _action27Executor;
	RoomScriptCameraPanExecutor _cameraPanExecutor;
	RoomScriptCoordinateSpriteExecutor _coordinateSpriteExecutor;
	DuelRoomScriptActionExecutor _duelActionExecutor;
	RoomScriptForegroundOverrideExecutor _foregroundOverrideExecutor;
	RoomScriptPairedActorTransitionExecutor _pairedActorTransitionExecutor;

	Common::Array<Common::Functor0<void> *> _handlers;
	Optional<SessionExit> _requestedSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_ACTION_EXECUTOR_H
