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

#include "harvester/scene/input.h"

#include "common/debug.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/dialogue/dialogue.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player/player.h"
#include "harvester/scene/interaction.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/scene/scene.h"
#include "harvester/scene/room_support.h"
#include "harvester/scene/scene_entities.h"

namespace Harvester {

static const int kCursorSequenceWalk = 0;
static const int kCursorSequenceExamine = 1;
static const int kCursorSequenceTalk = 2;
static const int kCursorSequenceOperate = 4;
static const int kCursorSequencePickup = 5;
static const int kCursorSequenceTransition = 6;
static const int kCursorSequenceNeutral = 7;

static const uint32 kRoomExitFastClickWindowTicks = 20;

static bool roomAllowsImmediateExitClick(const Common::String &roomName) {
	return !roomName.equalsIgnoreCase("LAVAPIT") &&
		!roomName.equalsIgnoreCase("BOWLSNTRY1");
}

static bool canTalkToRoomNpc(HarvesterEngine &engine, const NpcRecord &npc,
		const DialogueSystem *dialogue) {
	Script *script = engine.getScript();
	if (!script || !script->isNamedNpcDeathTypeClear(npc.npcName))
		return false;

	return dialogue && dialogue->hasRoomNpcHandler(npc.npcName);
}

const ObjectRecord *Input::findRoomObjectAtPoint(HarvesterEngine &engine,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *topEntity = nullptr;
	const ObjectRecord *topObject = nullptr;
	int topDrawIndex = -1;
	for (const ObjectRecord &object : sceneObjects) {
		if (object.objectName.empty())
			continue;

		const Entity *entity = entityManager->findSceneEntityByName(object.objectName);
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
		const bool objectVisible = object.visible && object.runtimeVisible;
		const bool topObjectVisible = topObject && topObject->visible && topObject->runtimeVisible;
		if (!topEntity ||
			(objectVisible && !topObjectVisible) ||
			(objectVisible == topObjectVisible && drawIndex > topDrawIndex)) {
			topEntity = entity;
			topObject = &object;
			topDrawIndex = drawIndex;
		}
	}
	if (!topEntity)
		return nullptr;

	return topObject;
}

static const RegionRecord *findRoomRegionAtPoint(HarvesterEngine &engine,
		const Common::Array<RegionRecord> &sceneRegions, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *topEntity = nullptr;
	int topDrawIndex = -1;
	for (const RegionRecord &region : sceneRegions) {
		const Entity *entity = entityManager->findSceneEntityByName(region.regionName);
		if (!entity || !entity->hitTest(point))
			continue;
		if (entity->getClassId() != kEntityClassRectHotspot19)
			continue;
		const int drawIndex = entityManager->findSceneEntityDrawIndexByName(entity->getName());
		if (drawIndex < 0)
			continue;
		if (!topEntity || drawIndex > topDrawIndex) {
			topEntity = entity;
			topDrawIndex = drawIndex;
		}
	}
	if (!topEntity)
		return nullptr;

	return SceneEntities::findSceneRegionByName(sceneRegions, topEntity->getName());
}

static const Entity *findRoomPlayerAtPoint(HarvesterEngine &engine, const Common::Point &point) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *entity = entityManager->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kEntityClassPlayer)
		return nullptr;

	return entity;
}

static const NpcRecord *findRoomNpcAtPoint(HarvesterEngine &engine,
		const Common::Array<NpcRecord> &sceneNpcs, const Common::Point &point,
		const DialogueSystem *dialogue) {
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (!entityManager)
		return nullptr;

	const Entity *entity = entityManager->findTopSceneEntityAt(point);
	if (!entity || entity->getClassId() != kEntityClassNpc)
		return nullptr;

	const NpcRecord *npc = SceneEntities::findSceneNpcByName(sceneNpcs, entity->getName());
	if (!npc || !canTalkToRoomNpc(engine, *npc, dialogue))
		return nullptr;

	return npc;
}

static bool suppressesInitialObjectInspectGate(const ObjectRecord &object) {
	// Native HARVEST.SCR uses ident key X as a no-text sentinel.
	return object.identTextKey.equalsIgnoreCase("X");
}

static bool usesBareOperatePrompt(const ObjectRecord &object) {
	return object.objectName.equalsIgnoreCase("HAPPLY_HS") ||
		object.objectName.equalsIgnoreCase("KILL_STEPH_HS");
}

static bool unlocksRoomObjectInteractionAfterInitialExamine(const ObjectRecord &object,
		Script &script) {
	if (suppressesInitialObjectInspectGate(object))
		return false;

	return object.operatable || script.isPickupObject(object);
}

static int resolveRoomObjectCursorSequence(const ObjectRecord &object, Script &script) {
	if (object.objectName.equalsIgnoreCase("EXIT_BM") || object.objectName.equalsIgnoreCase("EXIT_HS"))
		return kCursorSequenceTransition;

	ResolvedText inspectText;
	const bool pickupObject = script.isPickupObject(object);
	const bool pickupBlocked = pickupObject && script.isPickupBlockedByAction(object);
	if (!object.identShown && unlocksRoomObjectInteractionAfterInitialExamine(object, script))
		return kCursorSequenceExamine;
	if (pickupObject && !pickupBlocked)
		return kCursorSequencePickup;
	if (pickupBlocked)
		return kCursorSequenceExamine;
	if (object.operatable)
		return kCursorSequenceOperate;
	if (script.resolveObjectInspectText(object, inspectText))
		return kCursorSequenceExamine;

	if (script.hasObjectInteraction(object))
		return kCursorSequenceExamine;

	return kCursorSequenceNeutral;
}

static Common::String buildRoomObjectPrompt(const ObjectRecord &object, Script &script,
		int cursorSequence, const MenuTextConfig &menuTextConfig) {
	// Native room prompts come from explicit interaction metadata. Neutral scene sprites
	// should not surface synthetic "Examine <object id>" prompts or steal hover/click focus.
	if (cursorSequence == kCursorSequenceNeutral)
		return Common::String();

	const Common::String label = script.resolveObjectLabel(object);
	if (label.empty())
		return Common::String();

	if (cursorSequence == kCursorSequenceOperate) {
		if (usesBareOperatePrompt(object))
			return label;
		return Common::String::format("%s %s", menuTextConfig.operateVerb.c_str(), label.c_str());
	}
	if (cursorSequence == kCursorSequencePickup)
		return Common::String::format("%s %s", menuTextConfig.pickUpVerb.c_str(), label.c_str());
	if (cursorSequence == kCursorSequenceTalk)
		return Common::String::format("%s %s", menuTextConfig.talkToVerb.c_str(), label.c_str());
	if (cursorSequence == kCursorSequenceTransition)
		return Common::String::format("Go to %s", label.c_str());

	return Common::String::format("%s %s", menuTextConfig.examineTheVerb.c_str(), label.c_str());
}

static Common::String buildRoomNpcPrompt(const NpcRecord &npc,
		const MenuTextConfig &menuTextConfig) {
	Common::String label = !npc.entityInitArg.empty() ? npc.entityInitArg : npc.npcName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	if (label.empty())
		return Common::String();
	return Common::String::format("%s %s", menuTextConfig.talkToVerb.c_str(), label.c_str());
}

RoomHoverState Input::resolveRoomHoverState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<NpcRecord> &sceneNpcs,
		const Common::Array<RegionRecord> &sceneRegions, const Common::Point &mousePos,
		const MenuTextConfig &menuTextConfig, const DialogueSystem *dialogue) {
	RoomHoverState hoverState;
	EntityManager *entityManager = engine.getRuntimeEntities();
	if (const Entity *playerEntity = findRoomPlayerAtPoint(engine, mousePos)) {
		hoverState.playerEntity = playerEntity;
		hoverState.cursorSequence = kCursorSequenceExamine;
		return hoverState;
	}
	if (const NpcRecord *npc = findRoomNpcAtPoint(engine, sceneNpcs, mousePos, dialogue)) {
		hoverState.npc = npc;
		hoverState.cursorSequence = kCursorSequenceTalk;
		hoverState.promptText = buildRoomNpcPrompt(*npc, menuTextConfig);
		return hoverState;
	}

	Script *script = engine.getScript();
	if (!script)
		return hoverState;
	hoverState.object = findRoomObjectAtPoint(engine, sceneObjects, mousePos);
	hoverState.region = findRoomRegionAtPoint(engine, sceneRegions, mousePos);
	if (entityManager && hoverState.object && hoverState.region) {
		const int objectDrawIndex =
			entityManager->findSceneEntityDrawIndexByName(hoverState.object->objectName);
		const int regionDrawIndex =
			entityManager->findSceneEntityDrawIndexByName(hoverState.region->regionName);
		if (regionDrawIndex >= 0 && objectDrawIndex >= 0 && regionDrawIndex > objectDrawIndex)
			hoverState.object = nullptr;
	}
	if (hoverState.object) {
		hoverState.cursorSequence = resolveRoomObjectCursorSequence(*hoverState.object, *script);
		hoverState.promptText = buildRoomObjectPrompt(
			*hoverState.object, *script, hoverState.cursorSequence, menuTextConfig);
		if (hoverState.cursorSequence != kCursorSequenceNeutral || !hoverState.promptText.empty())
			return hoverState;
		hoverState.object = nullptr;
	}
	if (hoverState.region && hoverState.region->cursorEnabled) {
		hoverState.object = nullptr;
		hoverState.cursorSequence = kCursorSequenceTransition;
		hoverState.promptText.clear();
		return hoverState;
	}

	if (Player::supportsMovementBand(state) &&
		mousePos.y >= state.roomMaxZScreenY &&
		mousePos.y <= state.roomMinZScreenY) {
		hoverState.cursorSequence = kCursorSequenceWalk;
	}

	return hoverState;
}

Input::Input(HarvesterEngine &engine, Flow &flow, Scene &session, SceneState &scene,
		InventorySystem &inventory, CheatSystem &cheats,
		ItemInteractions &roomItems, Interaction &interactionProcessor,
		RoomPresentationState &presentationState,
		const Art &art, bool canExitCloseupToParent)
	: _engine(engine), _flow(flow), _session(session), _scene(scene),
	  _inventory(inventory), _cheats(cheats), _roomItems(roomItems), _interactionProcessor(interactionProcessor),
	  _presentationState(presentationState), _art(art),
	  _canExitCloseupToParent(canExitCloseupToParent) {
}

void Input::beginInspectText(const ObjectRecord &object, const ResolvedText &text) {
	_pendingInspectObjectName.clear();
	_inspectDismissPressed = false;
	_presentationState.inspectText = text;
	_presentationState.showingInspectText = true;
	_presentationState.inspectCanDismiss = false;
	debugC(2, kDebugRoom,
		"Harvester: showing IDENT object='%s' prompt='%s' box='%s' text='%s'",
		object.objectName.c_str(), _presentationState.currentInteractionPromptText.c_str(),
		_presentationState.inspectText.boxName.c_str(), _presentationState.inspectText.value.c_str());
}

InputResult Input::finishInspectText(bool &needsRedraw) {
	_inspectDismissPressed = false;
	_presentationState.showingInspectText = false;
	_presentationState.inspectCanDismiss = false;
	_presentationState.inspectText = ResolvedText();
	needsRedraw = true;
	const Common::String objectName = _pendingInspectObjectName;
	_pendingInspectObjectName.clear();
	if (objectName.empty())
		return InputResult();

	const ObjectRecord *object = SceneEntities::findSceneObjectByName(_scene.sceneObjects, objectName);
	InteractionResult interaction;
	if (!object || !_engine.getScript()->resolveObjectInteraction(*object, interaction, _scene.executionContext()))
		return InputResult();

	return applyObjectInteraction(interaction, needsRedraw);
}

InputResult Input::applyObjectInteraction(InteractionResult &interaction, bool &needsRedraw) {
	bool didTransition = false;
	Common::Error interactionError =
		_interactionProcessor.handleInteractionResult(
			interaction, didTransition, Common::String());
	if (interactionError.getCode() != Common::kNoError)
		return InputResult::leaveRoom(interactionError);
	if (_flow.hasPendingMainMenuReturn())
		return InputResult::leaveRoom(Common::kNoError);
	needsRedraw = true;
	return InputResult();
}

void Input::capturePlayerAttackTarget() {
	Player &player = _session.getPlayer();

	player.clearAttackTarget();

	const RoomHoverState hoverState = _flow.getRoomHoverState(_scene);
	_session.capturePlayerAttackTarget(_flow.getMousePos(), hoverState.npc);
}

InputResult Input::handleRightButtonDown(bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomPlayerState &playerState = player.getState();
	const RoomIdleAnimationState &idleState = player.getIdleState();

	_session.notePlayerActivity();
	if (idleState.active || idleState.exiting) {
		if (player.requestIdleAnimationExit(_scene.state))
			needsRedraw = true;
		return InputResult();
	}
	if (_session.isPlayerCombatLocked())
		return InputResult();
	_session.clearPendingRegionInteraction();
	if (_inventory.isOpen())
		return handleInventoryRightClick(needsRedraw);

	if (_inventory.hasSelection()) {
		if (_inventory.clearSelection())
			needsRedraw = true;
		return InputResult();
	}
	if (_roomItems.hasCarriedRoomItem()) {
		if (!_roomItems.stowCarriedRoomItemToInventory())
			return InputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return InputResult();
	}
	Script *script = _engine.getScript();
	if (!_presentationState.showingInspectText && !_session.isPlayerCombatLocked() && script && playerState.entity &&
			script->getPlayerCurrentHitPoints() > 0 && !script->isPlayerControlPaused()) {
		player.cancelMovementForAttack();
		(void)player.syncCombatLoadoutVisual(_engine, _scene.state, script->getPlayerCombatLoadout());
		capturePlayerAttackTarget();
		if (player.startAttackAnimation(_scene.state, _flow.getMousePos())) {
			debugC(1, kDebugCombat,
				"Harvester: combat player attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' cursor=(%d,%d)",
				playerState.combatLoadout,
				Player::describeCombatLoadout(playerState.combatLoadout),
				Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
				Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
				describeCombatTargetClass(playerState.attackTargetClassId),
				playerState.attackTargetName.c_str(), _flow.getMousePos().x, _flow.getMousePos().y);
			needsRedraw = true;
		} else {
			player.clearAttackTarget();
		}
	}
	return InputResult();
}

void Input::handleLeftButtonUp() {
	InputState &inputState = _session.getInputState();

	inputState.lastLeftButtonReleaseTick = Player::getRuntimeClockTicks();
	if (_presentationState.showingInspectText)
		_presentationState.inspectCanDismiss = true;
}

InputResult Input::handleLeftButtonDown(bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomIdleAnimationState &idleState = player.getIdleState();
	InputState &inputState = _session.getInputState();

	const uint32 now = Player::getRuntimeClockTicks();
	const bool isFastExitClick =
		inputState.lastLeftButtonReleaseTick != 0 &&
		now - inputState.lastLeftButtonReleaseTick < kRoomExitFastClickWindowTicks;
	_session.notePlayerActivity();
	if (idleState.active || idleState.exiting) {
		if (player.requestIdleAnimationExit(_scene.state))
			needsRedraw = true;
		return InputResult();
	}

	if (_presentationState.showingInspectText) {
		if (_presentationState.inspectCanDismiss)
			_inspectDismissPressed = true;
		return InputResult();
	}
	if (_session.isPlayerCombatLocked())
		return InputResult();

	if (_inventory.isOpen())
		return handleInventoryLeftClick(needsRedraw);

	return handleSceneClick(isFastExitClick, needsRedraw);
}

InputResult Input::handleSceneClick(bool isFastExitClick, bool &needsRedraw) {
	const RoomPlayerState &playerState = _session.getPlayer().getState();

	const RoomHoverState hoverState = _flow.getRoomHoverState(_scene);
	RoomHoverState clickHoverState = hoverState;
	const Entity *itemTarget = nullptr;
	if (_roomItems.hasSelectedItem()) {
		clickHoverState = RoomHoverState();
		itemTarget = _roomItems.findSelectedItemTarget(_flow.getMousePos());
		if (!itemTarget)
			return InputResult();
		if (itemTarget == playerState.entity)
			clickHoverState.playerEntity = itemTarget;
		if (itemTarget->getClassId() == kEntityClassNpc &&
				_flow.hasRoomNpcHandler(itemTarget->getName()) &&
				_engine.getScript()->isNamedNpcDeathTypeClear(itemTarget->getName())) {
			for (const NpcRecord &npc : _scene.state.roomNpcs) {
				if (npc.npcName.equalsIgnoreCase(itemTarget->getName())) {
					clickHoverState.npc = &npc;
					break;
				}
			}
		}
		clickHoverState.object = SceneEntities::findSceneObjectByName(_scene.sceneObjects, itemTarget->getName());
	}
	debugC(1, kDebugRoom,
		"Harvester: room click room='%s' mouse=(%d,%d) object='%s' npc='%s' region='%s' cursor_sequence=%d prompt='%s'",
		_scene.state.roomName.c_str(), _flow.getMousePos().x, _flow.getMousePos().y,
		clickHoverState.object ? clickHoverState.object->objectName.c_str() : "",
		clickHoverState.npc ? clickHoverState.npc->npcName.c_str() : "",
		clickHoverState.region ? clickHoverState.region->regionName.c_str() : "",
		clickHoverState.cursorSequence, clickHoverState.promptText.c_str());
	if (_roomItems.hasCarriedRoomItem()) {
		if (clickHoverState.playerEntity && playerState.entity &&
				clickHoverState.playerEntity == playerState.entity) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return InputResult::leaveRoom(Common::kReadingFailed);
			needsRedraw = true;
			return InputResult();
		}
	}
	if (_roomItems.hasSelectedItem())
		return handleSelectedItemClick(clickHoverState, itemTarget, needsRedraw);
	return handleSceneTargetClick(hoverState, isFastExitClick, needsRedraw);
}

InputResult Input::handleSelectedItemClick(const RoomHoverState &clickHoverState,
		const Entity *itemTarget, bool &needsRedraw) {
	InputState &inputState = _session.getInputState();

	const Common::String selectedItemName = _roomItems.getSelectedItemName();
	if (clickHoverState.npc) {
		inputState.clearHeldActions();
		_roomItems.clearSelectedItem();
		bool didTransition = false;
		Common::Error dialogueError = _interactionProcessor.runScriptedDialogue(
			clickHoverState.npc->npcName, selectedItemName, Common::String(), didTransition);
		if (dialogueError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(dialogueError);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		needsRedraw = true;
		return InputResult();
	}

	ObjectRecord *roomTarget = clickHoverState.object
		? SceneEntities::findSceneObjectByName(_scene.sceneObjects, clickHoverState.object->objectName)
		: nullptr;
	if (roomTarget) {
		Common::Error interactionError =
			_roomItems.handleSelectedItemTargetInteraction(*roomTarget, _interactionProcessor, needsRedraw);
		if (interactionError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(interactionError);
		needsRedraw = true;
	} else if (itemTarget) {
		Common::Error interactionError = _roomItems.handleSelectedItemMiss(_interactionProcessor, needsRedraw);
		if (interactionError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(interactionError);
	}
	return InputResult();
}

InputResult Input::handleSceneTargetClick(const RoomHoverState &hoverState,
		bool isFastExitClick, bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomPlayerState &playerState = player.getState();
	const RoomIdleAnimationState &idleState = player.getIdleState();
	InputState &inputState = _session.getInputState();

	if (hoverState.playerEntity) {
		if (!playerState.entity || hoverState.playerEntity != playerState.entity ||
			idleState.active || idleState.exiting ||
			playerState.hasMoveTarget || playerState.turnActive ||
			playerState.entity->getAnimationRate() != 0) {
			return InputResult();
		}
		if (!openInventoryOverlay())
			return InputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return InputResult();
	}
	if (hoverState.npc) {
		_session.clearPendingRegionInteraction();
		inputState.clearHeldActions();
		player.cancelMovement();
		bool didTransition = false;
		Common::Error dialogueError = _interactionProcessor.runScriptedDialogue(
			hoverState.npc->npcName, Common::String(), Common::String(), didTransition);
		if (dialogueError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(dialogueError);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
		needsRedraw = true;
		return InputResult();
	}
	// select one target in reverse render order; a selected object prevents exit-region selection.
	if (hoverState.region && !hoverState.object && playerState.entity) {
		if (isFastExitClick &&
			hoverState.region->startEnabled &&
			roomAllowsImmediateExitClick(_scene.state.roomName)) {
			_session.stopPlayerRegionInteraction();
			Common::Error interactionError =
				_interactionProcessor.runRegionInteraction(*hoverState.region, _inventory, needsRedraw);
			if (interactionError.getCode() != Common::kNoError)
				return InputResult::leaveRoom(interactionError);
			if (_flow.hasPendingMainMenuReturn())
				return InputResult::leaveRoom(Common::kNoError);
			needsRedraw = true;
			return InputResult();
		}
		_interactionProcessor.queueRegionInteraction(*hoverState.region, _flow.getMousePos());
		Common::Error interactionError =
			_interactionProcessor.tryActivatePendingRegion(_inventory, needsRedraw);
		if (interactionError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(interactionError);
		needsRedraw = true;
		return InputResult();
	}
	ObjectRecord *clickedObject = hoverState.object
		? SceneEntities::findSceneObjectByName(_scene.sceneObjects, hoverState.object->objectName)
		: nullptr;
	if (!clickedObject)
		return handleWalkClick(hoverState, needsRedraw);
	return handleObjectClick(*clickedObject, needsRedraw);
}

InputResult Input::handleWalkClick(const RoomHoverState &hoverState, bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomPlayerState &playerState = player.getState();

	_session.clearPendingRegionInteraction();
	if (hoverState.cursorSequence == 0 && playerState.entity) {
		if (_engine.isPathfindingDebugEnabled()) {
			const Common::Rect playerRect = playerState.entity->getScreenRect();
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding click accepted room='%s' mouse=(%d,%d) movement_band=(y=%d..%d,z=%d..%d) player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d)",
				_scene.state.roomName.c_str(), _flow.getMousePos().x, _flow.getMousePos().y,
				_scene.state.roomMaxZScreenY, _scene.state.roomMinZScreenY,
				_scene.state.roomMinZ, _scene.state.roomMaxZ,
				playerState.centerX, playerState.bottomY, static_cast<double>(playerState.z),
				playerRect.left, playerRect.top, playerRect.right, playerRect.bottom);
		}
		player.setMoveTargetFromScreenPoint(_scene.state, _flow.getMousePos().x, _flow.getMousePos().y);
		if (_engine.isPathfindingDebugEnabled()) {
			debugC(1, kDebugPathfinding,
				"Harvester: pathfinding click target room='%s' mouse=(%d,%d) target=(%d,%d,z=%.2f)",
				_scene.state.roomName.c_str(), _flow.getMousePos().x, _flow.getMousePos().y,
				playerState.targetX, playerState.targetBottomY, static_cast<double>(playerState.targetZ));
		}
		needsRedraw = true;
	}
	return InputResult();
}

InputResult Input::handleObjectClick(ObjectRecord &object, bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomPlayerState &playerState = player.getState();

	_session.clearPendingRegionInteraction();
	if (object.objectName.equalsIgnoreCase("EXIT_BM") ||
		object.objectName.equalsIgnoreCase("EXIT_HS")) {
		Common::Error exitError = _session.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(exitError);
		// Native closeup exits return through room_setup so the parent on-enter command runs.
		if (_canExitCloseupToParent)
			_flow.requestCloseupParentRestart();
		return InputResult::leaveRoom(Common::kNoError);
	}

	ResolvedText resolvedInspectText;
	const bool hasInspectText =
		_engine.getScript()->resolveObjectInspectText(object, resolvedInspectText);
	const bool unlocksAfterInitialExamine =
		unlocksRoomObjectInteractionAfterInitialExamine(object, *_engine.getScript());
	const bool canShowInspectText =
		hasInspectText && RoomPresentation::resolveInspectTextboxBitmap(_art, resolvedInspectText);
	if (!object.identShown && unlocksAfterInitialExamine) {
		object.identShown = true;
		_engine.getScript()->markObjectIdentShown(object);
		if (canShowInspectText) {
			beginInspectText(object, resolvedInspectText);
		} else if (hasInspectText) {
			debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
				resolvedInspectText.boxName.c_str(), object.objectName.c_str());
		}
		player.cancelMovement();
		needsRedraw = true;
		return InputResult();
	}
	// An IDENT can be shown before a non-operatable object's action, without setting identShown. Dispatch after dismissal.
	if (!object.identShown && !suppressesInitialObjectInspectGate(object) &&
			!unlocksAfterInitialExamine && canShowInspectText) {
		beginInspectText(object, resolvedInspectText);
		if (!object.actionTag.empty())
			_pendingInspectObjectName = object.objectName;
		player.cancelMovement();
		needsRedraw = true;
		return InputResult();
	}
	if (_engine.getScript()->isPickupObject(object)) {
		Common::Error pickupError = _roomItems.handleRoomItemPickup(object, playerState.entity != nullptr,
			_interactionProcessor, _session, needsRedraw);
		if (pickupError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(pickupError);
		return InputResult();
	}

	InteractionResult interaction;
	if (!_engine.getScript()->resolveObjectInteraction(
			object, interaction, _scene.executionContext())) {
		if (canShowInspectText) {
			beginInspectText(object, resolvedInspectText);
		} else if (hasInspectText) {
			debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
				resolvedInspectText.boxName.c_str(), object.objectName.c_str());
		}
		needsRedraw = true;
		return InputResult();
	}

	return applyObjectInteraction(interaction, needsRedraw);
}

InputResult Input::handleEvent(const Common::Event &event, bool &needsRedraw) {
	Common::Error result = Common::kNoError;
	if (_flow.handleSystemEvent(event, result))
		return InputResult::leaveRoom(result);

	if (_presentationState.showingInspectText && _inspectDismissPressed) {
		if (event.type == Common::EVENT_LBUTTONUP ||
				(event.type == Common::EVENT_KEYUP && event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				(event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == kActionMenu)) {
			if (event.type == Common::EVENT_LBUTTONUP)
				handleLeftButtonUp();
			return finishInspectText(needsRedraw);
		}
	}

	switch (event.type) {
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		return handleGameAction((GameAction)event.customType,
			event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START, needsRedraw);
	case Common::EVENT_MOUSEMOVE:
		handleMouseMove(needsRedraw);
		break;
	case Common::EVENT_RBUTTONDOWN:
		return handleRightButtonDown(needsRedraw);
	case Common::EVENT_LBUTTONUP:
		handleLeftButtonUp();
		break;
	case Common::EVENT_LBUTTONDOWN:
		return handleLeftButtonDown(needsRedraw);
	case Common::EVENT_KEYDOWN:
		return handleKeyDown(event.kbd, needsRedraw);
	case Common::EVENT_KEYUP:
		handleKeyUp(event.kbd);
		break;
	default:
		break;
	}
	return InputResult();
}

} // End of namespace Harvester
