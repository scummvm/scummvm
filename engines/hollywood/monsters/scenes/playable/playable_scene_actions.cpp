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

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/monsters/hollywood.h"
#include "hollywood/debug.h"
#include "hollywood/monsters/gameplay/cursor.h"
#include "hollywood/monsters/gameplay/inventory_actions.h"
#include "hollywood/monsters/scenes/playable/playable_scene.h"

namespace Hollywood {

const byte kActionInvalidFacing = 0xff;
const byte kActionInvalidCel = 0xff;

void PlayableScene::handleLeftClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	if (gameplayActionInputBlocked())
		return;

	processSceneActionClick(state);
}

void PlayableScene::handleInventoryItemClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	if (gameplayActionInputBlocked())
		return;

	_lastInventoryActionItemId = state.resolvedItem;
	_lastInventoryPrimaryItemId = state.primaryInventoryItem;
	_vm->cursor()->leaveInteractiveMode();
	if (_playerDirectedActorPathActive) {
		cancelConcurrentActorPath();
		_activeActorCel = 0;
	}
	if (!dispatchGenericInventoryAction(state))
		dispatchSceneAction(state.inventoryActionHandlerId);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

bool PlayableScene::gameplayActionInputBlocked() const {
	// NPC chatter does not block actions; scripted speech waits handle their own input.
	return isRealtimeSecondarySpeechActive() || _speechOverlay.visible ||
		_actionOverlayPlayer.isVisible() || _hideActiveActor ||
		(_actorPathPlaybackActive && !_playerDirectedActorPathActive);
}

void PlayableScene::runGameplayAction(uint16 handlerId) {
	if (handlerId == 0)
		return;

	if (_playerDirectedActorPathActive) {
		cancelConcurrentActorPath();
		_activeActorCel = 0;
	}
	_vm->cursor()->leaveInteractiveMode();
	dispatchSceneAction(handlerId);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void PlayableScene::processSceneActionClick(const GameplayLoopCursorState &state) {
	byte itemId = state.resolvedItem;
	if (state.relationModeActive) {
		processSceneRelationClick(state, itemId);
		return;
	}

	if (itemId == 0) {
		_lastSceneActionItemId = 0;
		if (state.currentStrip != 1)
			return;
		if (!shouldPlayGameplayClickPath())
			return;

		int targetX = state.sceneX;
		int targetY = state.sceneY;
		debugC(1, kDebugPath,
			"%s path click: cursor=(%u,%u) viewport=(%u,%u) rawTarget=(%d,%d) rawRegion=%u rawWalk=%u actor=(%d,%d) actorRegion=%u actorDrawOrder=%u",
			sceneDebugName(), state.cursorX, state.cursorY, viewportXOffset(), viewportYOffset(),
			targetX, targetY, paletteRegionAt(targetX, targetY), walkableMaskAt(targetX, targetY),
			_activeActorWorldX, _activeActorWorldY, paletteRegionAt(_activeActorWorldX, _activeActorWorldY),
			_activeActorDrawOrderMode);
		if (!adjustWalkTargetToFloorMask(targetX, targetY)) {
			debugC(1, kDebugPath, "%s path target rejected: raw=(%u,%u)", sceneDebugName(),
				state.sceneX, state.sceneY);
			return;
		}
		debugC(1, kDebugPath, "%s path target adjusted: raw=(%u,%u) adjusted=(%d,%d) adjustedRegion=%u adjustedWalk=%u",
			sceneDebugName(), state.sceneX, state.sceneY, targetX, targetY,
			paletteRegionAt(targetX, targetY), walkableMaskAt(targetX, targetY));
		startPlayerDirectedActorPath(targetX, targetY, kActionInvalidFacing, 0, 0);
		return;
	}

	SceneVerbActionRecord actionRecord = _hotspots.verbActionRecord(itemId, state.currentStrip);
	if (actionRecord.actionHandlerId == 0)
		return;
	_lastSceneActionItemId = itemId;
	if (!shouldPlayGameplayClickPath()) {
		runGameplayAction(actionRecord.actionHandlerId);
		return;
	}

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = target.interactionPoint.x;
	int targetY = target.interactionPoint.y;
	byte finalFacing = kActionInvalidFacing;
	byte finalCel = kActionInvalidCel;

	if (actionRecord.movementMode == 0) {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else {
			targetX = _activeActorWorldX;
			targetY = _activeActorWorldY;
			if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
				finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
					target.approachPoint.x, target.approachPoint.y);
			}
		}
	}
	if (actionRecord.movementMode == 1)
		finalFacing = target.facing;
	finalFacing = customizeSceneActionFacing(actionRecord.actionHandlerId, finalFacing);
	if (actionRecord.movementMode != 3)
		finalCel = 0;

	if (!startPlayerDirectedActorPath(targetX, targetY, finalFacing, finalCel,
			actionRecord.actionHandlerId))
		runGameplayAction(actionRecord.actionHandlerId);
}

void PlayableScene::processSceneRelationClick(const GameplayLoopCursorState &state, byte itemId) {
	if (itemId == 0)
		return;

	const SceneVerbActionRecord actionRecord =
		_hotspots.relationActionRecord(state.primaryInventoryItem, itemId, state.relationMode);
	if (actionRecord.actionHandlerId == 0)
		return;
	_lastSceneActionItemId = itemId;
	_lastInventoryActionItemId = 0;
	_lastInventoryPrimaryItemId = state.primaryInventoryItem;
	if (!shouldPlayGameplayClickPath()) {
		runGameplayAction(actionRecord.actionHandlerId);
		return;
	}

	const SceneActionTarget target = _hotspots.actionTarget(itemId);
	int targetX = _activeActorWorldX;
	int targetY = _activeActorWorldY;
	byte finalFacing = kActionInvalidFacing;

	if (actionRecord.movementMode != 0) {
		targetX = target.interactionPoint.x;
		targetY = target.interactionPoint.y;
		finalFacing = target.facing;
	} else {
		const bool atInteractionPoint =
			_activeActorWorldX == target.interactionPoint.x &&
			_activeActorWorldY == target.interactionPoint.y;
		if (atInteractionPoint) {
			if (_activeActorFacing != target.facing)
				finalFacing = target.facing;
		} else if (target.approachPoint.x != 0 || target.approachPoint.y != 0) {
			finalFacing = calculateFacingTowardPoint(_activeActorWorldX, _activeActorWorldY,
				target.approachPoint.x, target.approachPoint.y);
		}
	}
	finalFacing = customizeSceneActionFacing(actionRecord.actionHandlerId, finalFacing);

	if (!startPlayerDirectedActorPath(targetX, targetY, finalFacing, 0,
			actionRecord.actionHandlerId))
		runGameplayAction(actionRecord.actionHandlerId);
}

bool PlayableScene::dispatchGenericInventoryAction(const GameplayLoopCursorState &state) {
	const uint16 handlerId = state.inventoryActionHandlerId;

	switch (handlerId) {
	case 310: // Usar huevo/navaja-lima con gorro de aviador (use egg/nail-file knife with aviator cap).
		if (sceneStageIndex() == 802)
			return false;
		beginStaticSecondarySpeechLine(0xda, (byte)_random.getRandomNumber(1));
		return true;
	default:
		return false;
	}
}

void PlayableScene::dispatchSceneAction(uint16 handlerId) {
	if (dispatchCustomSceneAction(handlerId))
		return;

	if (dispatchGenericSceneAction(handlerId))
		return;

	warning("Unhandled %s action handler %u", sceneDebugName(), handlerId);
}

bool PlayableScene::dispatchGenericSceneAction(uint16 handlerId) {
	return dispatchSharedAction(_vm, this, handlerId, inventoryOwnerIndex());
}

} // End of namespace Hollywood
