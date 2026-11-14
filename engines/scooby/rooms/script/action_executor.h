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

#ifndef SCOOBY_ROOMS_SCRIPT_ACTION_EXECUTOR_H
#define SCOOBY_ROOMS_SCRIPT_ACTION_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "scooby/rooms/script/duel_action_executor.h"
#include "scooby/rooms/collision_probe.h"
#include "scooby/rooms/script/action_05_or_0c_executor.h"
#include "scooby/rooms/script/action_1e_executor.h"
#include "scooby/rooms/script/action_27_executor.h"
#include "scooby/rooms/script/camera_pan_executor.h"
#include "scooby/rooms/script/coordinate_sprite_executor.h"
#include "scooby/rooms/script/foreground_override_executor.h"
#include "scooby/rooms/script/paired_actor_transition_executor.h"
#include "scooby/rooms/tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/controller.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/rooms/actor_animator.h"
#include "scooby/rooms/vblank/handler.h"
#include "scooby/runtime/runtime_state.h"

// Owns the executable nested room-script action mapping and its recovered function boundaries.
//
// Ghidra g_apRoomScriptActionHandlers at ROM 0x00002EB4-0x00002FAB contains 62 pointer entries. The switch
// in execute() preserves all indices, the shared target selected by 0x05 and 0x0C, and the existing
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
	// actorAnimator: owner of the recovered actor-animation update called by action handlers.
	// roomVBlank: owner of the installed room callback that advances state during busy waits.
	// presentRoom: canonical room-presentation owner called by Action02.
	// interactions: owner of the automatic-interaction update called by Action04 and the
	//     interaction-display refresh called by Action05.
	// roomInterfaceTransitions: owner of Ghidra FinishRoomStartup at 0x00004862 (action 0x1C, also called
	//     directly during room startup) and of the transition selected by action-table index 0x1D.
	// tileStreamer: owner of room-plane restoration called by Action05.
	// restartApplication: managed owner of Ghidra RestartApplication.
	RoomScriptActionExecutor(const ScoobyDooRom &rom, TileScene &scene,
							 RuntimeState &state,
							 const RoomCollisionProbe &collisionProbe, DeterministicRandom &random,
							 FramePresenter &presenter,
							 RoomActorAnimator &actorAnimator,
							 RoomVBlankHandler &roomVBlank,
							 Common::Functor0<Optional<SessionExit>> &presentRoom,
							 InteractionController &interactions,
							 RoomInterfaceTransitionExecutor &roomInterfaceTransitions,
							 RoomTileStreamer &tileStreamer,
							 Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication);

	// Dispatches one nested action through its original table index.
	// action: unsigned action word read from the current command record.
	// Returns the managed handover from an original non-returning action, otherwise no value.
	Optional<SessionExit> execute(uint16 action);

private:
	void scriptAction00NoOp();
	void executeRoomScriptAction01();
	void scriptAction02PresentRoom();
	bool tryExecuteScriptAction02PresentRoom();
	void scriptAction03FadeOut();
	void scriptAction04WaitForButtonPress();
	void executeRoomScriptAction06();
	void scriptAction09HideScoobyDoo();
	void scriptAction0AShowScoobyDoo();
	void scriptAction0BScoobyDooStartAnimation();
	void scriptAction11StopShaggyLeadMode();
	void scriptAction12StartShaggyLeadMode();
	void scriptAction13PlaySfx1();
	void scriptAction14PlaySfx2();
	void scriptAction1AStartVerticalScroll();
	void scriptAction1BStopVerticalScroll();
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
	void scriptAction3CEnableScoobyAndShaggyWalkingSideBySide();
	void scriptAction3DDisableScoobyAndShaggyWalkingSideBySide();
	bool waitForRoomVerticalBlank();
	void requestSessionExit(SessionExit exit) { _requestedSessionExit = exit; }
	static void playAudioCommand(int command);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	DeterministicRandom &_random;
	FramePresenter &_presenter;
	RoomActorAnimator &_actorAnimator;
	RoomVBlankHandler &_roomVBlank;
	Common::Functor0<Optional<SessionExit>> &_presentRoom;
	InteractionController &_interactions;
	RoomInterfaceTransitionExecutor &_roomInterfaceTransitions;

	Common::Functor0Mem<bool, RoomScriptActionExecutor> _waitForRoomVerticalBlankFunctor;
	Common::Functor1Mem<SessionExit, void, RoomScriptActionExecutor> _requestSessionExitFunctor;
	Common::Functor0Mem<bool, RoomScriptActionExecutor> _tryExecuteRoomScriptAction02Functor;
	Common::Functor0Mem<void, RoomScriptActionExecutor> _scriptAction03FadeOutFunctor;

	RoomScriptAction05Or0CExecutor _action05Or0CExecutor;
	RoomScriptAction1EExecutor _action1EExecutor;
	RoomScriptAction27Executor _action27Executor;
	RoomScriptCameraPanExecutor _cameraPanExecutor;
	RoomScriptCoordinateSpriteExecutor _coordinateSpriteExecutor;
	DuelRoomScriptActionExecutor _duelActionExecutor;
	RoomScriptForegroundOverrideExecutor _foregroundOverrideExecutor;
	RoomScriptPairedActorTransitionExecutor _pairedActorTransitionExecutor;

	Optional<SessionExit> _requestedSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_SCRIPT_ACTION_EXECUTOR_H
