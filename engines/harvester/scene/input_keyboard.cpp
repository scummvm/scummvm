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

#include "graphics/screen.h"
#include "harvester/cheats.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/inventory.h"
#include "harvester/player/player.h"
#include "harvester/scene/interaction.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/scene/room_support.h"
#include "harvester/scene/scene.h"

namespace Harvester {

InputResult Input::handleGameAction(GameAction action, bool pressed, bool &needsRedraw) {
	InputState &inputState = _session.getInputState();
	bool *heldAction = nullptr;
	switch (action) {
	case kActionHighAttack:
		heldAction = &inputState.highAttackHeld;
		break;
	case kActionMediumAttack:
		heldAction = &inputState.mediumAttackHeld;
		break;
	case kActionLowAttack:
		heldAction = &inputState.lowAttackHeld;
		break;
	case kActionDuck:
		heldAction = &inputState.duckHeld;
		break;
	case kActionJump:
		heldAction = &inputState.jumpHeld;
		break;
	default:
		break;
	}
	if (heldAction) {
		inputState.speedUpActive = false;
		if (!pressed) {
			*heldAction = false;
			return InputResult();
		}
		_session.notePlayerActivity();
		if (_presentationState.showingInspectText || _inventory.isOpen())
			return InputResult();
		Player &player = _session.getPlayer();
		const RoomIdleAnimationState &idleState = player.getIdleState();
		if (idleState.active || idleState.exiting) {
			if (player.requestIdleAnimationExit(_scene.state))
				needsRedraw = true;
			return InputResult();
		}
		*heldAction = true;
		return InputResult();
	}
	static const Common::KeyCode keys[] = {
		Common::KEYCODE_LEFT, Common::KEYCODE_RIGHT, Common::KEYCODE_UP, Common::KEYCODE_DOWN,
		Common::KEYCODE_LCTRL, Common::KEYCODE_RSHIFT, Common::KEYCODE_LALT,
		Common::KEYCODE_i, Common::KEYCODE_ESCAPE, Common::KEYCODE_F1
	};
	if ((uint)action >= ARRAYSIZE(keys))
		return InputResult();
	const Common::KeyState key(keys[action]);
	if (pressed)
		return handleKeyDown(key, needsRedraw, true);
	handleKeyUp(key, true);
	return InputResult();
}

InputResult Input::handleKeyDown(const Common::KeyState &key, bool &needsRedraw, bool gameAction) {
	Player &player = _session.getPlayer();
	const RoomIdleAnimationState &idleState = player.getIdleState();
	InputState &inputState = _session.getInputState();
	// GOG update_actor_runtime_state (0x4d750) tests the last scan code,
	// not the held Alt slot. Any other key press or release clears fast speed.
	inputState.speedUpActive = gameAction && key.keycode == Common::KEYCODE_LALT;

	if (_presentationState.showingInspectText) {
		if (key.keycode == Common::KEYCODE_ESCAPE)
			_inspectDismissPressed = true;
		return InputResult();
	}

	_session.notePlayerActivity();
	if (idleState.active || idleState.exiting) {
		if (player.requestIdleAnimationExit(_scene.state))
			needsRedraw = true;
		return InputResult();
	}

	if (Script *script = _engine.getScript()) {
		InteractionResult cheatInteraction;
		CheatActivation cheatActivation;
		const CheatInputResult cheatInputResult = _cheats.processKey(
			key, *script, _scene.executionContext(),
			cheatInteraction, cheatActivation);
		if (cheatInputResult != kCheatInputIgnored) {
			if (cheatInputResult == kCheatInputComplete && cheatActivation.activated)
				return applyCheatActivation(cheatInteraction, cheatActivation, needsRedraw);
			return InputResult();
		}
	}

	if (_inventory.isOpen())
		return handleInventoryKeyDown(key, needsRedraw);

	if (_inventory.hasSelection()) {
		if (key.keycode == Common::KEYCODE_ESCAPE) {
			if (_inventory.clearSelection())
				needsRedraw = true;
			return InputResult();
		}
		if (key.keycode == Common::KEYCODE_i) {
			if (_inventory.clearSelection())
				needsRedraw = true;
			if (!openInventoryOverlay())
				return InputResult::leaveRoom(Common::kReadingFailed);
			needsRedraw = true;
			return InputResult();
		}
	}
	return handleGameplayKeyDown(key, needsRedraw, gameAction);
}

InputResult Input::handleGameplayKeyDown(const Common::KeyState &key,
		bool &needsRedraw, bool gameAction) {
	InputState &inputState = _session.getInputState();

	if (gameAction && key.keycode == Common::KEYCODE_LCTRL) {
		inputState.attackModifierHeld = true;
	} else if (gameAction && key.keycode == Common::KEYCODE_RSHIFT) {
		inputState.evasiveModifierHeld = true;
	} else if (gameAction && key.keycode == Common::KEYCODE_LEFT)
		inputState.moveLeft = true;
	else if (gameAction && key.keycode == Common::KEYCODE_RIGHT)
		inputState.moveRight = true;
	else if (gameAction && key.keycode == Common::KEYCODE_UP)
		inputState.moveUp = true;
	else if (gameAction && key.keycode == Common::KEYCODE_DOWN)
		inputState.moveDown = true;

	if (gameAction && key.keycode == Common::KEYCODE_F1)
		return openHelpScreen(needsRedraw);

	if (_session.isPlayerCombatLocked())
		return InputResult();

	if (key.keycode == Common::KEYCODE_k && _engine.isCombatDebugEnabled()) {
		Common::Error debugKillError = _session.killDebugActiveMonster(_flow.getMousePos(), needsRedraw);
		if (debugKillError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(debugKillError);
		return InputResult();
	} else if (gameAction && key.keycode == Common::KEYCODE_i) {
		if (!openInventoryOverlay())
			return InputResult::leaveRoom(Common::kReadingFailed);
		needsRedraw = true;
		return InputResult();
	} else if (gameAction && key.keycode == Common::KEYCODE_ESCAPE) {
		return openRoomMenu(needsRedraw);
	}

	if (key.keycode == Common::KEYCODE_RETURN ||
		key.keycode == Common::KEYCODE_KP_ENTER) {
		if (!_roomItems.stowCarriedRoomItemToInventory())
			return InputResult::leaveRoom(Common::kReadingFailed);
		Common::Error exitError = _session.runRoomExitCommands();
		if (exitError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(exitError);
		return InputResult::leaveRoom(Common::kNoError);
	}
	return InputResult();
}

void Input::handleKeyUp(const Common::KeyState &key, bool gameAction) {
	InputState &inputState = _session.getInputState();
	inputState.speedUpActive = false;
	if (!gameAction)
		return;

	if (key.keycode == Common::KEYCODE_LCTRL ||
			key.keycode == Common::KEYCODE_RCTRL)
		inputState.attackModifierHeld = false;
	else if (key.keycode == Common::KEYCODE_RSHIFT)
		inputState.evasiveModifierHeld = false;
	else if (key.keycode == Common::KEYCODE_LEFT)
		inputState.moveLeft = false;
	else if (key.keycode == Common::KEYCODE_RIGHT)
		inputState.moveRight = false;
	else if (key.keycode == Common::KEYCODE_UP)
		inputState.moveUp = false;
	else if (key.keycode == Common::KEYCODE_DOWN)
		inputState.moveDown = false;
}

InputResult Input::applyCheatActivation(InteractionResult &cheatInteraction,
		const CheatActivation &cheatActivation, bool &needsRedraw) {
	if (cheatActivation.clearInteractionState && _inventory.clearSelection())
		needsRedraw = true;
	if (cheatActivation.inventoryChanged && _inventory.isOpen() &&
			!_inventory.refresh()) {
		return InputResult::leaveRoom(Common::kReadingFailed);
	}
	if (cheatActivation.hasInteraction) {
		if (_inventory.isOpen())
			(void)_inventory.close();
		bool didTransition = false;
		Common::Error cheatError = _interactionProcessor.handleInteractionResult(
			cheatInteraction, didTransition, Common::String());
		if (cheatError.getCode() != Common::kNoError)
			return InputResult::leaveRoom(cheatError);
		if (_flow.hasPendingMainMenuReturn())
			return InputResult::leaveRoom(Common::kNoError);
	}
	needsRedraw = true;
	return InputResult();
}

InputResult Input::openHelpScreen(bool &needsRedraw) {
	Common::Error helpError = _flow.runHelpScreen(_scene.palette, _scene.targetPaletteBrightness);
	_session.getInputState().clearHeldActions();
	if (helpError.getCode() != Common::kNoError)
		return InputResult::leaveRoom(helpError);
	// Stow the carried item after help returns.
	if (!_roomItems.stowCarriedRoomItemToInventory())
		return InputResult::leaveRoom(Common::kReadingFailed);
	_inventory.clearSelection();
	needsRedraw = true;
	return InputResult();
}

InputResult Input::openRoomMenu(bool &needsRedraw) {
	Player &player = _session.getPlayer();
	InputState &inputState = _session.getInputState();

	if (_session.cancelPlayerAttackAnimation())
		needsRedraw = true;
	if (!_roomItems.stowCarriedRoomItemToInventory())
		return InputResult::leaveRoom(Common::kReadingFailed);
	inputState.clearHeldActions();
	_session.clearPendingRegionInteraction();
	player.cancelMovement();
	_session.captureCurrentSaveState();
	Graphics::Screen *activeScreen = _engine.getScreen();
	if (!activeScreen)
		return InputResult::leaveRoom(Common::kNoError);
	IndexedBitmap roomMenuBackdrop;
	RoomPresentation::drawRoomScene(_engine, *activeScreen, _scene, _scene.targetPaletteBrightness);
	if (!captureScreenBackdrop(*activeScreen, roomMenuBackdrop))
		return InputResult::leaveRoom(Common::kReadingFailed);
	Common::Error menuError = _flow.runRoomMenu(
		roomMenuBackdrop, _scene.palette, _scene.targetPaletteBrightness,
		_scene.state.hasEntrance);
	if (menuError.getCode() != Common::kNoError)
		return InputResult::leaveRoom(menuError);
	_flow.resetCursorAnimationSequence();
	_session.resetIdleState();
	needsRedraw = true;
	return InputResult();
}

} // End of namespace Harvester
