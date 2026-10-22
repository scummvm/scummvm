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

#include "harvester/scene/interaction.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player/player.h"
#include "harvester/scene/scene_entities.h"
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

bool ItemInteractions::hasSelectedItem() const {
	return hasCarriedRoomItem() || _inventory.hasSelection();
}

Common::String ItemInteractions::getSelectedItemName() const {
	return hasCarriedRoomItem() ? _carriedRoomItemName : _inventory.getSelectedItemName();
}

void ItemInteractions::clearSelectedItem() {
	clearCarriedRoomItem();
	_inventory.clearSelection();
}

void ItemInteractions::hideCarriedRoomItem() {
	if (!hasCarriedRoomItem())
		return;
	hideSceneObject(_carriedRoomItemName, nullptr);
	_sceneRuntime.removeSceneEntityByName(_carriedRoomItemName);
}

void ItemInteractions::clearCarriedRoomItem() {
	_carriedRoomItemName.clear();
	_carriedRoomItemLabel.clear();
	_carriedRoomItemBitmap = IndexedBitmap();
}

void ItemInteractions::hideSceneObject(const Common::String &objectName, const Common::String *ownerOrRoom) {
	ObjectRecord *sceneObject = SceneEntities::findSceneObjectByName(_scene.sceneObjects, objectName);
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
	// This returns whether fields changed, not whether the record was found.
	(void)script.syncRuntimeObjectRecord(savedObject);
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
				object.actionTag, pickupInteraction, true, _scene.executionContext())) {
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

	ObjectRecord carriedObject = object;
	++carriedObject.currentX;
	(void)script->syncRuntimeObjectRecord(carriedObject);
	hideCarriedRoomItem();
	debugC(2, kDebugRoom, "Harvester: carrying room item object='%s' owner='%s'",
		object.objectName.c_str(), object.currentOwnerOrRoom.c_str());

	bool didTransition = false;
	if (shouldDispatchPickupActionOnCarryStart(object)) {
		InteractionResult pickupInteraction;
		if (script->executeActionTag(
				object.actionTag, pickupInteraction, true, _scene.executionContext())) {
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

Common::Error ItemInteractions::handleSelectedItemTargetInteraction(const ObjectRecord &target,
		Interaction &interactionProcessor, bool &needsRedraw) {
	if (!hasSelectedItem())
		return Common::kNoError;

	const Common::String selectedItemName = getSelectedItemName();
	InteractionResult interaction;
	const bool handled = _engine.getScript()->resolveUseItemInteraction(
		selectedItemName, target, interaction, _scene.executionContext());
	if (!handled) {
		debugC(3, kDebugRoom, "Harvester: item use miss selected='%s' target='%s'",
			selectedItemName.c_str(), target.objectName.c_str());
		return handleSelectedItemMiss(interactionProcessor, needsRedraw);
	}

	// GOG handle_target_interaction (0x7ff50) accepts room-carried items and
	// requests a scene rebuild after every USEITEM match. Clear the selection
	// before callbacks can enter another room or stow a consumed item.
	clearSelectedItem();
	interaction.visualRuntimeStateChanged = true;
	debugC(2, kDebugRoom, "Harvester: using item object='%s' target='%s'",
		selectedItemName.c_str(), target.objectName.c_str());

	bool didTransition = false;
	Common::Error interactionError =
		interactionProcessor.handleInteractionResult(interaction, didTransition, selectedItemName);
	if (interactionError.getCode() != Common::kNoError)
		return interactionError;
	if (interactionProcessor.hasPendingMainMenuReturn())
		return Common::kNoError;
	if (!_inventory.refresh())
		return Common::kReadingFailed;

	needsRedraw = true;
	return Common::kNoError;
}

Common::Error ItemInteractions::handleSelectedItemMiss(Interaction &interactionProcessor, bool &needsRedraw) {
	static const char *const responseTags[] = {
		"GEN_RESP_3", "GEN_RESP_2", "GEN_RESP_4", "GEN_RESP_1", "GEN_RESP_1"
	};
	InteractionResult interaction;
	_engine.getScript()->executeActionTag(responseTags[_engine.getRandomNumber(4)],
		interaction, true, _scene.executionContext());
	bool didTransition = false;
	Common::Error interactionError =
		interactionProcessor.handleInteractionResult(interaction, didTransition, getSelectedItemName());
	needsRedraw = true;
	return interactionError;
}

const Entity *ItemInteractions::findSelectedItemTarget(const Common::Point &point, bool opaquePixels) {
	if (!hasSelectedItem())
		return nullptr;
	EntityManager *entityManager = _engine.getRuntimeEntities();
	const IndexedBitmap *bitmap = hasCarriedRoomItem() ? &_carriedRoomItemBitmap : _inventory.getSelectedItemBitmap();
	if (!entityManager || !bitmap || !bitmap->isValid())
		return nullptr;
	const Common::Point position(point.x - (int)bitmap->width / 2, point.y - (int)bitmap->height / 2);
	return entityManager->findTopSceneEntityOverlappingBitmap(*bitmap, position, getSelectedItemName(), opaquePixels);
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
		SceneState &scene, Player &player,
		Common::String &pendingRegionName, Common::String &pendingRoomChange,
		bool &pendingRoomChangeIsRoomName, bool &pendingRoomChangeUsesSavedRoomState,
		SaveRoomState &pendingRoomChangeSavedRoomState, bool canExitCloseupToParent,
		InteractionCallbacks &callbacks)
	: _engine(engine), _flow(flow), _scene(scene), _player(player),
	  _pendingRegionName(pendingRegionName), _pendingRoomChange(pendingRoomChange),
	  _pendingRoomChangeIsRoomName(pendingRoomChangeIsRoomName),
	  _pendingRoomChangeUsesSavedRoomState(pendingRoomChangeUsesSavedRoomState),
	  _pendingRoomChangeSavedRoomState(pendingRoomChangeSavedRoomState),
	  _canExitCloseupToParent(canExitCloseupToParent), _callbacks(callbacks) {
}

void Interaction::queueRegionInteraction(const RegionRecord &region, const Common::Point &mousePos) {
	const RoomPlayerState &playerState = _player.getState();
	_pendingRegionName = region.regionName;
	if (!playerState.entity)
		return;

	const bool playerOverlapsRegion = doesPlayerOverlapRegion(*playerState.entity, region);
	if (_engine.isPathfindingDebugEnabled()) {
		const Common::Rect playerRect = playerState.entity->getScreenRect();
		const Common::Rect regionBounds(region.left, region.top, region.right + 1, region.bottom + 1);
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding region click room='%s' mouse=(%d,%d) region='%s' bounds=(%d,%d)-(%d,%d) z=[%d,%d] start=%d cursor=%d desired_facing=%d player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d) overlaps=%d action='%s'",
			_scene.state.roomName.c_str(), mousePos.x, mousePos.y,
			region.regionName.c_str(),
			regionBounds.left, regionBounds.top, regionBounds.right, regionBounds.bottom,
			region.minZ, region.maxZ, region.startEnabled, region.cursorEnabled,
			region.desiredFacing,
			playerState.centerX, playerState.bottomY, (double)playerState.z,
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			playerOverlapsRegion, region.actionTag.c_str());
	}
	if (playerOverlapsRegion && doesPlayerFacingMatchRegion(playerState.facing, region))
		return;

	_player.setRegionMoveTarget(_scene.state,
		Player::resolveRegionTargetX(_scene.state, region, playerState), Player::resolveRegionTargetZ(region));
	if (_engine.isPathfindingDebugEnabled()) {
		debugC(1, kDebugPathfinding,
			"Harvester: pathfinding region target room='%s' region='%s' target=(%d,%d,z=%.2f)",
			_scene.state.roomName.c_str(), region.regionName.c_str(),
			playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
	}
}

Common::Error Interaction::runRegionInteraction(const RegionRecord &region,
		InventorySystem &inventory, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	InteractionResult interaction;
	if (!_engine.getScript()->resolveRegionInteraction(region, interaction, _scene.executionContext()))
		return Common::kNoError;
	debugC(2, kDebugRoom,
		"Harvester: region interaction room='%s' region='%s' action='%s' player=(%d,%d,z=%.2f) next_room='%s'",
		_scene.state.roomName.c_str(), region.regionName.c_str(), region.actionTag.c_str(),
		playerState.centerX, playerState.bottomY, (double)playerState.z,
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
	const RoomPlayerState &playerState = _player.getState();
	if (_pendingRegionName.empty() || !playerState.entity)
		return Common::kNoError;

	const RegionRecord *region = SceneEntities::findSceneRegionByName(_scene.sceneRegions, _pendingRegionName);
	if (!region || !region->startEnabled) {
		_pendingRegionName.clear();
		return Common::kNoError;
	}
	if (!doesPlayerOverlapRegion(*playerState.entity, *region))
		return Common::kNoError;
	if (!doesPlayerFacingMatchRegion(playerState.facing, *region)) {
		if (!playerState.hasMoveTarget && !playerState.turnActive && region->desiredFacing >= 0)
			(void)_player.startTurnAnimation(region->desiredFacing);
		return Common::kNoError;
	}

	_callbacks.stopPlayerRegionInteraction();
	return runRegionInteraction(*region, inventory, needsRedraw);
}

Common::Error Interaction::tryActivatePassiveRegion(InventorySystem &inventory, bool &needsRedraw) {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.entity)
		return Common::kNoError;

	for (const RegionRecord &region : _scene.sceneRegions) {
		if (!region.startEnabled)
			continue;
		if (!doesPlayerOverlapRegion(*playerState.entity, region))
			continue;
		if (!doesPlayerFacingMatchRegion(playerState.facing, region))
			continue;

		_callbacks.stopPlayerRegionInteraction();
		return runRegionInteraction(region, inventory, needsRedraw);
	}

	return Common::kNoError;
}

int Interaction::getPendingRegionFacing() const {
	const RegionRecord *pendingRegion = SceneEntities::findSceneRegionByName(_scene.sceneRegions, _pendingRegionName);
	return pendingRegion && pendingRegion->startEnabled ? pendingRegion->desiredFacing : -1;
}

Common::Error Interaction::handleInteractionResult(const InteractionResult &interaction,
		bool &didTransition, const Common::String &usedItemName, bool interruptPlayerMovement) {
	const RoomPlayerState &playerState = _player.getState();
	didTransition = false;

	preparePlayerForInteraction(playerState, interruptPlayerMovement);
	if (interaction.requestDemoEnding)
		return handleDemoEnding(didTransition);

	if (interaction.requestMainMenu)
		return handleMainMenuReturn(interaction.deathFlicPath);

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

	if (interaction.requestRoomRestart)
		return queueImplicitRoomRestart(interaction, discChanged, didTransition);
	if (interaction.requestCloseupExit)
		return queueCloseupExit(didTransition);

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
		return queueImplicitRoomRestart(interaction, discChanged, didTransition);

	if (!interaction.nextRoomName.empty() &&
			roomTransition == kStartupRoomTransitionChangeRoom) {
		return queueRoomChange(interaction.nextRoomName, didTransition);
	} else if (!interaction.nextRoomName.empty()) {
		return runCloseupInteraction(interaction, discChanged, restoreMusicPath, didTransition);
	} else if (interaction.visualRuntimeStateChanged) {
		if (!_callbacks.applyCurrentRoomRuntimeMutationsInPlace(true) &&
				!_callbacks.refreshCurrentScene(true)) {
			return Common::kReadingFailed;
		}
	}

	if (didTransition)
		return Common::kNoError;

	return applySameSceneEffects(interaction, playerState, didTransition, usedItemName,
		interruptPlayerMovement);
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
