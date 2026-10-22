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

#ifndef HARVESTER_INPUT_H
#define HARVESTER_INPUT_H

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/rect.h"
#include "common/str.h"

namespace Harvester {

class Art;
class CheatSystem;
class DialogueSystem;
class Entity;
class Flow;
class HarvesterEngine;
class InventorySystem;
class Interaction;
class ItemInteractions;
class Scene;
struct CheatActivation;
struct InteractionResult;
struct InventorySecondaryAction;
struct MenuTextConfig;
struct NpcRecord;
struct ObjectRecord;
struct RegionRecord;
struct ResolvedText;
struct RoomHoverState;
struct RoomPresentationState;
struct RoomSetupState;
struct SceneState;

enum GameAction {
	kActionMoveLeft,
	kActionMoveRight,
	kActionMoveUp,
	kActionMoveDown,
	kActionAttackModifier,
	kActionEvasiveModifier,
	kActionSpeedUp,
	kActionInventory,
	kActionMenu,
	kActionHelp,
	kActionHighAttack,
	kActionMediumAttack,
	kActionLowAttack,
	kActionDuck,
	kActionJump
};

struct InputState {
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool attackModifierHeld = false;
	bool evasiveModifierHeld = false;
	bool speedUpActive = false;
	bool highAttackHeld = false;
	bool mediumAttackHeld = false;
	bool lowAttackHeld = false;
	bool duckHeld = false;
	bool jumpHeld = false;
	uint32 lastLeftButtonReleaseTick = 0;
	uint32 nextKeyboardAttackAllowedTick = 0;

	void clearHeldActions() {
		moveLeft = moveRight = moveUp = moveDown = false;
		attackModifierHeld = evasiveModifierHeld = speedUpActive = false;
		highAttackHeld = mediumAttackHeld = lowAttackHeld = duckHeld = jumpHeld = false;
	}
};

// Enter and closeup exits return from runRoomLoop even when the error is kNoError.
struct InputResult {
	Common::Error error;
	bool returnFromRoom = false;

	static InputResult leaveRoom(const Common::Error &error) {
		InputResult result;
		result.error = error;
		result.returnFromRoom = true;
		return result;
	}
};

class Input {
public:
	Input(HarvesterEngine &engine, Flow &flow, Scene &session, SceneState &scene,
		InventorySystem &inventory, CheatSystem &cheats,
		ItemInteractions &roomItems, Interaction &interactionProcessor,
		RoomPresentationState &presentationState,
		const Art &art, bool canExitCloseupToParent);

	InputResult handleEvent(const Common::Event &event, bool &needsRedraw);

	static const ObjectRecord *findRoomObjectAtPoint(HarvesterEngine &engine,
		const Common::Array<ObjectRecord> &sceneObjects, const Common::Point &point);
	static RoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const RoomSetupState &state,
		const Common::Array<ObjectRecord> &sceneObjects,
		const Common::Array<NpcRecord> &npcs,
		const Common::Array<RegionRecord> &regions,
		const Common::Point &mousePos, const MenuTextConfig &menuTextConfig,
		const DialogueSystem *dialogue = nullptr);

private:
	void handleMouseMove(bool &needsRedraw);
	InputResult handleRightButtonDown(bool &needsRedraw);
	void handleLeftButtonUp();
	InputResult handleLeftButtonDown(bool &needsRedraw);
	InputResult handleSceneClick(bool isFastExitClick, bool &needsRedraw);
	InputResult handleSelectedItemClick(const RoomHoverState &clickHoverState,
		const Entity *itemTarget, bool &needsRedraw);
	InputResult handleSceneTargetClick(const RoomHoverState &hoverState,
		bool isFastExitClick, bool &needsRedraw);
	InputResult handleWalkClick(const RoomHoverState &hoverState, bool &needsRedraw);
	InputResult handleObjectClick(ObjectRecord &object, bool &needsRedraw);
	InputResult handleInventoryLeftClick(bool &needsRedraw);
	InputResult handleInventoryRightClick(bool &needsRedraw);
	InputResult applyInventorySecondaryAction(const ObjectRecord &object,
		const InventorySecondaryAction &secondaryAction, bool &needsRedraw);
	InputResult handleInventoryKeyDown(const Common::KeyState &key, bool &needsRedraw);
	InputResult handleKeyDown(const Common::KeyState &key, bool &needsRedraw, bool gameAction = false);
	InputResult handleGameplayKeyDown(const Common::KeyState &key, bool &needsRedraw, bool gameAction);
	void handleKeyUp(const Common::KeyState &key, bool gameAction = false);
	InputResult handleGameAction(GameAction action, bool pressed, bool &needsRedraw);
	InputResult applyCheatActivation(InteractionResult &cheatInteraction,
		const CheatActivation &cheatActivation, bool &needsRedraw);
	InputResult openRoomMenu(bool &needsRedraw);
	InputResult openHelpScreen(bool &needsRedraw);
	void beginInspectText(const ObjectRecord &object, const ResolvedText &text);
	InputResult finishInspectText(bool &needsRedraw);
	InputResult applyObjectInteraction(InteractionResult &interaction, bool &needsRedraw);
	void queueCombatLoadoutStatusMessage(const Common::String &message, bool &needsRedraw);
	bool openInventoryOverlay();
	void capturePlayerAttackTarget();

	HarvesterEngine &_engine;
	Flow &_flow;
	Scene &_session;
	SceneState &_scene;
	InventorySystem &_inventory;
	CheatSystem &_cheats;
	ItemInteractions &_roomItems;
	Interaction &_interactionProcessor;
	RoomPresentationState &_presentationState;
	const Art &_art;
	bool _canExitCloseupToParent;
	Common::String _pendingInspectObjectName;
	bool _inspectDismissPressed = false;
};

} // End of namespace Harvester

#endif // HARVESTER_INPUT_H
