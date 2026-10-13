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

#include "harvester/interaction.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player.h"
#include "harvester/scene_entities.h"
#include "harvester/resources.h"
#include "harvester/entity.h"

namespace Harvester {

const char *const kExitCloseupPendingRoomChange = "__EXIT_CLOSEUP__";

static const char *const kInventoryOwnerName = "INVENTORY";

static Common::String resolveCarriedObjectSpritePath(const ObjectRecord &object) {
	if (!object.altSpritePath.empty())
		return object.altSpritePath;
	return object.spritePath;
}

static bool shouldDispatchPickupActionOnCarryStart(const ObjectRecord &object) {
	if (object.actionTag.empty())
		return false;

	return !object.objectName.equalsIgnoreCase("SANDWICH") &&
		!object.objectName.equalsIgnoreCase("SANDWICH2") &&
		!object.objectName.equalsIgnoreCase("SYRINGE");
}

static bool shouldDispatchPickupActionOnDirectInventoryTransfer(const ObjectRecord &object) {
	if (object.actionTag.empty())
		return false;

	return !object.objectName.equalsIgnoreCase("ST_ASPRIN") &&
		!object.objectName.equalsIgnoreCase("ST_COUGHM") &&
		!object.objectName.equalsIgnoreCase("ST_VITAMN");
}

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	bitmap = IndexedBitmap();
	return decodeBitmap(data, bitmap);
}

ItemInteractions::ItemInteractions(HarvesterEngine &engine, SceneState &scene,
		SceneEntities &sceneRuntime, InventorySystem &inventory, const Common::String &pendingRoomChange)
	: _engine(engine), _scene(scene), _sceneRuntime(sceneRuntime), _inventory(inventory),
	  _pendingRoomChange(pendingRoomChange) {
}

bool ItemInteractions::hasCarriedRoomItem() const {
	return !_carriedRoomItemName.empty();
}

void ItemInteractions::clearCarriedRoomItem() {
	_carriedRoomItemName.clear();
	_carriedRoomItemLabel.clear();
	_carriedRoomItemBitmap = IndexedBitmap();
}

void ItemInteractions::hideSceneObject(const Common::String &objectName, const Common::String *ownerOrRoom) {
	ObjectRecord *sceneObject = findSceneObjectByName(_scene.sceneObjects, objectName);
	if (!sceneObject)
		return;

	sceneObject->visible = false;
	sceneObject->runtimeVisible = false;
	if (ownerOrRoom)
		sceneObject->currentOwnerOrRoom = *ownerOrRoom;
}

bool ItemInteractions::stowCarriedRoomItemToInventory() {
	if (!hasCarriedRoomItem())
		return true;

	_engine.getScript()->addRuntimeObjectToInventory(_carriedRoomItemName);
	clearCarriedRoomItem();
	return _inventory.refresh();
}

bool ItemInteractions::shouldCancelDeniedPickup(const InteractionResult &interaction, bool didTransition) const {
	return !didTransition &&
		!interaction.mutatedRuntimeState &&
		!interaction.requestPlayerGotoXZ &&
		!interaction.requestPlayerGotoZ &&
		interaction.lightingCommand == kStartupLightingCommandNone &&
		!interaction.requestMainMenu &&
		!interaction.requestDemoEnding &&
		interaction.dialogueNpcName.empty() &&
		interaction.dialogueContinuationTag.empty() &&
		interaction.continuationTag.empty() &&
		interaction.nextRoomName.empty() &&
		interaction.cutscenePath.empty() &&
		!interaction.modalText.value.empty();
}

Common::Error ItemInteractions::restoreDeniedPickup(Script &script, const ObjectRecord &savedObject, bool clearCarryState,
		InteractionCallbacks &callbacks, bool &needsRedraw) {
	if (!script.syncRuntimeObjectRecord(savedObject))
		return Common::kReadingFailed;
	if (clearCarryState)
		clearCarriedRoomItem();

	if (!callbacks.applyCurrentRoomRuntimeMutationsInPlace(true) && !callbacks.refreshCurrentScene(true))
		return Common::kReadingFailed;
	if (!_inventory.refresh())
		return Common::kReadingFailed;
	needsRedraw = true;
	return Common::kNoError;
}

Common::Error ItemInteractions::moveRoomItemDirectlyToInventory(const ObjectRecord &object,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw) {
	Script *script = _engine.getScript();
	if (!script)
		return Common::kReadingFailed;

	const ObjectRecord savedObject = object;

	script->addRuntimeObjectToInventory(object.objectName);
	const Common::String inventoryOwner(kInventoryOwnerName);
	hideSceneObject(object.objectName, &inventoryOwner);
	_sceneRuntime.removeSceneEntityByName(object.objectName);

	bool didTransition = false;
	if (shouldDispatchPickupActionOnDirectInventoryTransfer(object)) {
		InteractionResult pickupInteraction;
		if (script->executeActionTag(
				object.actionTag, pickupInteraction, true, _scene.state.roomName)) {
			Common::Error interactionError =
				interactionProcessor.handleInteractionResult(
					pickupInteraction, didTransition, Common::String());
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
			if (interactionProcessor.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
				debugC(1, kDebugRoom,
					"Harvester: cancelling denied direct pickup object='%s' action='%s' text='%s'",
					savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
					pickupInteraction.modalText.value.c_str());
				return restoreDeniedPickup(*script, savedObject, false, callbacks, needsRedraw);
			}
		}
	}

	if (!_inventory.refresh())
		return Common::kReadingFailed;
	needsRedraw = true;
	return Common::kNoError;
}

Common::Error ItemInteractions::beginRoomItemCarry(const ObjectRecord &object,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw) {
	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	if (!script || !resources)
		return Common::kReadingFailed;

	const ObjectRecord savedObject = object;

	clearCarriedRoomItem();
	_carriedRoomItemName = object.objectName;
	_carriedRoomItemLabel = script->resolveObjectLabel(object);
	const Common::String spritePath = resolveCarriedObjectSpritePath(object);
	if (!spritePath.empty())
		loadBitmapResource(*resources, spritePath, _carriedRoomItemBitmap);

	script->setRuntimeObjectVisible(object.currentOwnerOrRoom, object.objectName, false);
	hideSceneObject(object.objectName, nullptr);
	_sceneRuntime.removeSceneEntityByName(object.objectName);

	bool didTransition = false;
	if (shouldDispatchPickupActionOnCarryStart(object)) {
		InteractionResult pickupInteraction;
		if (script->executeActionTag(
				object.actionTag, pickupInteraction, true, _scene.state.roomName)) {
			Common::Error interactionError =
				interactionProcessor.handleInteractionResult(
					pickupInteraction, didTransition, Common::String());
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
			if (interactionProcessor.hasPendingMainMenuReturn())
				return Common::kNoError;
			if (shouldCancelDeniedPickup(pickupInteraction, didTransition)) {
				debugC(1, kDebugRoom,
					"Harvester: cancelling denied carried pickup object='%s' action='%s' text='%s'",
					savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
					pickupInteraction.modalText.value.c_str());
				return restoreDeniedPickup(*script, savedObject, true, callbacks, needsRedraw);
			}
			if (!didTransition &&
					!pickupInteraction.dialogueNpcName.empty() &&
					!script->isObjectInInventory(savedObject.objectName)) {
				debugC(1, kDebugRoom,
					"Harvester: restoring carried pickup after dialogue object='%s' action='%s' npc='%s'",
					savedObject.objectName.c_str(), savedObject.actionTag.c_str(),
					pickupInteraction.dialogueNpcName.c_str());
				return restoreDeniedPickup(*script, savedObject, true, callbacks, needsRedraw);
			}
		}
	}

	if (!_pendingRoomChange.empty()) {
		if (!stowCarriedRoomItemToInventory())
			return Common::kReadingFailed;
		needsRedraw = true;
		return Common::kNoError;
	}

	needsRedraw = true;
	return Common::kNoError;
}

Common::Error ItemInteractions::handleInventoryTargetInteraction(const ObjectRecord &target,
		Interaction &interactionProcessor, bool &needsRedraw) {
	if (!_inventory.hasSelection())
		return Common::kNoError;

	const Common::String selectedItemName = _inventory.getSelectedItemName();
	InteractionResult interaction;
	const bool handled = _engine.getScript()->resolveUseItemInteraction(
		selectedItemName, target, interaction, _scene.state.roomName);
	if (!handled) {
		debugC(1, kDebugInventory,
			"Harvester: inventory target miss selected='%s' target='%s' owner='%s' action='%s'",
			selectedItemName.c_str(), target.objectName.c_str(),
			target.currentOwnerOrRoom.c_str(), target.actionTag.c_str());
		return Common::kNoError;
	}

	StartupRoomTransitionKind roomTransition = interaction.roomTransition;
	if (roomTransition == kStartupRoomTransitionNone && !interaction.nextRoomName.empty())
		roomTransition = kStartupRoomTransitionCloseup;

	// Flow shares one inventory system across nested room loops. Clear the held item
	// before entering a closeup so it is not still carried while the child room loop
	// is active.
	if (!interaction.nextRoomName.empty() &&
			roomTransition != kStartupRoomTransitionChangeRoom)
		_inventory.clearSelection();

	bool didTransition = false;
	Common::Error interactionError =
		interactionProcessor.handleInteractionResult(interaction, didTransition, selectedItemName);
	if (interactionError.getCode() != Common::kNoError)
		return interactionError;
	_inventory.clearSelection();
	if (interactionProcessor.hasPendingMainMenuReturn())
		return Common::kNoError;
	if (!_inventory.refresh())
		return Common::kReadingFailed;

	needsRedraw = true;
	return Common::kNoError;
}

ObjectRecord *ItemInteractions::findSelectedInventoryRoomTarget(const Common::Point &point) {
	if (!_inventory.hasSelection())
		return nullptr;

	EntityManager *entityManager = _engine.getRuntimeEntities();
	Script *script = _engine.getScript();
	if (!entityManager || !script)
		return nullptr;

	const Common::String selectedItemName = _inventory.getSelectedItemName();
	ObjectRecord *bestTarget = nullptr;
	int bestDrawIndex = -1;
	for (ObjectRecord &candidate : _scene.sceneObjects) {
		if (candidate.objectName.empty() ||
				!script->hasUseItemInteraction(selectedItemName, candidate)) {
			continue;
		}

		const Entity *entity = entityManager->findSceneEntityByName(candidate.objectName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() == kEntityClassBackground ||
				entity->getClassId() == kEntityClassPlayer ||
				entity->getClassId() == kEntityClassRectHotspot19) {
			continue;
		}

		const int drawIndex = entityManager->findSceneEntityDrawIndexByName(entity->getName());
		if (drawIndex < 0)
			continue;
		if (!bestTarget || drawIndex > bestDrawIndex) {
			bestTarget = &candidate;
			bestDrawIndex = drawIndex;
		}
	}

	return bestTarget;
}

Common::Error ItemInteractions::handleRoomItemPickup(const ObjectRecord &object, bool playerPresent,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw) {
	InteractionResult blockedPickupInteraction;
	if (_engine.getScript()->isPickupBlockedByAction(object, &blockedPickupInteraction)) {
		bool didTransition = false;
		Common::Error interactionError = interactionProcessor.handleInteractionResult(
			blockedPickupInteraction, didTransition, Common::String());
		if (interactionError.getCode() != Common::kNoError)
			return interactionError;
		needsRedraw = true;
		return Common::kNoError;
	}

	return playerPresent
		? beginRoomItemCarry(object, interactionProcessor, callbacks, needsRedraw)
		: moveRoomItemDirectlyToInventory(object, interactionProcessor, callbacks, needsRedraw);
}

Interaction::Interaction(HarvesterEngine &engine, Flow &flow,
		SceneState &scene, RoomPlayerState &playerState,
		Common::String &pendingRegionName, Common::String &pendingRoomChange,
		bool &pendingRoomChangeIsRoomName, bool &pendingRoomChangeUsesSavedRoomState,
		SaveRoomState &pendingRoomChangeSavedRoomState, bool canExitCloseupToParent,
		InteractionCallbacks &callbacks)
	: _engine(engine), _flow(flow), _scene(scene), _playerState(playerState),
	  _pendingRegionName(pendingRegionName), _pendingRoomChange(pendingRoomChange),
	  _pendingRoomChangeIsRoomName(pendingRoomChangeIsRoomName),
	  _pendingRoomChangeUsesSavedRoomState(pendingRoomChangeUsesSavedRoomState),
	  _pendingRoomChangeSavedRoomState(pendingRoomChangeSavedRoomState),
	  _canExitCloseupToParent(canExitCloseupToParent), _callbacks(callbacks) {
}

void Interaction::queueRegionInteraction(const RegionRecord &region, const Common::Point &mousePos) {
	_pendingRegionName = region.regionName;
	if (!_playerState.entity)
		return;

	const bool playerOverlapsRegion = doesPlayerOverlapRegion(*_playerState.entity, region);
	if (_engine.isPathfindingDebugEnabled()) {
		const Common::Rect playerRect = _playerState.entity->getScreenRect();
		const Common::Rect regionBounds(region.left, region.top, region.right + 1, region.bottom + 1);
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding region click room='%s' mouse=(%d,%d) region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d] start=%d cursor=%d desired_facing=%d player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d) overlaps=%d action='%s'",
			_scene.state.roomName.c_str(), mousePos.x, mousePos.y,
			region.regionName.c_str(),
			regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
			region.minZ, region.maxZ, region.startEnabled, region.cursorEnabled,
			region.desiredFacing,
			_playerState.centerX, _playerState.bottomY, (double)_playerState.z,
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			playerOverlapsRegion, region.actionTag.c_str());
	}
	if (playerOverlapsRegion && doesPlayerFacingMatchRegion(_playerState.facing, region))
		return;

	Player::setRegionMoveTarget(_scene.state, _playerState,
		Player::resolveRegionTargetX(_scene.state, region, _playerState),
		Player::resolveRegionTargetZ(region));
	if (_engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding region target room='%s' region='%s' target=(%d,%d,z=%.2f)",
			_scene.state.roomName.c_str(), region.regionName.c_str(),
			_playerState.targetX, _playerState.targetBottomY, (double)_playerState.targetZ);
	}
}

Common::Error Interaction::runRegionInteraction(const RegionRecord &region,
		InventorySystem &inventory, bool &needsRedraw) {
	InteractionResult interaction;
	if (!_engine.getScript()->resolveRegionInteraction(region, interaction, _scene.state.roomName))
		return Common::kNoError;
	debugC(2, kDebugRoom,
		"Harvester: region interaction room='%s' region='%s' action='%s' player=(%d,%d,z=%.2f) next_room='%s'",
		_scene.state.roomName.c_str(), region.regionName.c_str(), region.actionTag.c_str(),
		_playerState.centerX, _playerState.bottomY, (double)_playerState.z,
		interaction.nextRoomName.c_str());

	bool didTransition = false;
	Common::Error interactionError =
		handleInteractionResult(interaction, didTransition, Common::String());
	if (interactionError.getCode() != Common::kNoError)
		return interactionError;
	if (_flow.hasPendingMainMenuReturn())
		return Common::kNoError;

	if (!inventory.refresh())
		return Common::kReadingFailed;
	needsRedraw = true;
	return Common::kNoError;
}

Common::Error Interaction::tryActivatePendingRegion(InventorySystem &inventory, bool &needsRedraw) {
	if (_pendingRegionName.empty() || !_playerState.entity)
		return Common::kNoError;

	const RegionRecord *region = findSceneRegionByName(_scene.sceneRegions, _pendingRegionName);
	if (!region || !region->startEnabled) {
		_pendingRegionName.clear();
		return Common::kNoError;
	}
	if (!doesPlayerOverlapRegion(*_playerState.entity, *region))
		return Common::kNoError;
	if (!doesPlayerFacingMatchRegion(_playerState.facing, *region)) {
		if (!_playerState.hasMoveTarget && !_playerState.turnActive && region->desiredFacing >= 0)
			(void)Player::startTurnAnimation(_playerState, region->desiredFacing);
		return Common::kNoError;
	}

	_callbacks.stopPlayerRegionInteraction();
	return runRegionInteraction(*region, inventory, needsRedraw);
}

Common::Error Interaction::tryActivatePassiveRegion(InventorySystem &inventory, bool &needsRedraw) {
	if (!_playerState.entity)
		return Common::kNoError;

	for (const RegionRecord &region : _scene.sceneRegions) {
		if (!region.startEnabled)
			continue;
		if (!doesPlayerOverlapRegion(*_playerState.entity, region))
			continue;
		if (!doesPlayerFacingMatchRegion(_playerState.facing, region))
			continue;

		_callbacks.stopPlayerRegionInteraction();
		return runRegionInteraction(region, inventory, needsRedraw);
	}

	return Common::kNoError;
}

int Interaction::getPendingRegionFacing() const {
	const RegionRecord *pendingRegion = findSceneRegionByName(_scene.sceneRegions, _pendingRegionName);
	return pendingRegion && pendingRegion->startEnabled ? pendingRegion->desiredFacing : -1;
}

bool Interaction::hasPendingMainMenuReturn() const {
	return _flow.hasPendingMainMenuReturn();
}

Common::Error Interaction::handleCombatInteraction(const InteractionResult &interaction, bool &needsRedraw) {
	bool didTransition = false;
	Common::Error interactionError =
		handleInteractionResult(interaction, didTransition, Common::String());
	if (interactionError.getCode() != Common::kNoError)
		return interactionError;
	if (!_flow.hasPendingMainMenuReturn() &&
			_engine.getScript() &&
			_engine.getScript()->getPlayerCurrentHitPoints() <= 0) {
		_callbacks.stopPlayerRegionInteraction();
		_callbacks.startPlayerDefeatSequence("combat_interaction", Common::String(), 1);
	}

	needsRedraw = true;
	return Common::kNoError;
}

Common::Error Interaction::handleInteractionResult(const InteractionResult &interaction,
		bool &didTransition, const Common::String &usedItemName, bool interruptPlayerMovement) {
	didTransition = false;

	if (interruptPlayerMovement) {
		_playerState.hasMoveTarget = false;
		_playerState.turnActive = false;
		_playerState.turnTargetFacing = -1;
		_pendingRegionName.clear();
	} else if (_playerState.hasMoveTarget || _playerState.turnActive) {
		debugC(2, kDebugPlayer,
			"Harvester: background action preserving player movement room='%s' pos=(%d,%d,z=%.2f) target=(%d,%d,z=%.2f) region='%s' turning=%d",
			_scene.state.roomName.c_str(), _playerState.centerX, _playerState.bottomY,
			_playerState.z, _playerState.targetX, _playerState.targetBottomY,
			_playerState.targetZ, _pendingRegionName.c_str(), _playerState.turnActive);
	}
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
	else if (interaction.requestPlayerGotoZ)
		_callbacks.applyPlayerGotoXZ(_playerState.centerX, interaction.playerGotoZ);

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
				handleInteractionResult(continuationInteraction, didTransition, usedItemName, interruptPlayerMovement);
			if (interactionError.getCode() != Common::kNoError)
				return interactionError;
		}
	}

	return Common::kNoError;
}

Common::Error Interaction::runScriptedDialogue(const Common::String &npcName,
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
		interaction.requestPlayerGotoZ ||
		interaction.lightingCommand != kStartupLightingCommandNone ||
		!interaction.modalText.value.empty() ||
		!interaction.dialogueNpcName.empty() ||
		!interaction.dialogueContinuationTag.empty() ||
		!interaction.continuationTag.empty() ||
		interaction.mutatedRuntimeState ||
		interaction.visualRuntimeStateChanged;
}

} // End of namespace Harvester
