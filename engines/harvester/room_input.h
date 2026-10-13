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

#ifndef HARVESTER_ROOM_INPUT_H
#define HARVESTER_ROOM_INPUT_H

#include "common/error.h"
#include "common/events.h"

namespace Harvester {

class Art;
class CheatSystem;
class Flow;
class HarvesterEngine;
class InventorySystem;
class RoomInteractionProcessor;
class RoomItemInteractions;
class RoomSession;
struct ObjectRecord;
struct ResolvedText;
struct RoomPresentationState;
struct RoomSceneResources;

struct RoomInputState {
	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool attackModifierHeld = false;
	uint32 lastLeftButtonReleaseTick = 0;
	uint32 nextKeyboardAttackAllowedTick = 0;
};

// Enter and closeup exits return from runRoomLoop even when the error is kNoError.
struct RoomInputResult {
	Common::Error error;
	bool returnFromRoom = false;

	static RoomInputResult leaveRoom(const Common::Error &error) {
		RoomInputResult result;
		result.error = error;
		result.returnFromRoom = true;
		return result;
	}
};

class RoomInput {
public:
	RoomInput(HarvesterEngine &engine, Flow &flow, RoomSession &session, RoomSceneResources &scene,
		InventorySystem &inventory, CheatSystem &cheats,
		RoomItemInteractions &roomItems, RoomInteractionProcessor &interactionProcessor,
		RoomPresentationState &presentationState,
		const Art &art, bool canExitCloseupToParent);

	RoomInputResult handleEvent(const Common::Event &event, bool &needsRedraw);

private:
	void handleMouseMove(bool &needsRedraw);
	RoomInputResult handleRightButtonDown(bool &needsRedraw);
	void handleLeftButtonUp();
	RoomInputResult handleLeftButtonDown(bool &needsRedraw);
	RoomInputResult handleKeyDown(const Common::KeyState &key, bool &needsRedraw);
	void handleKeyUp(const Common::KeyState &key);
	void beginInspectText(const ObjectRecord &object, const ResolvedText &text);
	void queueCombatLoadoutStatusMessage(const Common::String &message, bool &needsRedraw);
	bool openInventoryOverlay();
	void capturePlayerAttackTarget();

	HarvesterEngine &_engine;
	Flow &_flow;
	RoomSession &_session;
	RoomSceneResources &_scene;
	InventorySystem &_inventory;
	CheatSystem &_cheats;
	RoomItemInteractions &_roomItems;
	RoomInteractionProcessor &_interactionProcessor;
	RoomPresentationState &_presentationState;
	const Art &_art;
	bool _canExitCloseupToParent;
};

} // End of namespace Harvester

#endif // HARVESTER_ROOM_INPUT_H
