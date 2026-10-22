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
#include "harvester/detection.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player/player.h"
#include "harvester/scene/interaction.h"
#include "harvester/scene/room_support.h"
#include "harvester/scene/scene.h"

namespace Harvester {

static const int kNativeInventoryDragCloseLeft = 0x45;
static const int kNativeInventoryDragCloseTop = 0x4b;
static const int kNativeInventoryDragCloseRight = 0x239;
static const int kNativeInventoryDragCloseBottom = 0x1b4;
static const uint32 kCombatLoadoutStatusDisplayTicks = 75;

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

void Input::queueCombatLoadoutStatusMessage(const Common::String &message, bool &needsRedraw) {
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

bool Input::openInventoryOverlay() {
	Player &player = _session.getPlayer();
	InputState &inputState = _session.getInputState();

	inputState.clearHeldActions();
	_session.clearPendingRegionInteraction();
	(void)_session.cancelPlayerAttackAnimation();
	player.cancelMovement();
	if (!_roomItems.stowCarriedRoomItemToInventory())
		return false;
	return _inventory.open();
}

void Input::handleMouseMove(bool &needsRedraw) {
	if (_inventory.isOpen() && _inventory.hasSelection()) {
		const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow.getMousePos());
		// Native inventory handoff is driven by the latched selected item, not by
		// a second click after the selection has already been made.
		if (!inventoryHover && isOutsideNativeInventoryDragCloseBounds(_flow.getMousePos())) {
			if (_inventory.close())
				needsRedraw = true;
		}
	}
	needsRedraw = true;
}

InputResult Input::handleInventoryLeftClick(bool &needsRedraw) {
	const Common::Rect inventoryPanelBounds = _inventory.getPanelBounds();
	const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow.getMousePos());
	debugLogInventoryClick("left", _flow.getMousePos(), inventoryHover);
	if (inventoryHover) {
		if (InventorySystem::isExitObject(inventoryHover->object)) {
			debugC(1, kDebugInventory, "Harvester: inventory left click exit button");
			const bool clearedSelection = _inventory.clearSelection();
			const bool closedInventory = _inventory.close();
			if (clearedSelection || closedInventory)
				needsRedraw = true;
			return InputResult();
		}
		if (InventorySystem::isStatusObject(inventoryHover->object))
			return InputResult();

		if (!_inventory.hasSelection()) {
			debugC(1, kDebugInventory,
				"Harvester: inventory left click selecting object='%s'",
				inventoryHover->object.objectName.c_str());
			_inventory.selectItem(inventoryHover->object.objectName);
			_inventory.setPromptText(_inventory.buildSelectedPrompt(
				Common::String(), _flow.getMenuTextConfig()));
			needsRedraw = true;
			return InputResult();
		}

		if (!inventoryHover->object.objectName.equalsIgnoreCase(_inventory.getSelectedItemName())) {
			debugC(1, kDebugInventory,
				"Harvester: inventory left click using selected='%s' on target='%s'",
				_inventory.getSelectedItemName().c_str(),
				inventoryHover->object.objectName.c_str());
			Common::Error interactionError =
				_roomItems.handleSelectedItemTargetInteraction(inventoryHover->object, _interactionProcessor, needsRedraw);
			if (interactionError.getCode() != Common::kNoError)
				return InputResult::leaveRoom(interactionError);
		}
		needsRedraw = true;
		return InputResult();
	}

	if (!inventoryPanelBounds.contains(_flow.getMousePos()) && _inventory.close())
		needsRedraw = true;
	return InputResult();
}

InputResult Input::handleInventoryRightClick(bool &needsRedraw) {
	Player &player = _session.getPlayer();
	const RoomPlayerState &playerState = player.getState();

	const InventoryVisual *inventoryHover = _inventory.findItemAtPoint(_flow.getMousePos());
	debugLogInventoryClick("right", _flow.getMousePos(), inventoryHover);
	if (_inventory.hasSelection()) {
		debugC(1, kDebugInventory,
			"Harvester: inventory right click cleared active selection='%s'",
			_inventory.getSelectedItemName().c_str());
		if (_inventory.clearSelection())
			needsRedraw = true;
		return InputResult();
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
				player.syncCombatLoadout(_engine, _scene.state);
				if (!_inventory.refresh())
					return InputResult::leaveRoom(Common::kReadingFailed);
			}
			needsRedraw = true;
			return InputResult();
		}

		InventorySecondaryAction secondaryAction;
		if (_inventory.resolveSecondaryAction(inventoryHover->object, secondaryAction))
			return applyInventorySecondaryAction(inventoryHover->object, secondaryAction, needsRedraw);
		debugC(1, kDebugInventory,
			"Harvester: inventory right click object='%s' produced no combat toggle or secondary action",
			inventoryHover->object.objectName.c_str());
	}
	debugC(1, kDebugInventory, "Harvester: inventory right click closing overlay");
	if (_inventory.close())
		needsRedraw = true;
	return InputResult();
}

InputResult Input::applyInventorySecondaryAction(const ObjectRecord &object,
		const InventorySecondaryAction &secondaryAction, bool &needsRedraw) {
	debugC(1, kDebugInventory,
		"Harvester: inventory right click executing secondary action object='%s' action='%s' closeInventory=%d",
		object.objectName.c_str(),
		secondaryAction.actionTag.c_str(), secondaryAction.closeInventory);
	if (secondaryAction.closeInventory)
		(void)_inventory.close();

	InteractionResult interaction;
	bool didTransition = false;
	const bool executedActionTag = _engine.getScript()->executeActionTag(
			secondaryAction.actionTag, interaction, true, _scene.executionContext());
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
			return InputResult::leaveRoom(interactionError);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
	}

	if (_inventory.isOpen()) {
		if (!_inventory.refresh())
			return InputResult::leaveRoom(Common::kReadingFailed);
	} else {
		debugC(1, kDebugInventory,
			"Harvester: inventory right click skipped overlay refresh after action='%s' transitioned=%d closeInventory=%d",
			secondaryAction.actionTag.c_str(), didTransition, secondaryAction.closeInventory);
	}
	needsRedraw = true;
	return InputResult();
}

InputResult Input::handleInventoryKeyDown(const Common::KeyState &key, bool &needsRedraw) {
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
	return InputResult();
}

} // End of namespace Harvester
