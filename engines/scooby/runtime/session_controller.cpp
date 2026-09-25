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

#include "session_controller.h"

#include "common/algorithm.h"

#include "scooby/common/contracts.h"

namespace Scooby {

SessionController::SessionController(const ScoobyDooRom &rom, TileScene &scene, IndexedFrame &frame,
									 RaylibHost &host, FrameClock &clock, RuntimeState &state,
									 DeterministicRandom &random)
	: _rom(rom), _host(host), _state(state), _input(_host, _state), _roomTileStreamer(_rom, scene, _state),
	  _roomActorInitializer(_rom, _state), _roomObjectTilePatches(_rom, scene, _state),
	  _roomSprites(_rom, scene, _state),
	  _roomVBlank(_rom, _input, _roomObjectTilePatches, scene, _roomSprites, _state, _roomTileStreamer),
	  _presenter(scene, frame, _host, clock), _restartSequence(_state, _presenter), _graphicsWorkspace(),
	  _waitForRoomVerticalBlankFunctor(this, &SessionController::waitForRoomVerticalBlank),
	  _presentRoomFunctor(this, &SessionController::presentRoom),
	  _resetRoomStateForReloadFunctor(this, &SessionController::resetRoomStateForReload),
	  _reloadAndPresentRoomFunctor(this, &SessionController::reloadAndPresentRoom),
	  _restartApplicationFunctor(this, &SessionController::restartApplication),
	  _drawSelectedActionIconFunctor(&_actionMenu, &ActionMenuController::drawSelectedActionIcon),
	  _commitActionPromptUpdateFunctor(&_actionMenu, &ActionMenuController::commitActionPromptUpdate),
	  _refreshActionPromptsFunctor(&_actionMenu, &ActionMenuController::refreshActionPrompts),
	  _commitActionPromptTilesFunctor(&_actionMenu, &ActionMenuController::commitActionPromptTiles),
	  _updateRoomActorsAndInterfaceFunctor(&_roomActorUpdater, &RoomActorUpdater::updateRoomActorsAndInterface),
	  _refreshInteractionDisplayFunctor(&_interactions, &InteractionController::refreshInteractionDisplay),
	  _finishRoomStartupFunctor(&_roomInterfaceTransitions, &RoomInterfaceTransitionExecutor::finishRoomStartup),
	  _executeRoomScriptAction1DFunctor(&_roomInterfaceTransitions,
										&RoomInterfaceTransitionExecutor::executeRoomScriptAction1D),
	  _runRoomScriptInitializationCommandsFunctor(&_roomScripts,
												  &RoomScriptController::runRoomScriptInitializationCommands),
	  _runRoomScriptCommandsFunctor(&_roomScripts, &RoomScriptController::runRoomScriptCommands),
	  _runInteractionActionMenuFunctor(&_interactionActionMenu,
									   &InteractionActionMenuExecutor::runInteractionActionMenu),
	  _actionMenu(_rom, scene, _graphicsWorkspace, _state, _waitForRoomVerticalBlankFunctor),
	  _roomCollisionProbe(_rom, _state),
	  _roomActorUpdater(_rom, scene, _state, _roomCollisionProbe, _drawSelectedActionIconFunctor),
	  _actorAnimations(_rom, _state, _waitForRoomVerticalBlankFunctor),
	  _updateActorAnimationFramesFunctor(&_actorAnimations, &RoomActorAnimator::updateActorAnimationFrames),
	  _handleRoomVBlankFunctor(&_roomVBlank, &RoomVBlankHandler::handleRoomVBlank),
	  _scriptedRoomMovement(_rom, _state, _roomCollisionProbe, _updateActorAnimationFramesFunctor,
							_waitForRoomVerticalBlankFunctor),
	  _dialogue(_rom, scene, _state, _presenter, _updateActorAnimationFramesFunctor, _handleRoomVBlankFunctor),
	  _interactions(_rom, _state, _waitForRoomVerticalBlankFunctor, random, _actionMenu, _dialogue),
	  _processAutomaticInteractionsFunctor(&_interactions, &InteractionController::processAutomaticInteractions),
	  _chooseRandomInteractionAnimationFunctor(&_interactions,
											   &InteractionController::chooseRandomInteractionAnimation),
	  _roomInterfaceTransitions(_rom, scene, _graphicsWorkspace, _state, _updateActorAnimationFramesFunctor,
								_waitForRoomVerticalBlankFunctor),
	  _roomSceneLoader(_rom, scene, _state, _actionMenu, _roomActorInitializer, _roomInterfaceTransitions,
					   _roomTileStreamer),
	  _roomScripts(_rom, scene, _state, _roomCollisionProbe, _scriptedRoomMovement, random, _presenter,
				   _updateActorAnimationFramesFunctor, _handleRoomVBlankFunctor, _presentRoomFunctor,
				   _resetRoomStateForReloadFunctor, _reloadAndPresentRoomFunctor, _interactions,
				   _refreshInteractionDisplayFunctor, _finishRoomStartupFunctor,
				   _executeRoomScriptAction1DFunctor, _dialogue, _commitActionPromptUpdateFunctor,
				   _refreshActionPromptsFunctor, _drawSelectedActionIconFunctor, _roomTileStreamer,
				   _restartApplicationFunctor),
	  _interactionActionMenu(_rom, scene, _graphicsWorkspace, _state, _presenter,
							 _updateActorAnimationFramesFunctor, _handleRoomVBlankFunctor,
							 _chooseRandomInteractionAnimationFunctor,
							 _runRoomScriptInitializationCommandsFunctor, _runRoomScriptCommandsFunctor,
							 _dialogue, _commitActionPromptUpdateFunctor, _refreshActionPromptsFunctor),
	  _studioLogos(_rom, scene, _presenter, _host, _handleRoomVBlankFunctor),
	  _mainMenuPresentation(_rom, scene, _presenter), _mainMenuVBlank(_input, _mainMenuPresentation, random, _state),
	  _episodeOpening(_rom, scene, _presenter, _mainMenuVBlank, _state),
	  _mainMenu(_rom, _mainMenuPresentation, _mainMenuVBlank, _input, _state, _presenter, _restartSequence),
	  _ambientMovement(_rom, random, _state, _scriptedRoomMovement) {
	// Ghidra 0x00007F1E-region wiring: install the action-prompt tile commit and room-actor update owners
	// once their targets exist, matching the original delegate assignments performed immediately after
	// each target's construction.
	_roomVBlank.bindActionPromptTileCommit(_commitActionPromptTilesFunctor);
	_roomVBlank.bindRoomActorUpdate(_updateRoomActorsAndInterfaceFunctor);
	_dialogue.bindDismissalInteractions(_processAutomaticInteractionsFunctor,
										_chooseRandomInteractionAnimationFunctor);
	_interactions.bindInteractionActionMenu(_runInteractionActionMenuFunctor);
}

Optional<SessionExit> SessionController::runMainMenu() {
	switch (_mainMenu.runMainMenu(_handleRoomVBlankFunctor)) {
	case MainMenuExit::ContinueToRoom:
		return Optional<SessionExit>();
	case MainMenuExit::RestartRequested:
		return Optional<SessionExit>(SessionExit::RestartRequested);
	case MainMenuExit::HostClosed:
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	SDM_UNREACHABLE("Unhandled main-menu exit.");
	return Optional<SessionExit>(SessionExit::HostClosed); // unreachable, silences -Wreturn-type
}

bool SessionController::waitForRoomVerticalBlank() {
	_roomVBlank.handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}

Optional<SessionExit> SessionController::resetRoomStateForReload() {
	// Ghidra 0x00002138-0x0000213B: preserve all native registers. Managed locals retain the script cursor
	// and exclusive bound represented by A5/A6; the remaining register preservation is intrinsic to calls.
	int32 scriptStartOffset = _state.RoomScriptStartOffset;
	int32 scriptEndOffset = _state.RoomScriptEndOffset;

	// Ghidra 0x0000213C-0x00002167: while native interrupts are masked, reset controller input, clear
	// display bit three, load the selected room, then set display bits two and six before restoring
	// service.
	_state.ControllerOneInput = 0xFF;
	_state.DisplayFlags &= 0xF7;
	_roomSceneLoader.loadRoomScene();
	_state.DisplayFlags |= 0x04;
	_state.DisplayFlags |= 0x40;

	// Ghidra 0x00002168-0x00002177: progress byte-zero bit one independently skips prompt refresh.
	if ((_state.ProgressStateBytes[0] & 0x02) == 0) {
		_actionMenu.refreshActionPrompts();
	}

	// Ghidra 0x00002178-0x0000217D: stage actor animation and tile state exactly once before waiting.
	_actorAnimations.updateActorAnimationFrames();

	// Ghidra 0x0000217E-0x0000218B: wait at least one retrace, then repeat only while any actor tile
	// publication remains pending. The installed room callback supplies the original asynchronous work.
	do {
		if (!waitForRoomVerticalBlank()) {
			return Optional<SessionExit>(SessionExit::HostClosed);
		}
	} while (_state.ActorTileUploadPendingFlags != 0);

	// Ghidra 0x0000218C-0x0000218F: hand the loaded room to its exit-script owner. A managed non-returning
	// descendant bypasses the native restoration range just as its original target did.
	Optional<SessionExit> requestedExit = _roomScripts.runRoomExitScript();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	// Ghidra 0x00002190-0x00002195: restore every preserved register, including A5/A6, then return.
	_state.RoomScriptStartOffset = scriptStartOffset;
	_state.RoomScriptEndOffset = scriptEndOffset;
	return Optional<SessionExit>();
}

Optional<SessionExit> SessionController::reloadAndPresentRoom() {
	// Ghidra 0x00002196-0x00002199: reload through the separate original owner, then reproduce the native
	// fallthrough into PresentRoom with an explicit managed call. Non-returning handovers bypass
	// fallthrough.
	Optional<SessionExit> resetExit = resetRoomStateForReload();
	if (resetExit.hasValue()) {
		return resetExit;
	}

	return presentRoom();
}

Optional<SessionExit> SessionController::presentRoom() {
	// Ghidra 0x0000219A-0x0000219D: preserve all native registers. Managed locals retain the script cursor
	// and exclusive bound represented by A5/A6; the remaining register preservation is intrinsic to calls.
	int32 scriptStartOffset = _state.RoomScriptStartOffset;
	int32 scriptEndOffset = _state.RoomScriptEndOffset;

	// Ghidra 0x0000219E-0x000021AB: request the complete interaction refresh, then invoke its distinct
	// owner.
	_state.InteractionFlags |= kInteractionRefreshPendingMask;
	_interactions.refreshInteractionDisplay();

	// Ghidra 0x000021AC-0x000021AF: preserve one room VBlank before the palette transition. Host closure
	// is a managed non-returning handover from the native wait boundary.
	if (!waitForRoomVerticalBlank()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	// Ghidra 0x000021B0-0x000021C9: g_awSharedPalette and its contiguous room half form the complete
	// 64-color target. Gate normal room updates while FadePaletteIn services each native retrace.
	_state.DisplayFlags &= static_cast<uint8>(~kRoomUpdateEnabledMask);
	if (!_presenter.fadePaletteIn(MakeSpan(_state.TargetPalette), &_handleRoomVBlankFunctor)) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	_state.DisplayFlags |= kRoomUpdateEnabledMask;

	// Ghidra 0x000021CA-0x0000220F: copy every current interaction/display field to its previous slot in
	// exact order, including the second current-interaction write before the final icon and secondary
	// words.
	_state.PreviousInteraction = _state.CurrentInteraction;
	_state.PreviousDisplayFlags = _state.DisplayFlags;
	_state.PreviousInteractionFlags = _state.InteractionFlags;
	_state.PreviousRoomNameTextOffset = _state.RoomNameTextOffset;
	_state.PreviousInteraction = _state.CurrentInteraction;
	_state.PreviousActionIcon = _state.ActiveActionIcon;
	_state.PreviousSecondaryInteraction = _state.SecondaryInteraction;

	// Ghidra 0x00002210-0x00002213: preserve the separate room-entry-script call. A managed non-returning
	// descendant bypasses the native restoration range just as its original target did.
	Optional<SessionExit> requestedExit = _roomScripts.runRoomEntryScript();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	// Ghidra 0x00002214-0x00002219: restore every preserved register, including A5/A6, then return.
	_state.RoomScriptStartOffset = scriptStartOffset;
	_state.RoomScriptEndOffset = scriptEndOffset;
	return Optional<SessionExit>();
}

bool SessionController::loadRoomObjectTable() {
	// Ghidra 0x00007F66-0x00007F91: select and publish the immutable descriptor, then reset typed tables.
	_state.ActiveEpisodeDescriptorOffset = static_cast<int32>(
		_rom.readUInt32(
			kEpisodeDescriptorTableOffset + _state.EpisodeIndex * static_cast<int>(sizeof(uint32))));
	int descriptorOffset = _state.ActiveEpisodeDescriptorOffset;
	int sourceOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kDescriptorObjectStartField));
	int sourceEndOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kDescriptorObjectEndField));
	Common::Array<RoomObject> roomObjects;
	_state.InventoryObjectIndices.clear();

	// Ghidra 0x00007F92-0x00007FEF: decode every variable-size source record and inventory index.
	while (sourceOffset != sourceEndOffset) {
		uint16 actionWord = _rom.readUInt16(sourceOffset + 10);
		uint16 flagsWord = _rom.readUInt16(sourceOffset + 12);
		bool hasFixedPosition = (flagsWord & kHasFixedPositionMask) != 0;

		RoomObject roomObject;
		roomObject.TilePatchIndex = _rom.readInt16(sourceOffset);
		roomObject.ShapeIndex = _rom.readInt16(sourceOffset + 2);
		roomObject.InventoryGraphicIndex = _rom.readInt16(sourceOffset + 4);
		roomObject.RoomId = _rom.readInt16(sourceOffset + 6);
		roomObject.PositionIndex = _rom.readInt16(sourceOffset + 8);
		roomObject.FixedX = hasFixedPosition
								? _rom.readInt16(sourceOffset + kSourceRecordSize)
								: static_cast<int16>(0);
		roomObject.FixedY = hasFixedPosition
								? _rom.readInt16(
									  sourceOffset + kSourceRecordSize + static_cast<int>(sizeof(int16)))
								: static_cast<int16>(0);
		roomObject.ActionIcon = static_cast<int8>(actionWord >> 8);
		roomObject.ActionArgument = static_cast<uint8>(actionWord);
		roomObject.Flags = static_cast<uint8>(flagsWord >> 8);
		roomObject.FixedPositionIndex = -1;

		roomObjects.push_back(roomObject);
		if (roomObject.RoomId == kInventoryRoomId) {
			_state.InventoryObjectIndices.push_back(static_cast<int32>(roomObjects.size()) - 1);
		}

		sourceOffset += hasFixedPosition ? kExtendedSourceRecordSize : kSourceRecordSize;
	}

	_state.RoomObjects = roomObjects;

	// Ghidra 0x00007FF0-0x00008009: publish the initial room, resume-audio command, and A5/A6 script
	// window.
	int scriptHeaderOffset = static_cast<int>(_rom.readUInt32(
		descriptorOffset + kDescriptorInitialRoomScriptField));
	_state.RoomId = _rom.readInt16(scriptHeaderOffset);
	_state.ResumeAudioCommand = _rom.readInt16(scriptHeaderOffset + static_cast<int>(sizeof(int16)));
	_state.RoomScriptStartOffset = scriptHeaderOffset + kRoomScriptHeaderSize;
	_state.RoomScriptEndOffset = _state.RoomScriptStartOffset + _rom.readInt16(scriptHeaderOffset + 4);

	// Ghidra 0x0000800A-0x00008019: run the episode opening, then preserve its explicit audio-stop
	// boundary.
	if (!_episodeOpening.runEpisodeOpeningSequence()) {
		return false;
	}

	stopAudioPlayback(0x1E);
	return true;
}

void SessionController::initializeRoomRuntimeState() {
	// Ghidra 0x0000801A-0x00008039: bind the first two actor slots to the shape-one and shape-two
	// animation descriptors, then reset their selected descriptor-relative frame offsets.
	_state.ActorAnimationDescriptorOffsets[0] = 0x32700;
	_state.ActorAnimationDescriptorOffsets[1] = 0x642D0;
	_state.ActorSelectedFrameOffsets[0] = 0;
	_state.ActorSelectedFrameOffsets[1] = 0;

	// Ghidra 0x0000803A-0x0000807B: enable and restart the first two compact-frame actors, clear all
	// pending frame, movement, priority, and visibility state, and omit write-only byte 0xFF0AC8.
	_state.ActiveActorFlags = 0x03;
	_state.ActorAnimationRestartFlags = 0x03;
	_state.ActorCompactFrameFlags = 0x03;
	_state.ActorTileUploadPendingFlags = 0;
	_state.ActorMovementFlags = 0;
	_state.ActorCompactConversionPendingFlags = 0;
	_state.ActorHighPriorityFlags = 0;
	_state.ActorVisibilityBlockFlags = 0;
	_state.ActorPriorityUpdateBlockFlags = 0;

	// Ghidra 0x0000807C-0x000080C7: select animation offset four for slots zero and one, zero the other
	// four animation offsets, and clear every actor's authored room-position index.
	Common::fill(_state.ActorAnimationOffsets.begin(), _state.ActorAnimationOffsets.end(), 0);
	_state.ActorAnimationOffsets[0] = 4;
	_state.ActorAnimationOffsets[1] = 4;
	Common::fill(_state.ActorPositionIndices.begin(), _state.ActorPositionIndices.end(), 0);

	// Ghidra 0x000080C8-0x00008157: make every animation delay immediately ready and replace both
	// six-pointer frame histories' raw 0xFF000000 invalid markers with managed null entries.
	Common::fill(_state.ActorAnimationDelays.begin(), _state.ActorAnimationDelays.end(), static_cast<int16>(-1));
	Common::fill(_state.ActorCurrentFrameDataOffsets.begin(), _state.ActorCurrentFrameDataOffsets.end(), Optional<int32>());
	Common::fill(_state.ActorPendingFrameDataOffsets.begin(), _state.ActorPendingFrameDataOffsets.end(), Optional<int32>());

	// Ghidra 0x00008158-0x0000816F: clear the first two actors' cached horizontal and vertical
	// interpolation values; later actor slots still carry the process-zeroed values consumed at creation.
	_state.ActorPreviousHorizontalScales[0] = 0;
	_state.ActorPreviousHorizontalScales[1] = 0;
	_state.ActorPreviousVerticalScales[0] = 0;
	_state.ActorPreviousVerticalScales[1] = 0;

	// Ghidra 0x00008170-0x0000818F: visit every decoded object. The fixed-position-bit branch either
	// executes a literal NOP or converges immediately, so the complete loop has no managed state effect.

	// Ghidra 0x00008190-0x00008194: advance the initialized slots to their first authored frames.
	_actorAnimations.updateActorAnimationFrames();
}

SessionExit SessionController::runGame() {
	if (_state._saveGameData.shouldLoadGameData) {
		_state.EpisodeIndex = _state._saveGameData.episodeIdx;
		Common::copy(_state._saveGameData.data.begin(), _state._saveGameData.data.begin() + 29,
				  _state.ProgressStateBytes.begin());
		_state.InitialRoomId = _state._saveGameData.roomId;
		_state.InitializationFlags |= 0x01;
		_state.InitializationFlags |= 4; // resume game
		_state.InitialRoomPositionCoordinateOffset = 0;

		_state.VideoFlags &= 0xFD;
		_state._saveGameData.shouldLoadGameData = false;
	} else {
		if (!_studioLogos.run()) {
			return SessionExit::HostClosed;
		}

		Optional<SessionExit> initialMenuExit = runMainMenu();
		if (initialMenuExit.hasValue()) {
			return initialMenuExit.value();
		}
	}

	initializeAudioDriver();
	_state.resetRuntimeState(_rom);
	if (!loadRoomObjectTable()) {
		return SessionExit::HostClosed;
	}

	initializeRoomRuntimeState();
	_state.DisplayFlags |= kInteractionPendingMask;
	_state.RoomScriptFlags &= 0xFC;
	Optional<SessionExit> initialScriptExit = _roomScripts.runRoomScriptCommands();
	if (initialScriptExit.hasValue()) {
		return initialScriptExit.value();
	}

	_state.RoomScriptFlags &= 0xFC;
	_state.advanceRoomScriptBlock(_rom);
	Optional<SessionExit> entryScriptExit = _roomScripts.runRoomScriptCommands();
	if (entryScriptExit.hasValue()) {
		return entryScriptExit.value();
	}

	_state.DisplayFlags &= static_cast<uint8>(~kInteractionPendingMask);
	if ((_state.InitializationFlags & 0x04) != 0) {
		_state.InitialRoomPositionCoordinateOffset = 0;
		_state.RoomId = _state.InitialRoomId;
		Optional<SessionExit> resetExit = resetRoomStateForReload();
		if (resetExit.hasValue()) {
			return resetExit.value();
		}
	}

	Optional<SessionExit> presentationExit = presentRoom();
	if (presentationExit.hasValue()) {
		return presentationExit.value();
	}

	_roomInterfaceTransitions.finishRoomStartup();

	while (!_host.shouldClose()) {
		if (_state._saveGameData.shouldLoadGameData) {
			return SessionExit::RestartRequested;
		}

		// Native VBLANK can preempt every pass through Ghidra 0x00000D38-0x000013B9. The managed loop
		// services that installed callback and presents one clocked frame before consuming its state.
		if (!waitForRoomVerticalBlank()) {
			return SessionExit::HostClosed;
		}

		Optional<SessionExit> automaticInteractionExit = _interactions.processAutomaticInteractions();
		if (automaticInteractionExit.hasValue()) {
			return automaticInteractionExit.value();
		}

		if ((_state.DisplayFlags & kInteractionPendingMask) != 0) {
			Optional<SessionExit> selectedInteractionExit = _interactions.runSelectedInteraction();
			if (selectedInteractionExit.hasValue()) {
				return selectedInteractionExit.value();
			}

			_state.SecondaryInteraction = 0;
			if (!waitForRoomVerticalBlank()) {
				return SessionExit::HostClosed;
			}

			_state.DisplayFlags &= static_cast<uint8>(~kInteractionPendingMask);
			if (!waitForRoomVerticalBlank()) {
				return SessionExit::HostClosed;
			}

			continue;
		}

		if ((_state.ControllerOneInput & 0x80) == 0 && (_state.TransitionFlags & 0x10) == 0) {
			_state.DisplayFlags &= 0xBF;
			if (!_presenter.fadePaletteOut(&_handleRoomVBlankFunctor)) {
				return SessionExit::HostClosed;
			}

			Optional<SessionExit> menuExit = runMainMenu();
			if (menuExit.hasValue()) {
				return menuExit.value();
			}

			Optional<SessionExit> reloadExit = reloadAndPresentRoom();
			if (reloadExit.hasValue()) {
				return reloadExit.value();
			}

			_state.DisplayFlags &= static_cast<uint8>(~kInteractionPendingMask);
			if (!waitForRoomVerticalBlank()) {
				return SessionExit::HostClosed;
			}

			continue;
		}

		_interactions.refreshInteractionDisplay();
		_interactions.snapshotInteractionState();
		if (!_ambientMovement.tryStartRandomMove()) {
			return SessionExit::HostClosed;
		}

		if ((_state.DisplayFlags & kInteractionDisplayMask) != 0) {
			if (!_interactions.updateCursorInteraction()) {
				return SessionExit::HostClosed;
			}
		} else {
			bool restartRoomLoop = false;
			Optional<SessionExit> automaticUpdateExit = _interactions.updateAutomaticInteraction(
				restartRoomLoop);
			if (automaticUpdateExit.hasValue()) {
				return automaticUpdateExit.value();
			}

			if (restartRoomLoop) {
				continue;
			}
		}

		_actorAnimations.updateActorAnimationFrames();
	}

	return SessionExit::HostClosed;
}
} // namespace Scooby