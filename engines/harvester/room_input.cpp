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

#include "harvester/room_input.h"

#include "common/debug.h"
#include "graphics/screen.h"
#include "harvester/art.h"
#include "harvester/cheats.h"
#include "harvester/detection.h"
#include "harvester/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player.h"
#include "harvester/room_interaction.h"
#include "harvester/room_presentation.h"
#include "harvester/room_session.h"
#include "harvester/room_support.h"

namespace Harvester {

static const uint32 kRoomExitFastClickWindowTicks = 20;
static const int kNativeInventoryDragCloseLeft = 0x45;
static const int kNativeInventoryDragCloseTop = 0x4b;
static const int kNativeInventoryDragCloseRight = 0x239;
static const int kNativeInventoryDragCloseBottom = 0x1b4;
static const uint32 kCombatLoadoutStatusDisplayTicks = 75;

static bool roomAllowsImmediateExitClick(const Common::String &roomName) {
	return !roomName.equalsIgnoreCase("LAVAPIT") &&
		!roomName.equalsIgnoreCase("BOWLSNTRY1");
}

static bool isOutsideNativeInventoryDragCloseBounds(const Common::Point &point) {
	return point.x < kNativeInventoryDragCloseLeft ||
		point.x >= kNativeInventoryDragCloseRight ||
		point.y < kNativeInventoryDragCloseTop ||
		point.y >= kNativeInventoryDragCloseBottom;
}

static void debugLogInventoryClick(const char *buttonLabel, const Common::Point &point,
		const InventoryVisual *inventoryHover) {
	if (!inventoryHover) {
		debugC(1, kDebugInventory,
			"Harvester: inventory %s click at (%d,%d) hit no item", buttonLabel, point.x, point.y);
		return;
	}

	debugC(1, kDebugInventory,
		"Harvester: inventory %s click at (%d,%d) object='%s' sprite='%s' alt='%s' action='%s' owner='%s' text='%s' bounds=(%d,%d)-(%d,%d) exit=%d status=%d",
		buttonLabel, point.x, point.y, inventoryHover->object.objectName.c_str(),
		inventoryHover->object.spritePath.c_str(), inventoryHover->object.altSpritePath.c_str(),
		inventoryHover->object.actionTag.c_str(), inventoryHover->object.currentOwnerOrRoom.c_str(),
		inventoryHover->object.inventoryTextKey.c_str(), inventoryHover->bounds.left,
		inventoryHover->bounds.top, inventoryHover->bounds.right, inventoryHover->bounds.bottom,
		InventorySystem::isExitObject(inventoryHover->object),
		InventorySystem::isStatusObject(inventoryHover->object));
}

RoomInput::RoomInput(HarvesterEngine &engine, Flow &flow, RoomSession &session, RoomSceneResources &scene,
		InventorySystem &inventory, CheatSystem &cheats,
		RoomItemInteractions &roomItems, RoomInteractionProcessor &interactionProcessor,
		RoomPresentationState &presentationState,
		const Art &art, bool canExitCloseupToParent)
	: _engine(engine), _flow(flow), _session(session), _scene(scene),
	  _inventory(inventory), _cheats(cheats), _roomItems(roomItems), _interactionProcessor(interactionProcessor),
	  _presentationState(presentationState), _art(art),
	  _canExitCloseupToParent(canExitCloseupToParent) {
}

void RoomInput::beginInspectText(const ObjectRecord &object, const ResolvedText &text) {
	_presentationState.inspectText = text;
	_presentationState.showingInspectText = true;
	_presentationState.inspectCanDismiss = false;
	debugC(2, kDebugRoom,
		"Harvester: showing IDENT object='%s' prompt='%s' box='%s' text='%s'",
		object.objectName.c_str(), _presentationState.currentInteractionPromptText.c_str(),
		_presentationState.inspectText.boxName.c_str(), _presentationState.inspectText.value.c_str());
}

void RoomInput::queueCombatLoadoutStatusMessage(const Common::String &message, bool &needsRedraw) {
	if (message.empty()) {
		const bool hadMessage = !_presentationState.combatLoadoutStatusText.value.empty();
		_presentationState.combatLoadoutStatusText = ResolvedText();
		_presentationState.combatLoadoutStatusHideTick = 0;
		if (hadMessage)
			needsRedraw = true;
		return;
	}

	_presentationState.combatLoadoutStatusText.boxName = "BOX1";
	_presentationState.combatLoadoutStatusText.value = message;
	_presentationState.combatLoadoutStatusHideTick =
		Player::getRuntimeClockTicks() + kCombatLoadoutStatusDisplayTicks;
	needsRedraw = true;
}

bool RoomInput::openInventoryOverlay() {
	RoomPlayerState &playerState = _session.getPlayerState();
	RoomInputState &inputState = _session.getInputState();

	inputState.moveLeft = false;
	inputState.moveRight = false;
	inputState.moveUp = false;
	inputState.moveDown = false;
	_session.clearPendingRegionInteraction();
	(void)_session.cancelPlayerAttackAnimation();
	playerState.hasMoveTarget = false;
	playerState.turnActive = false;
	playerState.turnTargetFacing = -1;
	if (!_roomItems.stowCarriedRoomItemToInventory())
		return false;
	return _inventory.open();
}

void RoomInput::capturePlayerAttackTarget() {
	RoomPlayerState &playerState = _session.getPlayerState();

	playerState.attackTargetName.clear();
	playerState.attackTargetClassId = -1;

	const RoomHoverState hoverState = resolveRoomHoverState(
		_engine, _scene.state, _scene.sceneObjects, _scene.state.roomNpcs, _scene.sceneRegions,
		_flow._mousePos, _flow._menuTextConfig, &_flow._dialogue);
	_session.capturePlayerAttackTarget(_flow._mousePos, hoverState.npc);
}

void RoomInput::handleMouseMove(bool &needsRedraw) {
	if (_inventory.isOpen() && _inventory.hasSelection()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow._mousePos);
		// Native inventory handoff is driven by the latched selected item, not by
		// a second click after the selection has already been made.
		if (!inventoryHover && isOutsideNativeInventoryDragCloseBounds(_flow._mousePos)) {
			if (_inventory.close())
				needsRedraw = true;
		}
	}
	needsRedraw = true;
}

RoomInputResult RoomInput::handleRightButtonDown(bool &needsRedraw) {
	RoomPlayerState &playerState = _session.getPlayerState();
	RoomIdleAnimationState &idleState = _session.getIdleState();

	_session.notePlayerActivity();
	if (idleState.active || idleState.exiting) {
		if (Player::requestIdleAnimationExit(_scene.state, playerState, idleState))
			needsRedraw = true;
		return RoomInputResult();
	}
	if (_session.isPlayerCombatLocked())
		return RoomInputResult();
	_session.clearPendingRegionInteraction();
	if (_inventory.isOpen()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow._mousePos);
		debugLogInventoryClick("right", _flow._mousePos, inventoryHover);
		if (_inventory.hasSelection()) {
			debugC(1, kDebugInventory,
				"Harvester: inventory right click cleared active selection='%s'",
				_inventory.getSelectedItemName().c_str());
			if (_inventory.clearSelection())
				needsRedraw = true;
			return RoomInputResult();
		}
		if (inventoryHover &&
				!InventorySystem::isExitObject(inventoryHover->object) &&
				!InventorySystem::isStatusObject(inventoryHover->object)) {
			Script *script = _engine.getScript();
			const int previousLoadout = playerState.combatLoadout >= 0
				? playerState.combatLoadout
				: (script ? script->getPlayerCombatLoadout() : 0);
			bool loadoutChanged = false;
			if (_inventory.toggleCombatLoadout(inventoryHover->object,
					playerState.combatLoadout, loadoutChanged)) {
				debugC(1, kDebugInventory,
					"Harvester: inventory right click handled as combat toggle object='%s' changed=%d",
					inventoryHover->object.objectName.c_str(), loadoutChanged);
				if (loadoutChanged) {
					if (script) {
						const Common::String statusMessage = buildCombatLoadoutStatusMessage(
							*script, inventoryHover->object, previousLoadout,
							script->getPlayerCombatLoadout());
						queueCombatLoadoutStatusMessage(statusMessage, needsRedraw);
						_presentationState.closeInventoryAfterCombatLoadoutStatus = true;
					}
					_session.captureCurrentSaveState();
					if (playerState.entity) {
						(void)Player::syncCombatLoadoutVisual(_engine, _scene.state, playerState,
							_engine.getScript()->getPlayerCombatLoadout());
					} else if (_engine.getScript()) {
						playerState.combatLoadout =
							_engine.getScript()->getPlayerCombatLoadout();
					}
					if (!_inventory.refresh())
						return RoomInputResult::leaveRoom(Common::kReadingFailed);
				}
				needsRedraw = true;
				return RoomInputResult();
			}

			InventorySecondaryAction secondaryAction;
			if (_inventory.resolveSecondaryAction(inventoryHover->object, secondaryAction)) {
				debugC(1, kDebugInventory,
					"Harvester: inventory right click executing secondary action object='%s' action='%s' closeInventory=%d",
					inventoryHover->object.objectName.c_str(),
					secondaryAction.actionTag.c_str(), secondaryAction.closeInventory);
				if (secondaryAction.closeInventory)
					(void)_inventory.close();

				InteractionResult interaction;
				bool didTransition = false;
				const bool executedActionTag = _engine.getScript()->executeActionTag(
						secondaryAction.actionTag, interaction, true, _scene.state.roomName);
				debugC(1, kDebugInventory,
					"Harvester: inventory right click executeActionTag('%s') -> %d music='%s' cutscene='%s' nextRoom='%s'",
					secondaryAction.actionTag.c_str(), executedActionTag,
					interaction.musicPath.c_str(), interaction.cutscenePath.c_str(),
					interaction.nextRoomName.c_str());
				if (executedActionTag) {
					Common::Error interactionError =
						_interactionProcessor.handleInteractionResult(
							interaction, didTransition, Common::String());
					debugC(1, kDebugInventory,
						"Harvester: inventory right click action='%s' interaction result error=%d transitioned=%d pendingMainMenu=%d",
						secondaryAction.actionTag.c_str(), interactionError.getCode(),
						didTransition, _flow.hasPendingMainMenuReturn());
					if (interactionError.getCode() != Common::kNoError)
						return RoomInputResult::leaveRoom(interactionError);
					if (_flow.hasPendingMainMenuReturn())
						return RoomInputResult::leaveRoom(Common::kNoError);
				}

				if (_inventory.isOpen()) {
					if (!_inventory.refresh())
						return RoomInputResult::leaveRoom(Common::kReadingFailed);
				} else {
					debugC(1, kDebugInventory,
						"Harvester: inventory right click skipped overlay refresh after action='%s' transitioned=%d closeInventory=%d",
						secondaryAction.actionTag.c_str(), didTransition, secondaryAction.closeInventory);
				}
				needsRedraw = true;
				return RoomInputResult();
			}
			debugC(1, kDebugInventory,
				"Harvester: inventory right click object='%s' produced no combat toggle or secondary action",
				inventoryHover->object.objectName.c_str());
		}
		debugC(1, kDebugInventory, "Harvester: inventory right click closing overlay");
		if (_inventory.close())
			needsRedraw = true;
		return RoomInputResult();
	}
	if (_inventory.hasSelection()) {
		if (_inventory.clearSelection())
			needsRedraw = true;
		return RoomInputResult();
	}
	if (_roomItems.hasCarriedRoomItem()) {
		if (!_roomItems.stowCarriedRoomItemToInventory())
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return RoomInputResult();
	}
	Script *script = _engine.getScript();
	if (!_presentationState.showingInspectText && !_session.isPlayerCombatLocked() && script && playerState.entity &&
			script->getPlayerCurrentHitPoints() > 0 && !script->isPlayerControlPaused()) {
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		playerState.turnFirstFrame = -1;
		playerState.turnLastFrame = -1;
		playerState.turnEndFrame = -1;
		playerState.turnPlayBackwards = false;
		(void)Player::syncCombatLoadoutVisual(_engine, _scene.state, playerState,
			script->getPlayerCombatLoadout());
		capturePlayerAttackTarget();
		if (Player::startAttackAnimation(_scene.state, playerState, _flow._mousePos)) {
			debugC(1, kDebugCombat,
				"Harvester: combat player attack start loadout=%d weapon='%s' damage=%d damage_type='%s' target_class='%s' target='%s' cursor=(%d,%d)",
				playerState.combatLoadout,
				Player::describeCombatLoadout(playerState.combatLoadout),
				Player::resolveCombatLoadoutDamageAmount(playerState.combatLoadout),
				Player::describeCombatDamageType(Player::resolveCombatLoadoutDamageType(playerState.combatLoadout)),
				describeCombatTargetClass(playerState.attackTargetClassId),
				playerState.attackTargetName.c_str(), _flow._mousePos.x, _flow._mousePos.y);
			needsRedraw = true;
		} else {
			playerState.attackTargetName.clear();
			playerState.attackTargetClassId = -1;
		}
	}
	return RoomInputResult();
}

void RoomInput::handleLeftButtonUp() {
	RoomInputState &inputState = _session.getInputState();

	inputState.lastLeftButtonReleaseTick = Player::getRuntimeClockTicks();
	if (_presentationState.showingInspectText)
		_presentationState.inspectCanDismiss = true;
}

RoomInputResult RoomInput::handleLeftButtonDown(bool &needsRedraw) {
	RoomPlayerState &playerState = _session.getPlayerState();
	RoomIdleAnimationState &idleState = _session.getIdleState();
	RoomInputState &inputState = _session.getInputState();

	const uint32 now = Player::getRuntimeClockTicks();
	const bool isFastExitClick =
		inputState.lastLeftButtonReleaseTick != 0 &&
		now - inputState.lastLeftButtonReleaseTick < kRoomExitFastClickWindowTicks;
	_session.notePlayerActivity();
	if (idleState.active || idleState.exiting) {
		if (Player::requestIdleAnimationExit(_scene.state, playerState, idleState))
			needsRedraw = true;
		return RoomInputResult();
	}

	if (_presentationState.showingInspectText) {
		if (_presentationState.inspectCanDismiss) {
			_presentationState.showingInspectText = false;
			_presentationState.inspectCanDismiss = false;
			_presentationState.inspectText = ResolvedText();
			needsRedraw = true;
		}
		return RoomInputResult();
	}
	if (_session.isPlayerCombatLocked())
		return RoomInputResult();

	const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
	if (_inventory.isOpen()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow._mousePos);
		debugLogInventoryClick("left", _flow._mousePos, inventoryHover);
		if (inventoryHover) {
			if (InventorySystem::isExitObject(inventoryHover->object)) {
				debugC(1, kDebugInventory, "Harvester: inventory left click exit button");
				const bool clearedSelection = _inventory.clearSelection();
				const bool closedInventory = _inventory.close();
				if (clearedSelection || closedInventory)
					needsRedraw = true;
				return RoomInputResult();
			}
			if (InventorySystem::isStatusObject(inventoryHover->object))
				return RoomInputResult();

			if (!_inventory.hasSelection()) {
				debugC(1, kDebugInventory,
					"Harvester: inventory left click selecting object='%s'",
					inventoryHover->object.objectName.c_str());
				_inventory.selectItem(inventoryHover->object.objectName);
				_inventory.setPromptText(_inventory.buildSelectedPrompt(
					Common::String(), _flow._menuTextConfig));
				needsRedraw = true;
				return RoomInputResult();
			}

			if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
				debugC(1, kDebugInventory,
					"Harvester: inventory left click using selected='%s' on target='%s'",
					_inventory.getSelectedItemName().c_str(),
					inventoryHover->object.objectName.c_str());
				Common::Error interactionError =
					_roomItems.handleInventoryTargetInteraction(inventoryHover->object, _interactionProcessor, needsRedraw);
				if (interactionError.getCode() != Common::kNoError)
					return RoomInputResult::leaveRoom(interactionError);
			}
			needsRedraw = true;
			return RoomInputResult();
		}

		if (_inventory.hasSelection()) {
			if (!inventoryPanelBounds.contains(_flow._mousePos) && _inventory.close())
				needsRedraw = true;
			return RoomInputResult();
		}
		if (!inventoryPanelBounds.contains(_flow._mousePos) && _inventory.close())
			needsRedraw = true;
		return RoomInputResult();
	}

	const RoomHoverState hoverState = resolveRoomHoverState(
		_engine, _scene.state, _scene.sceneObjects, _scene.state.roomNpcs, _scene.sceneRegions,
		_flow._mousePos, _flow._menuTextConfig, &_flow._dialogue);
	ObjectRecord *selectedRoomTarget = nullptr;
	if (_inventory.hasSelection() && !hoverState.npc)
		selectedRoomTarget = _roomItems.findSelectedInventoryRoomTarget(_flow._mousePos);
	if (selectedRoomTarget &&
			(!hoverState.object ||
			 !selectedRoomTarget->objectName.equalsIgnoreCase(hoverState.object->objectName))) {
		debugC(1, kDebugInventory,
			"Harvester: inventory target remap selected='%s' hovered='%s' remapped='%s'",
			_inventory.getSelectedItemName().c_str(),
			hoverState.object ? hoverState.object->objectName.c_str() : "",
			selectedRoomTarget->objectName.c_str());
	}
	RoomHoverState clickHoverState = hoverState;
	if (selectedRoomTarget)
		clickHoverState.object = selectedRoomTarget;
	debugC(1, kDebugRoom,
		"Harvester: room click room='%s' mouse=(%d,%d) object='%s' npc='%s' region='%s' cursor_sequence=%d prompt='%s'",
		_scene.state.roomName.c_str(), _flow._mousePos.x, _flow._mousePos.y,
		clickHoverState.object ? clickHoverState.object->objectName.c_str() : "",
		clickHoverState.npc ? clickHoverState.npc->npcName.c_str() : "",
		clickHoverState.region ? clickHoverState.region->regionName.c_str() : "",
		clickHoverState.cursorSequence, clickHoverState.promptText.c_str());
	if (_roomItems.hasCarriedRoomItem()) {
		if (clickHoverState.playerEntity && playerState.entity &&
				clickHoverState.playerEntity == playerState.entity) {
			if (!_roomItems.stowCarriedRoomItemToInventory())
				return RoomInputResult::leaveRoom(Common::kReadingFailed);
			needsRedraw = true;
		}
		return RoomInputResult();
	}
	if (_inventory.hasSelection()) {
		const Common::String selectedItemName = _inventory.getSelectedItemName();
		if (clickHoverState.npc) {
			bool didTransition = false;
			Common::Error dialogueError = _interactionProcessor.runScriptedDialogue(
				clickHoverState.npc->npcName, selectedItemName, Common::String(), didTransition);
			if (dialogueError.getCode() != Common::kNoError)
				return RoomInputResult::leaveRoom(dialogueError);
			if (_flow.hasPendingMainMenuReturn())
				return RoomInputResult::leaveRoom(Common::kNoError);
			if (_inventory.clearSelection())
				needsRedraw = true;
			return RoomInputResult();
		}

		ObjectRecord *roomTarget = clickHoverState.object
			? findSceneObjectByName(_scene.sceneObjects, clickHoverState.object->objectName)
			: nullptr;
		if (roomTarget) {
			Common::Error interactionError =
				_roomItems.handleInventoryTargetInteraction(*roomTarget, _interactionProcessor, needsRedraw);
			if (interactionError.getCode() != Common::kNoError)
				return RoomInputResult::leaveRoom(interactionError);
			needsRedraw = true;
		}
		return RoomInputResult();
	}
	if (hoverState.playerEntity) {
		if (!playerState.entity || hoverState.playerEntity != playerState.entity ||
			idleState.active || idleState.exiting ||
			playerState.hasMoveTarget || playerState.turnActive ||
			playerState.entity->getAnimationRate() != 0) {
			return RoomInputResult();
		}
		if (!openInventoryOverlay())
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return RoomInputResult();
	}
	if (hoverState.npc) {
		_session.clearPendingRegionInteraction();
		inputState.moveLeft = false;
		inputState.moveRight = false;
		inputState.moveUp = false;
		inputState.moveDown = false;
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		bool didTransition = false;
		Common::Error dialogueError = _interactionProcessor.runScriptedDialogue(
			hoverState.npc->npcName, Common::String(), Common::String(), didTransition);
		if (dialogueError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(dialogueError);
		if (_flow.hasPendingMainMenuReturn())
			return RoomInputResult::leaveRoom(Common::kNoError);
		needsRedraw = true;
		return RoomInputResult();
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
				return RoomInputResult::leaveRoom(interactionError);
			if (_flow.hasPendingMainMenuReturn())
				return RoomInputResult::leaveRoom(Common::kNoError);
			needsRedraw = true;
			return RoomInputResult();
		}
		_interactionProcessor.queueRegionInteraction(*hoverState.region, _flow._mousePos);
		Common::Error interactionError =
			_interactionProcessor.tryActivatePendingRegion(_inventory, needsRedraw);
		if (interactionError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(interactionError);
		needsRedraw = true;
		return RoomInputResult();
	}
	ObjectRecord *clickedObject = hoverState.object
		? findSceneObjectByName(_scene.sceneObjects, hoverState.object->objectName)
		: nullptr;
	if (!clickedObject) {
		_session.clearPendingRegionInteraction();
		if (hoverState.cursorSequence == 0 && playerState.entity) {
			if (_engine.isPathfindingDebugEnabled()) {
				const Common::Rect playerRect = playerState.entity->getScreenRect();
				debugC(1, kDebugPathfinding,
					"Harvester: pathfinding click accepted room='%s' mouse=(%d,%d) movement_band=(y=%d..%d,z=%d..%d) player=(%d,%d,z=%.2f) player_rect=(%d,%d)-(%d,%d)",
					_scene.state.roomName.c_str(), _flow._mousePos.x, _flow._mousePos.y,
					_scene.state.roomMaxZScreenY, _scene.state.roomMinZScreenY,
					_scene.state.roomMinZ, _scene.state.roomMaxZ,
					playerState.centerX, playerState.bottomY, (double)playerState.z,
					playerRect.left, playerRect.top, playerRect.right, playerRect.bottom);
			}
			Player::setMoveTargetFromScreenPoint(_scene.state, playerState, _flow._mousePos.x, _flow._mousePos.y);
			if (_engine.isPathfindingDebugEnabled()) {
				debugC(1, kDebugPathfinding,
					"Harvester: pathfinding click target room='%s' mouse=(%d,%d) target=(%d,%d,z=%.2f)",
					_scene.state.roomName.c_str(), _flow._mousePos.x, _flow._mousePos.y,
					playerState.targetX, playerState.targetBottomY, (double)playerState.targetZ);
			}
			needsRedraw = true;
		}
		return RoomInputResult();
	}
	_session.clearPendingRegionInteraction();
	if (clickedObject->objectName.equalsIgnoreCase("EXIT_BM") ||
		clickedObject->objectName.equalsIgnoreCase("EXIT_HS")) {
		Common::Error exitError = _session.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(exitError);
		// Native closeup exits return through room_setup so the parent on-enter command runs.
		if (_canExitCloseupToParent)
			_flow.requestCloseupParentRestart();
		return RoomInputResult::leaveRoom(Common::kNoError);
	}

	ResolvedText resolvedInspectText;
	const bool hasInspectText =
		_engine.getScript()->resolveObjectInspectText(*clickedObject, resolvedInspectText);
	const bool unlocksAfterInitialExamine =
		unlocksRoomObjectInteractionAfterInitialExamine(*clickedObject, *_engine.getScript());
	const bool canShowInspectText =
		hasInspectText && resolveInspectTextboxBitmap(_art, resolvedInspectText);
	if (!clickedObject->identShown && unlocksAfterInitialExamine) {
		clickedObject->identShown = true;
		_engine.getScript()->markObjectIdentShown(*clickedObject);
		if (canShowInspectText) {
			beginInspectText(*clickedObject, resolvedInspectText);
		} else if (hasInspectText) {
			debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
				resolvedInspectText.boxName.c_str(), clickedObject->objectName.c_str());
		}
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		needsRedraw = true;
		return RoomInputResult();
	}
	if (_engine.getScript()->isPickupObject(*clickedObject)) {
		Common::Error pickupError = _roomItems.handleRoomItemPickup(*clickedObject, playerState.entity != nullptr,
			_interactionProcessor, _session, needsRedraw);
		if (pickupError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(pickupError);
		return RoomInputResult();
	}

	InteractionResult interaction;
	if (!_engine.getScript()->resolveObjectInteraction(
			*clickedObject, interaction, _scene.state.roomName)) {
		if (canShowInspectText) {
			beginInspectText(*clickedObject, resolvedInspectText);
		} else if (hasInspectText) {
			debug(1, "Harvester: unsupported IDENT textbox '%s' for object '%s'",
				resolvedInspectText.boxName.c_str(), clickedObject->objectName.c_str());
		}
		needsRedraw = true;
		return RoomInputResult();
	}

	bool didTransition = false;
	Common::Error interactionError =
		_interactionProcessor.handleInteractionResult(
			interaction, didTransition, Common::String());
	if (interactionError.getCode() != Common::kNoError)
		return RoomInputResult::leaveRoom(interactionError);
	if (_flow.hasPendingMainMenuReturn())
		return RoomInputResult::leaveRoom(Common::kNoError);
	needsRedraw = true;
	return RoomInputResult();
}

RoomInputResult RoomInput::handleKeyDown(const Common::KeyState &key, bool &needsRedraw) {
	RoomPlayerState &playerState = _session.getPlayerState();
	RoomIdleAnimationState &idleState = _session.getIdleState();
	RoomInputState &inputState = _session.getInputState();

	if (_presentationState.showingInspectText) {
		if (key.keycode == Common::KEYCODE_ESCAPE) {
			_presentationState.showingInspectText = false;
			_presentationState.inspectCanDismiss = false;
			_presentationState.inspectText = ResolvedText();
			needsRedraw = true;
		}
		return RoomInputResult();
	}

	_session.notePlayerActivity();
	if (key.hasFlags(Common::KBD_CTRL))
		inputState.attackModifierHeld = true;
	if (idleState.active || idleState.exiting) {
		if (Player::requestIdleAnimationExit(_scene.state, playerState, idleState))
			needsRedraw = true;
		return RoomInputResult();
	}

	if (Script *script = _engine.getScript()) {
		InteractionResult cheatInteraction;
		CheatActivation cheatActivation;
		const CheatInputResult cheatInputResult = _cheats.processKey(
			key, *script, _scene.state.roomName,
			cheatInteraction, cheatActivation);
		if (cheatInputResult != kCheatInputIgnored) {
			if (cheatInputResult == kCheatInputComplete && cheatActivation.activated) {
				if (cheatActivation.clearInteractionState && _inventory.clearSelection())
					needsRedraw = true;
				if (cheatActivation.inventoryChanged && _inventory.isOpen() &&
						!_inventory.refresh()) {
					return RoomInputResult::leaveRoom(Common::kReadingFailed);
				}
				if (cheatActivation.hasInteraction) {
					if (_inventory.isOpen())
						(void)_inventory.close();
					bool didTransition = false;
					Common::Error cheatError = _interactionProcessor.handleInteractionResult(
						cheatInteraction, didTransition, Common::String());
					if (cheatError.getCode() != Common::kNoError)
						return RoomInputResult::leaveRoom(cheatError);
					if (_flow.hasPendingMainMenuReturn())
						return RoomInputResult::leaveRoom(Common::kNoError);
				}
				needsRedraw = true;
			}
			return RoomInputResult();
		}
	}

	if (_inventory.isOpen()) {
		if (key.keycode == Common::KEYCODE_ESCAPE) {
			const bool clearedSelection = _inventory.clearSelection();
			const bool closedInventory = _inventory.close();
			if (clearedSelection || closedInventory)
				needsRedraw = true;
		} else if (key.keycode == Common::KEYCODE_RETURN ||
				key.keycode == Common::KEYCODE_KP_ENTER ||
				key.keycode == Common::KEYCODE_i) {
			if (_inventory.close())
				needsRedraw = true;
		}
		return RoomInputResult();
	}
	if (_inventory.hasSelection()) {
		if (key.keycode == Common::KEYCODE_ESCAPE) {
			if (_inventory.clearSelection())
				needsRedraw = true;
			return RoomInputResult();
		}
		if (key.keycode == Common::KEYCODE_i) {
			if (_inventory.clearSelection())
				needsRedraw = true;
			if (!openInventoryOverlay())
				return RoomInputResult::leaveRoom(Common::kReadingFailed);
			needsRedraw = true;
			return RoomInputResult();
		}
	}
	if (_session.isPlayerCombatLocked())
		return RoomInputResult();

	if (key.keycode == Common::KEYCODE_LCTRL ||
			key.keycode == Common::KEYCODE_RCTRL) {
		inputState.attackModifierHeld = true;
	} else if (key.keycode == Common::KEYCODE_k && _engine.isCombatDebugEnabled()) {
		Common::Error debugKillError = _session.killDebugActiveMonster(_flow._mousePos, needsRedraw);
		if (debugKillError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(debugKillError);
		return RoomInputResult();
	} else if (key.keycode == Common::KEYCODE_LEFT)
		inputState.moveLeft = true;
	else if (key.keycode == Common::KEYCODE_RIGHT)
		inputState.moveRight = true;
	else if (key.keycode == Common::KEYCODE_UP)
		inputState.moveUp = true;
	else if (key.keycode == Common::KEYCODE_DOWN)
		inputState.moveDown = true;
	else if (key.keycode == Common::KEYCODE_i) {
		if (!openInventoryOverlay())
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return RoomInputResult();
	} else if (key.keycode == Common::KEYCODE_ESCAPE) {
		if (_session.cancelPlayerAttackAnimation())
			needsRedraw = true;
		if (!_roomItems.stowCarriedRoomItemToInventory())
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		inputState.moveLeft = false;
		inputState.moveRight = false;
		inputState.moveUp = false;
		inputState.moveDown = false;
		_session.clearPendingRegionInteraction();
		playerState.hasMoveTarget = false;
		playerState.turnActive = false;
		playerState.turnTargetFacing = -1;
		_session.captureCurrentSaveState();
		Graphics::Screen *activeScreen = _engine.getScreen();
		if (!activeScreen)
			return RoomInputResult::leaveRoom(Common::kNoError);
		IndexedBitmap roomMenuBackdrop;
		drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
		if (!captureScreenBackdrop(*activeScreen, roomMenuBackdrop))
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		Common::Error menuError = _flow.runRoomMenuStub(
			roomMenuBackdrop, _scene.palette, _scene.targetPaletteBrightness,
			_scene.state.hasEntrance);
		if (menuError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(menuError);
		_flow.resetCursorAnimationSequence();
		_session.resetIdleState();
		needsRedraw = true;
		return RoomInputResult();
	}

	if (key.keycode == Common::KEYCODE_RETURN ||
		key.keycode == Common::KEYCODE_KP_ENTER) {
		if (!_roomItems.stowCarriedRoomItemToInventory())
			return RoomInputResult::leaveRoom(Common::kReadingFailed);
		Common::Error exitError = _session.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return RoomInputResult::leaveRoom(exitError);
		return RoomInputResult::leaveRoom(Common::kNoError);
	}
	return RoomInputResult();
}

void RoomInput::handleKeyUp(const Common::KeyState &key) {
	RoomInputState &inputState = _session.getInputState();

	if (key.keycode == Common::KEYCODE_LCTRL ||
			key.keycode == Common::KEYCODE_RCTRL)
		inputState.attackModifierHeld = false;
	else if (key.keycode == Common::KEYCODE_LEFT)
		inputState.moveLeft = false;
	else if (key.keycode == Common::KEYCODE_RIGHT)
		inputState.moveRight = false;
	else if (key.keycode == Common::KEYCODE_UP)
		inputState.moveUp = false;
	else if (key.keycode == Common::KEYCODE_DOWN)
		inputState.moveDown = false;
}

RoomInputResult RoomInput::handleEvent(const Common::Event &event, bool &needsRedraw) {
	Common::Error result = Common::kNoError;
	if (_flow.handleSystemEvent(event, result))
		return RoomInputResult::leaveRoom(result);

	switch (event.type) {
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
	return RoomInputResult();
}

} // End of namespace Harvester
