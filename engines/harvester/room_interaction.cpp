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

#include "harvester/room_interaction.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"

namespace Harvester {

const char *const kExitCloseupPendingRoomChange = "__EXIT_CLOSEUP__";

RoomInteractionProcessor::RoomInteractionProcessor(HarvesterEngine &engine, Flow &flow,
		RoomSceneResources &scene, RoomPlayerState &playerState,
		Common::String &pendingRegionName, Common::String &pendingRoomChange,
		bool &pendingRoomChangeIsRoomName, bool &pendingRoomChangeUsesSavedRoomState,
		SaveRoomState &pendingRoomChangeSavedRoomState, bool canExitCloseupToParent,
		RoomInteractionCallbacks &callbacks)
	: _engine(engine), _flow(flow), _scene(scene), _playerState(playerState),
	  _pendingRegionName(pendingRegionName), _pendingRoomChange(pendingRoomChange),
	  _pendingRoomChangeIsRoomName(pendingRoomChangeIsRoomName),
	  _pendingRoomChangeUsesSavedRoomState(pendingRoomChangeUsesSavedRoomState),
	  _pendingRoomChangeSavedRoomState(pendingRoomChangeSavedRoomState),
	  _canExitCloseupToParent(canExitCloseupToParent), _callbacks(callbacks) {
}

Common::Error RoomInteractionProcessor::handleInteractionResult(const InteractionResult &interaction,
		bool &didTransition, const Common::String &usedItemName) {
	didTransition = false;

	_playerState.hasMoveTarget = false;
	_playerState.turnActive = false;
	_playerState.turnTargetFacing = -1;
	_pendingRegionName.clear();
	if (interaction.requestDemoEnding) {
		Common::Error endingError = _flow.runDemoEnding();
		if (endingError.getCode() != Common::kNoError)
			return endingError;
		didTransition = true;
		return Common::kNoError;
	}

	if (interaction.requestMainMenu) {
		_engine.stopMusic();
		_engine.stopSound();
		if (!interaction.deathFlicPath.empty()) {
			FstPlayer fstPlayer(_engine);
			if (!fstPlayer.play(interaction.deathFlicPath))
				return Common::kReadingFailed;
		}

		if (!interaction.deathFlicPath.empty())
			_flow.requestGameOverReturn();
		else
			_flow.requestMainMenuReturn();
		return Common::kNoError;
	}

	Common::String restoreMusicPath = _engine.getMusicPath();
	bool discChanged = false;
	if (interaction.cdChangeDisc > 0) {
		ResourceManager *resources = _engine.getResources();
		const int previousDisc = resources ? resources->getCurrentDisc() : 0;
		Common::Error cdPromptError = _callbacks.showCdChangePrompt(interaction.cdChangeDisc);
		if (cdPromptError.getCode() != Common::kNoError)
			return cdPromptError;
		discChanged = resources && previousDisc > 0 &&
			previousDisc != resources->getCurrentDisc();
	}
	if (!interaction.musicPath.empty()) {
		(void)_engine.playMusic(interaction.musicPath);
		restoreMusicPath = _engine.getMusicPath();
	}
	_flow.executeStartupAudioCommands(interaction.audioCommands);
	if (interaction.mutatedRuntimeState)
		_callbacks.syncGlobalTimerEntities(interaction.previousTimerRecords);

	auto queueImplicitRoomRestart = [&]() -> Common::Error {
		_pendingRoomChangeIsRoomName = false;
		_pendingRoomChangeUsesSavedRoomState = false;
		_pendingRoomChangeSavedRoomState.clear();
		if (!interaction.roomRestartTargetName.empty()) {
			_pendingRoomChange = interaction.roomRestartTargetName;
		} else if (discChanged && interaction.cdChangeDisc == 3) {
			// Native disc-3 prompt reloads re-enter room_setup through the startup target
			// buffer, which remains "START" during ordinary play and resolves to CD3 RECEPTION.
			_pendingRoomChange = "START";
		} else if (!_scene.state.entranceName.empty()) {
			_pendingRoomChange = _scene.state.entranceName;
		} else {
			_pendingRoomChange = _scene.state.roomName;
			_pendingRoomChangeIsRoomName = true;
		}
		didTransition = !_pendingRoomChange.empty();
		debugC(1, kDebugRoom,
			"Harvester: implicit room restart target='%s' explicit='%s' discChanged=%d cdChangeDisc=%d sceneEntrance='%s' sceneRoom='%s'",
			_pendingRoomChange.c_str(), interaction.roomRestartTargetName.c_str(),
			discChanged ? 1 : 0, interaction.cdChangeDisc,
			_scene.state.entranceName.c_str(), _scene.state.roomName.c_str());
		return didTransition ? Common::kNoError : Common::kReadingFailed;
	};
	auto queueSavedRoomRestart = [&](const SaveRoomState &savedState) -> Common::Error {
		_pendingRoomChange.clear();
		_pendingRoomChangeIsRoomName = false;
		_pendingRoomChangeUsesSavedRoomState = false;
		_pendingRoomChangeSavedRoomState.clear();
		_pendingRoomChange = !savedState.entranceName.empty()
			? savedState.entranceName
			: savedState.roomName;
		_pendingRoomChangeIsRoomName = savedState.entranceName.empty();
		_pendingRoomChangeUsesSavedRoomState = !_pendingRoomChange.empty();
		_pendingRoomChangeSavedRoomState = savedState;
		debugC(1, kDebugRoom,
			"Harvester: queued saved room restart target='%s' room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d music='%s'",
			_pendingRoomChange.c_str(), savedState.roomName.c_str(),
			savedState.entranceName.c_str(), savedState.playerX, savedState.playerY,
			savedState.playerZ, savedState.playerFacing, savedState.musicPath.c_str());
		didTransition = !_pendingRoomChange.empty();
		return didTransition ? Common::kNoError : Common::kReadingFailed;
	};
	if (interaction.requestRoomRestart)
		return queueImplicitRoomRestart();
	if (interaction.requestCloseupExit) {
		if (!_canExitCloseupToParent) {
			debugC(1, kDebugRoom,
				"Harvester: EXIT_CLOSEUP ignored for room='%s' without parent room loop",
				_scene.state.roomName.c_str());
			return Common::kNoError;
		}

		_pendingRoomChange = kExitCloseupPendingRoomChange;
		didTransition = true;
		return Common::kNoError;
	}

	StartupRoomTransitionKind roomTransition = interaction.roomTransition;
	if (roomTransition == kStartupRoomTransitionNone && !interaction.nextRoomName.empty())
		roomTransition = kStartupRoomTransitionCloseup;

	const bool needsTownScriptDiscReload =
		discChanged &&
		(interaction.cdChangeDisc == 1 || interaction.cdChangeDisc == 3) &&
		!interaction.requestRoomRestart &&
		interaction.nextRoomName.empty() &&
		!interaction.mutatedRuntimeState;
	if (needsTownScriptDiscReload)
		return queueImplicitRoomRestart();

	if (!interaction.nextRoomName.empty() &&
			roomTransition == kStartupRoomTransitionChangeRoom) {
		Common::String resolvedTransitionTarget = interaction.nextRoomName;
		Common::Error resolveError = _flow.resolveRoomTransitionTarget(
			interaction.nextRoomName, resolvedTransitionTarget);
		if (resolveError.getCode() != Common::kNoError)
			return resolveError;
		if (resolvedTransitionTarget.empty())
			return Common::kNoError;

		Common::Error exitError = _callbacks.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return exitError;

		// Native CHANGE_ROOM queues a room handoff for the live loop instead of nesting.
		_pendingRoomChange = Common::move(resolvedTransitionTarget);
		didTransition = true;
	} else if (!interaction.nextRoomName.empty()) {
		Common::Error exitError = _callbacks.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return exitError;

		SaveRoomState parentRoomState;
		const bool hasParentRoomState = _engine.hasCurrentSaveRoomState();
		if (hasParentRoomState)
			parentRoomState = _engine.getCurrentSaveRoomState();
		Common::Error roomError = _flow.runRoomLoop(interaction.nextRoomName);
		if (_flow.hasPendingMainMenuReturn())
			return Common::kNoError;
		if (_flow.takePendingCloseupParentRestart()) {
			if (hasParentRoomState && parentRoomState.valid)
				return queueSavedRoomRestart(parentRoomState);
			return queueImplicitRoomRestart();
		}
		if (_flow.hasPendingDebugRoomChange()) {
			didTransition = true;
			return Common::kNoError;
		}
		if (roomError.getCode() != Common::kReadingFailed &&
				roomError.getCode() != Common::kNoError) {
			return roomError;
		}

		if (!_callbacks.refreshCurrentScene(true))
			return Common::kReadingFailed;

		_flow.executeStartupAudioCommands(_scene.state.audioCommands);
		if (!restoreMusicPath.empty())
			(void)_engine.playMusic(restoreMusicPath);
		else
			_engine.stopMusic();
		didTransition = true;
	} else if (interaction.visualRuntimeStateChanged) {
		if (!_callbacks.applyCurrentRoomRuntimeMutationsInPlace(true) &&
				!_callbacks.refreshCurrentScene(true)) {
			return Common::kReadingFailed;
		}
	}

	if (didTransition)
		return Common::kNoError;

	if (interaction.requestPlayerDeath &&
			_engine.getScript() &&
			_engine.getScript()->getPlayerCurrentHitPoints() <= 0) {
		_callbacks.stopPlayerRegionInteraction();
		_callbacks.startPlayerDefeatSequence("script_kill_pc", Common::String(),
			interaction.playerDeathDamageType);
	}

	if (interaction.requestPlayerGotoXZ)
		_callbacks.applyPlayerGotoXZ(interaction.playerGotoX, interaction.playerGotoZ);

	if (!interaction.moveEntityToPlayerZName.empty()) {
		EntityManager *entityManager = _engine.getRuntimeEntities();
		Entity *entity = entityManager
			? entityManager->findSceneEntityByName(interaction.moveEntityToPlayerZName)
			: nullptr;
		if (entity && _playerState.entity) {
			entity->setPosition(entity->getX(), entity->getY(), _playerState.entity->getZ());
			entityManager->reinsertSceneEntity(entity);
			debugC(1, kDebugPlayer, "Harvester: MOVE_BM2PCZ entity='%s' z=%.2f",
				interaction.moveEntityToPlayerZName.c_str(), entity->getZ());
		}
	}

	if (interaction.lightingCommand != kStartupLightingCommandNone) {
		Common::Error lightingError = _callbacks.applyLightingCommand(interaction.lightingCommand);
		if (lightingError.getCode() != Common::kNoError)
			return lightingError;
	}

	if (!interaction.cutscenePath.empty()) {
		FstPlayer fstPlayer(_engine);
		if (!fstPlayer.play(interaction.cutscenePath))
			return Common::kReadingFailed;
	}

	if (!interaction.modalText.value.empty()) {
		Common::Error modalError = _callbacks.runModalShowText(interaction.modalText);
		if (modalError.getCode() != Common::kNoError)
			return modalError;
	}

	if (!interaction.dialogueNpcName.empty()) {
		Common::Error dialogueError = runScriptedDialogue(
			interaction.dialogueNpcName, usedItemName, interaction.dialogueContinuationTag,
			didTransition);
		if (dialogueError.getCode() != Common::kNoError)
			return dialogueError;
	}

	if (didTransition)
		return Common::kNoError;

	if (!interaction.continuationTag.empty()) {
		InteractionResult continuationInteraction;
		if (_engine.getScript()->executeActionTag(
				interaction.continuationTag, continuationInteraction, true,
				_scene.state.roomName)) {
			Common::Error interactionError =
				handleInteractionResult(continuationInteraction, didTransition, usedItemName);
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
		}
	}

	return Common::kNoError;
}

Common::Error RoomInteractionProcessor::runScriptedDialogue(const Common::String &npcName,
		const Common::String &usedItemName, const Common::String &continuationTag,
		bool &didTransition) {
	didTransition = false;

	const NpcRecord *dialogueNpc = _engine.getScript()->findRuntimeNpcRecord(npcName);
	if (dialogueNpc) {
		IndexedBitmap dialogueBackdrop;
		if (!_callbacks.captureDialogueBackdrop(dialogueBackdrop))
			return Common::kReadingFailed;

		Common::Error dialogueError = _flow.runRoomNpcDialogue(
			dialogueBackdrop, _scene.palette, _scene.targetPaletteBrightness, *dialogueNpc,
			usedItemName);
		if (dialogueError.getCode() != Common::kNoError)
			return dialogueError;
	} else {
		warning("Harvester: unresolved startup dialogue npc '%s' while processing room dialogue",
			npcName.c_str());
	}

	InteractionResult dialogueInteraction;
	bool abortRemainingCommandChain = false;
	if (_flow.takeQueuedDialogueInteraction(dialogueInteraction)) {
		abortRemainingCommandChain = dialogueInteraction.abortRemainingCommandChain;
		Common::Error interactionError =
			handleInteractionResult(dialogueInteraction, didTransition, usedItemName);
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		if (_flow.hasPendingMainMenuReturn())
			return Common::kNoError;
	}
	if (!didTransition && !abortRemainingCommandChain && !continuationTag.empty()) {
		InteractionResult continuationInteraction;
		if (_engine.getScript()->executeActionTag(
				continuationTag, continuationInteraction, true, _scene.state.roomName)) {
			Common::Error interactionError =
				handleInteractionResult(continuationInteraction, didTransition, usedItemName);
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
			if (_flow.hasPendingMainMenuReturn())
				return Common::kNoError;
		}
	}

	_flow.resetCursorAnimationSequence();
	_callbacks.resetIdleState();
	return Common::kNoError;
}

bool hasRoomEntryInteraction(const InteractionResult &interaction) {
	return !interaction.nextRoomName.empty() ||
		!interaction.musicPath.empty() ||
		!interaction.audioCommands.empty() ||
		interaction.cdChangeDisc > 0 ||
		!interaction.cutscenePath.empty() ||
		!interaction.deathFlicPath.empty() ||
		interaction.requestMainMenu ||
		interaction.requestDemoEnding ||
		interaction.requestCloseupExit ||
		interaction.requestRoomRestart ||
		interaction.requestPlayerDeath ||
		!interaction.moveEntityToPlayerZName.empty() ||
		interaction.requestPlayerGotoXZ ||
		interaction.lightingCommand != kStartupLightingCommandNone ||
		!interaction.modalText.value.empty() ||
		!interaction.dialogueNpcName.empty() ||
		!interaction.dialogueContinuationTag.empty() ||
		!interaction.continuationTag.empty() ||
		interaction.mutatedRuntimeState ||
		interaction.visualRuntimeStateChanged;
}

} // End of namespace Harvester
