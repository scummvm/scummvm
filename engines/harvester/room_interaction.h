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

#ifndef HARVESTER_ROOM_INTERACTION_H
#define HARVESTER_ROOM_INTERACTION_H

#include "common/error.h"
#include "harvester/room_support.h"
#include "harvester/saveload.h"

namespace Harvester {

class Flow;
class HarvesterEngine;
struct IndexedBitmap;

extern const char *const kExitCloseupPendingRoomChange;

class RoomInteractionCallbacks {
public:
	virtual ~RoomInteractionCallbacks() {}

	virtual bool refreshCurrentScene(bool preservePlayerPlacement) = 0;
	virtual bool applyCurrentRoomRuntimeMutationsInPlace(bool preservePaletteState) = 0;
	virtual void syncGlobalTimerEntities(const Common::Array<TimerRecord> &previousTimerRecords) = 0;
	virtual bool captureDialogueBackdrop(IndexedBitmap &dialogueBackdrop) = 0;
	virtual Common::Error showCdChangePrompt(int discNumber) = 0;
	virtual Common::Error runRoomExitCommands() = 0;
	virtual Common::Error applyLightingCommand(StartupLightingCommand lightingCommand) = 0;
	virtual void applyPlayerGotoXZ(int x, int z) = 0;
	virtual Common::Error runModalShowText(const ResolvedText &modalText) = 0;
	virtual void resetIdleState() = 0;
	virtual void stopPlayerRegionInteraction() = 0;
	virtual void startPlayerDefeatSequence(const char *reason, const Common::String &sourceName,
		int damageType) = 0;
};

class RoomInteractionProcessor {
public:
	RoomInteractionProcessor(HarvesterEngine &engine, Flow &flow, RoomSceneResources &scene,
		RoomPlayerState &playerState, Common::String &pendingRegionName,
		Common::String &pendingRoomChange, bool &pendingRoomChangeIsRoomName,
		bool &pendingRoomChangeUsesSavedRoomState, SaveRoomState &pendingRoomChangeSavedRoomState,
		bool canExitCloseupToParent, RoomInteractionCallbacks &callbacks);

	Common::Error handleInteractionResult(const InteractionResult &interaction,
		bool &didTransition, const Common::String &usedItemName);
	Common::Error runScriptedDialogue(const Common::String &npcName, const Common::String &usedItemName,
		const Common::String &continuationTag, bool &didTransition);

private:
	HarvesterEngine &_engine;
	Flow &_flow;
	RoomSceneResources &_scene;
	RoomPlayerState &_playerState;
	Common::String &_pendingRegionName;
	Common::String &_pendingRoomChange;
	bool &_pendingRoomChangeIsRoomName;
	bool &_pendingRoomChangeUsesSavedRoomState;
	SaveRoomState &_pendingRoomChangeSavedRoomState;
	bool _canExitCloseupToParent;
	RoomInteractionCallbacks &_callbacks;
};

bool hasRoomEntryInteraction(const InteractionResult &interaction);

} // End of namespace Harvester

#endif // HARVESTER_ROOM_INTERACTION_H
