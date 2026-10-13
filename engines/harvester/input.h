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

#include "common/error.h"
#include "common/events.h"

namespace Harvester {

class Art;
class CheatSystem;
class Flow;
class HarvesterEngine;
class InventorySystem;
class Interaction;
class ItemInteractions;
class Scene;
struct ObjectRecord;
struct ResolvedText;
struct RoomPresentationState;
struct SceneState;

struct InputState {
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool attackModifierHeld = false;
	uint32 lastLeftButtonReleaseTick = 0;
	uint32 nextKeyboardAttackAllowedTick = 0;
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

private:
	void handleMouseMove(bool &needsRedraw);
	InputResult handleRightButtonDown(bool &needsRedraw);
	void handleLeftButtonUp();
	InputResult handleLeftButtonDown(bool &needsRedraw);
	InputResult handleKeyDown(const Common::KeyState &key, bool &needsRedraw);
	void handleKeyUp(const Common::KeyState &key);
	void beginInspectText(const ObjectRecord &object, const ResolvedText &text);
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
};

} // End of namespace Harvester

#endif // HARVESTER_INPUT_H
