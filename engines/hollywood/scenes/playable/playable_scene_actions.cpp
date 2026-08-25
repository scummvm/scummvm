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

#include "hollywood/scenes/playable/playable_scene.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "hollywood/gameplay/cursor.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const byte kActionInvalidFacing = 0xff;
const byte kActionInvalidCel = 0xff;

void PlayableScene::handleLeftClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_vm->cursor()->leaveInteractiveMode();
	processSceneActionClick(state);
	if (!Engine::shouldQuit() && !shouldExitGameplayLoop()) {
		_skipRequested = false;
		_vm->cursor()->enterInteractiveMode();
		_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	}
}

void PlayableScene::handleInventoryItemClick(const GameplayLoopCursorState &state) {
	_skipRequested = false;
	_lastInventoryActionItemId = state.resolvedItem;
	_lastInventoryPrimaryItemId = state.primaryInventoryItem;
	_vm->cursor()->leaveInteractiveMode();
	if (!dispatchGenericInventoryAction(state))
		dispatchSceneAction(state.inventoryActionHandlerId);
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
		walkActiveActorTo(targetX, targetY, kActionInvalidFacing, 0, true);
		return;
	}

	SceneVerbActionRecord actionRecord = _hotspots.verbActionRecord(itemId, state.currentStrip);
	if (actionRecord.actionHandlerId == 0)
		return;
	_lastSceneActionItemId = itemId;
	if (!shouldPlayGameplayClickPath()) {
		dispatchSceneAction(actionRecord.actionHandlerId);
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
	if (actionRecord.movementMode != 3)
		finalCel = 0;

	if (!walkActiveActorTo(targetX, targetY, finalFacing, finalCel, true))
		return;
	dispatchSceneAction(actionRecord.actionHandlerId);
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
		dispatchSceneAction(actionRecord.actionHandlerId);
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

	if (!walkActiveActorTo(targetX, targetY, finalFacing, 0, true))
		return;
	dispatchSceneAction(actionRecord.actionHandlerId);
}

bool PlayableScene::dispatchGenericInventoryAction(const GameplayLoopCursorState &state) {
	const uint16 handlerId = state.inventoryActionHandlerId;

	switch (handlerId) {
	case 310: // Usar huevo/navaja-lima con gorro de aviador (use egg/nail-file knife with aviator cap).
		beginStaticSecondarySpeechLine(0xda, (byte)_random.getRandomNumber(1));
		return true;
	case 331: // Usar lupa con placa/póster/rata/etc. (use magnifying glass with cell inventory objects).
		beginSecondarySpeechLine(0x13, 0);
		return true;
	case 335: // Usar bisturí/navaja multiusos con placa/póster/etc. (use cutting tools with cell inventory objects).
		beginSecondarySpeechLine(0x17, 0);
		return true;
	case 336: // Dar pamela a rata (give hat to rat).
		beginSecondarySpeechLine(0x18, 0);
		return true;
	case 337: // Usar/Dar placa/perfume/pintura/pintauñas/etc. con rata (use/give items with rat).
		if (_vm->gameState().cellPlateRatProgress != 0) {
			beginSecondarySpeechLine(0x19, 1);
			return true;
		}

		beginSecondarySpeechLine(0x19, 0);
		if (state.primaryInventoryItem == 0x14 || state.resolvedItem == 0x14) {
			_vm->gameState().cellPlateRatProgress = 1;
			_vm->gameState().cellPipesActive = false;
			removeInventoryItem(0x14);
			_soundBank0.playSample(1, 100);
			if (sceneStageIndex() == 710)
				applySceneStateToHotspotsAndPatches(0xff);
		}
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
	// Shared callbacks used directly by scene action tables.
	switch (handlerId) {
	case 0:  // Shared no-op/default action slot.
	case 1:  // Usar/Dar inventory relation starter; no direct speech.
	case 35: // Mirar florero variant (look at vase): silent in original.
	case 41: // Mirar florero variant (look at vase): silent in original.
	case 45: // Mirar florero variant (look at vase): silent in original.
	case 49: // Mirar disco (look at record): silent in original.
		return true;
	case 2: // Generic failed use/combine: no effect.
		beginStaticSecondarySpeechLine(1, (byte)_random.getRandomNumber(1));
		return true;
	case 3: // Generic nonsensical action.
		beginStaticSecondarySpeechLine(2, 0);
		return true;
	case 4: // Generic impossible action/object too large.
		beginStaticSecondarySpeechLine(3, (byte)_random.getRandomNumber(1));
		return true;
	case 5: // Sue refuses risky physical action: impossible/stockings/nails.
	{
		const byte variant = (byte)_random.getRandomNumber(2);
		if (variant == 2)
			beginStaticSecondarySpeechLine(3, 1);
		else
			beginStaticSecondarySpeechLine(4, variant);
		return true;
	}
	case 6: // Coger inventory item already owned (take already-have item).
		beginStaticSecondarySpeechLine(5, 0);
		return true;
	case 7: // Abrir non-openable inventory object (open object).
		beginStaticSecondarySpeechLine(6, (byte)_random.getRandomNumber(1));
		return true;
	case 8: // Abrir bote de pintura (open paint can): already open.
		beginStaticSecondarySpeechLine(7, 0);
		return true;
	case 9: // Generic feminine object already open.
		beginStaticSecondarySpeechLine(8, 0);
		return true;
	case 10: // Cerrar non-closable inventory object (close object).
		beginStaticSecondarySpeechLine(9, (byte)_random.getRandomNumber(1));
		return true;
	case 11: // Cerrar closed masculine object: magnetófono/perfume/makeup/polish.
		beginStaticSecondarySpeechLine(0x0a, 0);
		return true;
	case 12: // Cerrar agenda/cartera (close notebook/wallet): already closed.
		beginStaticSecondarySpeechLine(0x0b, 0);
		return true;
	case 13: // Dar to uninterested target (give item): recipient would not want it.
		beginStaticSecondarySpeechLine(0x0c, (byte)_random.getRandomNumber(1));
		return true;
	case 14: // Dar owned item refusal: Sue wants to keep it.
		beginStaticSecondarySpeechLine(0x0d, (byte)_random.getRandomNumber(1));
		return true;
	case 15: // Generic immovable object.
		beginStaticSecondarySpeechLine(0x0e, 0);
		return true;
	case 16: // Hablar with object/non-responsive target (talk).
		beginStaticSecondarySpeechLine(0x0f, (byte)_random.getRandomNumber(2));
		return true;
	case 17: // Action requires an item Sue does not have yet.
		beginStaticSecondarySpeechLine(0x10, 0);
		return true;
	case 18: // Generic bad idea/refusal condition.
		beginStaticSecondarySpeechLine(0x11, (byte)_random.getRandomNumber(1));
		return true;
	case 19: // Sue pickup/useful-line slot.
		beginStaticSecondarySpeechLine(0x12, (byte)_random.getRandomNumber(2));
		return true;
	case 20: // Wrong time for this action.
		beginStaticSecondarySpeechLine(0x13, 0);
		return true;
	case 21: // Generic pickup/confirmation line.
		beginStaticSecondarySpeechLine(0x14, 0);
		return true;
	case 22: // Wrong-time/occasion line; not a pickup confirmation.
		beginStaticSecondarySpeechLine(0x15, 0);
		return true;
	case 23: // Generic no-reason/no-result action.
		beginStaticSecondarySpeechLine(0x16, (byte)_random.getRandomNumber(1));
		return true;
	case 24: // Generic unnecessary action.
		beginStaticSecondarySpeechLine(0x17, (byte)_random.getRandomNumber(1));
		return true;
	case 25: // Coger unavailable object (take object): cannot pick it up.
		beginStaticSecondarySpeechLine(0x18, (byte)_random.getRandomNumber(1));
		return true;
	case 26: // Usar magnetófono (use tape recorder): save tape for interviews.
		beginStaticSecondarySpeechLine(0x19, 0);
		return true;
	case 27: // Dar magnetófono (give tape recorder): Sue keeps it.
		beginStaticSecondarySpeechLine(0x1a, 0);
		return true;
	case 28: // Usar agenda (use notebook): no notes needed now.
		beginStaticSecondarySpeechLine(0x1b, 0);
		return true;
	case 29: // Dar agenda (give notebook): Sue keeps it.
		beginStaticSecondarySpeechLine(0x1c, 0);
		return true;
	case 30: // Usar/Dar cartera (use/give wallet): do not play with wallet.
		beginStaticSecondarySpeechLine(0x1d, 0);
		return true;
	case 31: // Mirar cartera (look at wallet): gift from Sue's father.
		beginStaticSecondarySpeechLine(0x1e, 0);
		return true;
	case 32: // Abrir cartera (open wallet): only documentation inside.
		beginStaticSecondarySpeechLine(0x1f, 0);
		return true;
	case 33: // Mirar/Abrir agenda (look/open notebook): no useful notes now.
		beginStaticSecondarySpeechLine(0x20, 0);
		return true;
	case 34: // Mirar florero (look at vase): empty.
		beginStaticSecondarySpeechLine(0x21, 0);
		return true;
	case 36: // Mirar invitación/pase de prensa (look at invitation/press pass).
		beginStaticSecondarySpeechLine(0x23, 0);
		return true;
	case 37: // Mirar hoja revelada (look at revealed Frankie note): read message.
		beginStaticSecondarySpeechLine(0x43, 1);
		beginStaticSecondarySpeechLine(0x24, 0);
		beginStaticSecondarySpeechLine(0x43, 2);
		return true;
	case 38: // Mirar magnetófono (look at tape recorder): enough tape left.
		beginStaticSecondarySpeechLine(0x25, 0);
		return true;
	case 39: // Mirar trapo con carbón (look at sooty rag): wrapped safely.
		beginStaticSecondarySpeechLine(0x26, 0);
		return true;
	case 40: // Mirar tarjeta (look at card): M.K.O./Otto J. Hannover.
		beginStaticSecondarySpeechLine(0x27, 0);
		return true;
	case 42: // Mirar hueso (look at bone): should return it to Húmero.
		beginStaticSecondarySpeechLine(0x29, 0);
		return true;
	case 43: // Mirar vaso vacío (look at empty glass).
		beginStaticSecondarySpeechLine(0x2a, 0);
		return true;
	case 44: // Mirar vaso con ponche (look at punch glass).
		beginStaticSecondarySpeechLine(0x2b, 0);
		return true;
	case 46: // Mirar hoja en blanco (look at blank sheet).
		beginStaticSecondarySpeechLine(0x2d, 0);
		return true;
	case 47: // Mirar trapo/gamuza (look at cloth).
		beginStaticSecondarySpeechLine(0x2e, 0);
		return true;
	case 48: // Mirar llave (look at key): Bruno will miss it.
		beginStaticSecondarySpeechLine(0x2f, 0);
		return true;
	case 50: // Mirar manivela (look at crank).
		beginStaticSecondarySpeechLine(0x31, 0);
		return true;
	case 51: // Mirar placa (look at plate/plaque): painted to blend into wall.
		beginStaticSecondarySpeechLine(0x32, 0);
		return true;
	case 52: // Mirar póster (look at poster): familiar face.
		beginStaticSecondarySpeechLine(0x33, 0);
		return true;
	case 53: // Mirar rata (look at rat): stunned after the hit.
		beginStaticSecondarySpeechLine(0x34, 0);
		return true;
	case 54: // Mirar bisturí (look at scalpel): Sue wonders where Ron got it.
		beginStaticSecondarySpeechLine(0x35, 0);
		return true;
	case 55: // Mirar pie de micro con algodón (look at mic stand with cotton).
		beginStaticSecondarySpeechLine(0x36, 0);
		return true;
	case 56: // Mirar pamela (look at hat): Sue keeps it.
		beginStaticSecondarySpeechLine(0x37, 0);
		return true;
	case 57: // Usar pamela (use hat): do not put it inside; it may stain.
		beginStaticSecondarySpeechLine(0x38, 0);
		return true;
	case 58: // Mirar navaja multiusos (look at multi-tool knife).
		beginStaticSecondarySpeechLine(0x39, 0);
		return true;
	case 59: // Mirar varita zahorí (look at divining rod).
		beginStaticSecondarySpeechLine(0x3a, 0);
		return true;
	case 60: // Mirar/Abrir frasco de perfume (look/open perfume bottle): empty.
		beginStaticSecondarySpeechLine(0x3b, 0);
		return true;
	case 61: // Mirar baraja de cartas (look at deck of cards).
		beginStaticSecondarySpeechLine(0x3c, 0);
		return true;
	case 62: // Mirar pinzas (look at tweezers): not for eyebrows.
		beginStaticSecondarySpeechLine(0x3d, 0);
		return true;
	case 63: // Mirar lupa (look at magnifying glass): smells of perfume.
		beginStaticSecondarySpeechLine(0x3e, 0);
		return true;
	case 64: // Mirar bote de pintura (look at black paint can).
		beginStaticSecondarySpeechLine(0x3f, 0);
		return true;
	case 65: // Mirar maletín de maquillaje (look at makeup case).
		beginStaticSecondarySpeechLine(0x40, 0);
		return true;
	case 66: // Abrir maletín de maquillaje: find pintauñas multicolor if missing.
		if (hasInventoryItem(0x22)) {
			beginStaticSecondarySpeechLine(0x41, 1);
			return true;
		}
		addInventoryItem(0x22);
		_soundBank0.playSample(1, 100);
		beginStaticSecondarySpeechLine(0x41, 0);
		return true;
	case 67: // Mirar pintauñas multicolor (look at multicolor nail polish).
		beginStaticSecondarySpeechLine(0x42, 0);
		return true;
	case 68: // Usar trapo con carbón + hoja: reveal Frankie's note.
		beginStaticSecondarySpeechLine(0x43, 0);
		removeInventoryItem(0x08);
		removeInventoryItem(0x0f);
		addInventoryItem(0x06);
		_soundBank0.playSample(1, 100);
		beginStaticSecondarySpeechLine(0x43, 1);
		beginStaticSecondarySpeechLine(0x24, 0);
		beginStaticSecondarySpeechLine(0x43, 2);
		return true;
	case 69: // Door/lock condition: no key needed, it is not locked.
		beginStaticSecondarySpeechLine(0x44, 0);
		return true;
	case 224: // That item cannot be used here.
		beginStaticSecondarySpeechLine(0xd3, 0);
		return true;
	case 227: // Ron cannot take that object.
		beginStaticSecondarySpeechLine(0xd6, (byte)_random.getRandomNumber(1));
		return true;
	case 230: // The world's largest diamond cannot cut this glass.
		beginStaticSecondarySpeechLine(0xd9, 0);
		return true;
	case 231: // Generic item-on-room combination failure.
		beginStaticSecondarySpeechLine(0xda, (byte)_random.getRandomNumber(1));
		return true;
	default:
		return false;
	}
}

} // End of namespace Hollywood
