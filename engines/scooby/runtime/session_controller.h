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

#ifndef SCOOBY_SESSION_CONTROLLER_H
#define SCOOBY_SESSION_CONTROLLER_H

#include "common/scummsys.h"

#include "episode_opening_sequence.h"
#include "runtime_state.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/shared_graphics_workspace.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/input/controller_input.h"
#include "scooby/interactions/action_menu_controller.h"
#include "scooby/interactions/dialogue_controller.h"
#include "scooby/interactions/interaction_action_menu_executor.h"
#include "scooby/interactions/interaction_controller.h"
#include "scooby/main_menu/main_menu_controller.h"
#include "scooby/main_menu/main_menu_presentation.h"
#include "scooby/main_menu/vblank/main_menu_vblank_handler.h"
#include "scooby/movement/ambient_movement_controller.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/raylib_host.h"
#include "scooby/rendering/frame_clock.h"
#include "scooby/rooms/room_actor_animator.h"
#include "scooby/rooms/room_actor_initializer.h"
#include "scooby/rooms/room_actor_updater.h"
#include "scooby/rooms/room_collision_probe.h"
#include "scooby/rooms/room_interface_transition_executor.h"
#include "scooby/rooms/room_object_tile_patch_renderer.h"
#include "scooby/rooms/room_scene_loader.h"
#include "scooby/rooms/room_script_controller.h"
#include "scooby/rooms/room_sprite_renderer.h"
#include "scooby/rooms/room_tile_streamer.h"
#include "scooby/rooms/vblank/room_vblank_handler.h"
#include "scooby/startup/restart_sequence.h"
#include "scooby/startup/studio_logo_sequence.h"
#include "session_exit.h"

// Owns the C++ recovery of Ghidra RunAdventure at 0x00000BF4 and coordinates its domains.

namespace Scooby {

class SessionController {
public:
	// Binds the recovered runtime state to the verified ROM and the existing video presentation boundary.
	// rom: verified cartridge address space.
	// scene: logical tile scene populated from decoded ROM assets.
	// frame: shared logical output frame.
	// host: raylib window and input owner.
	// clock: continuous retrace cadence carried forward from the startup sequence.
	// state: runtime state initialized before the startup presentations.
	// random: deterministic random state initialized and warmed before the startup presentations.
	SessionController(const ScoobyDooRom &rom, TileScene &scene,
					  IndexedFrame &frame,
					  RaylibHost &host, FrameClock &clock, RuntimeState &state,
					  DeterministicRandom &random);

	// Reimplements Ghidra RunAdventure at 0x00000BF4 from entry through its perpetual frame loop. Audio
	// remains an explicitly deferred nonthrowing boundary.
	// Returns the reset request or host-close condition that ended the recovered control loop.
	//
	// Ghidra: RunAdventure (0x00000BF4). The installed room callback at 0x0000A65C remains active
	// throughout the steady loop at 0x00000D38-0x000013B9. Managed execution runs it before each clocked
	// room frame rather than introducing a concurrent timer. Title-menu room continuation resumes the
	// original post-menu path in this order: ResetRuntimeState, LoadRoomObjectTable,
	// InitializeRoomRuntimeState, two room-script passes, room presentation, and the perpetual update
	// loop. Runtime reset deliberately preserves g_wEpisodeIndex at 0xFF06AA, allowing room-object
	// loading to consume the episode selected by the shared password prompt.
	SessionExit runGame();

private:
	static const int kDescriptorInitialRoomScriptField = 0x30;
	static const int kDescriptorObjectEndField = 0x08;
	static const int kDescriptorObjectStartField = 0x04;
	static const int kEpisodeDescriptorTableOffset = 0x31AF2;
	static const int kExtendedSourceRecordSize = 18;
	static const uint16 kHasFixedPositionMask = 0x0200;
	static const int16 kInventoryRoomId = 1;
	static const int kRoomScriptHeaderSize = 6;
	static const int kSourceRecordSize = 14;
	static const uint8 kInteractionDisplayMask = 0x08;
	static const uint8 kInteractionPendingMask = 0x80;
	static const uint8 kInteractionRefreshPendingMask = 0x02;
	static const uint8 kRoomUpdateEnabledMask = 0x40;

	Optional<SessionExit> runMainMenu();
	bool waitForRoomVerticalBlank();

	// Loads the selected room state, publishes pending actor graphics, and runs its exit script.
	// Returns a host or nested-script exit request, otherwise no value.
	//
	// Ghidra: resetRoomStateForReload (0x00002138). Managed callback execution is non-reentrant, so the
	// interrupt-masked load section retains its exact state and call ordering without modeling the 68000
	// status register. Typed script offsets preserve native A5/A6 across the nested exit script. The
	// host-close result is the managed extension of the native wait.
	Optional<SessionExit> resetRoomStateForReload();

	// Reloads the selected room before entering its complete presentation sequence.
	// Returns a host or nested-script exit request, otherwise no value.
	//
	// Ghidra: reloadAndPresentRoom (0x00002196). The complete body is 0x00002196-0x00002199; native
	// control falls through into the distinct PresentRoom entry at 0x0000219A after the reset call
	// returns.
	Optional<SessionExit> reloadAndPresentRoom();

	// Refreshes, fades in, and snapshots a loaded room before running its entry script.
	// Returns a host or nested-script exit request, otherwise no value.
	//
	// Ghidra: presentRoom (0x0000219A). The complete body is 0x0000219A-0x00002219. The managed retrace
	// callback replaces native interrupt progress during both the leading wait and the eight-step fade.
	// Snapshot state retains the duplicate current-interaction write after the following three
	// current-field copies in the original body. Typed script offsets preserve native A5/A6 across the
	// nested entry script.
	Optional<SessionExit> presentRoom();

	// Binds RestartSequence::run to the recovered RoomScriptActionExecutor restart-application boundary.
	// beforeRetrace: optional callback serviced during the restart-confirmation retrace.
	bool restartApplication(const Common::Functor0<void> *beforeRetrace) {
		return _restartSequence.run(beforeRetrace);
	}

	// Loads the selected episode's mutable object state, script window, and opening handover.
	// Returns false when the host closes during the episode opening.
	//
	// Ghidra: loadRoomObjectTable (0x00007F66-0x00008019). Episode-zero source records occupy
	// 0x000FCA86-0x000FD4AF and produce 181 objects; episode-one records occupy 0x001505A8-0x00150E95
	// and produce 155. Each record has seven big-endian words plus optional fixed X/Y when flags mask
	// 0x0200 is set, and becomes one 26-byte logical runtime record. Descriptor field +0x30 selects
	// initial script headers 0x0014F5DC or 0x001BAD9C; the episode opening runs before the ordered
	// silent stop-audio handover.
	bool loadRoomObjectTable();

	// Defers Ghidra StopAudioPlayback at 0x00009776 without blocking room startup.
	// command: original audio command supplied in D0.
	static void stopAudioPlayback(int command) { (void)command; }

	// Initializes the six actor animation slots and publishes their first room frame.
	//
	// Ghidra: initializeRoomRuntimeState (0x0000801A).
	void initializeRoomRuntimeState();

	// Defers Ghidra InitializeAudioDriver at 0x001F6F52 without blocking recovered control flow.
	void initializeAudioDriver() {
	}

	const ScoobyDooRom &_rom;
	RaylibHost &_host;
	RuntimeState &_state;
	ControllerInput _input;
	RoomTileStreamer _roomTileStreamer;
	RoomActorInitializer _roomActorInitializer;
	RoomObjectTilePatchRenderer _roomObjectTilePatches;
	RoomSpriteRenderer _roomSprites;
	RoomVBlankHandler _roomVBlank;
	FramePresenter _presenter;
	RestartSequence _restartSequence;
	SharedGraphicsWorkspace _graphicsWorkspace;
	Common::Functor0Mem<bool, SessionController> _waitForRoomVerticalBlankFunctor;
	Common::Functor0Mem<Optional<SessionExit>, SessionController> _presentRoomFunctor;
	Common::Functor0Mem<Optional<SessionExit>, SessionController> _resetRoomStateForReloadFunctor;
	Common::Functor0Mem<Optional<SessionExit>, SessionController> _reloadAndPresentRoomFunctor;
	Common::Functor1Mem<const Common::Functor0<void> *, bool, SessionController> _restartApplicationFunctor;
	Common::Functor1Mem<int, void, ActionMenuController> _drawSelectedActionIconFunctor;
	Common::Functor0Mem<void, ActionMenuController> _commitActionPromptUpdateFunctor;
	Common::Functor0Mem<void, ActionMenuController> _refreshActionPromptsFunctor;
	Common::Functor0Mem<void, ActionMenuController> _commitActionPromptTilesFunctor;
	Common::Functor0Mem<void, RoomActorUpdater> _updateRoomActorsAndInterfaceFunctor;
	Common::Functor0Mem<void, InteractionController> _refreshInteractionDisplayFunctor;
	Common::Functor0Mem<void, RoomInterfaceTransitionExecutor> _finishRoomStartupFunctor;
	Common::Functor0Mem<void, RoomInterfaceTransitionExecutor> _executeRoomScriptAction1DFunctor;
	Common::Functor0Mem<Optional<SessionExit>, RoomScriptController> _runRoomScriptInitializationCommandsFunctor;
	Common::Functor0Mem<Optional<SessionExit>, RoomScriptController> _runRoomScriptCommandsFunctor;
	Common::Functor0Mem<Optional<SessionExit>, InteractionActionMenuExecutor> _runInteractionActionMenuFunctor;
	ActionMenuController _actionMenu;
	RoomCollisionProbe _roomCollisionProbe;
	RoomActorUpdater _roomActorUpdater;
	RoomActorAnimator _actorAnimations;
	Common::Functor0Mem<void, RoomActorAnimator> _updateActorAnimationFramesFunctor;
	Common::Functor0Mem<void, RoomVBlankHandler> _handleRoomVBlankFunctor;
	ScriptedRoomMovementExecutor _scriptedRoomMovement;
	DialogueController _dialogue;
	InteractionController _interactions;
	Common::Functor0Mem<Optional<SessionExit>, InteractionController> _processAutomaticInteractionsFunctor;
	Common::Functor0Mem<void, InteractionController> _chooseRandomInteractionAnimationFunctor;
	RoomInterfaceTransitionExecutor _roomInterfaceTransitions;
	RoomSceneLoader _roomSceneLoader;
	RoomScriptController _roomScripts;
	InteractionActionMenuExecutor _interactionActionMenu;
	StudioLogoSequence _studioLogos;
	MainMenuPresentation _mainMenuPresentation;
	MainMenuVBlankHandler _mainMenuVBlank;
	EpisodeOpeningSequence _episodeOpening;
	MainMenuController _mainMenu;
	AmbientMovementController _ambientMovement;
};
} // namespace Scooby

#endif // SCOOBY_SESSION_CONTROLLER_H
