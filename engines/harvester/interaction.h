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

#ifndef HARVESTER_INTERACTION_H
#define HARVESTER_INTERACTION_H

#include "common/error.h"
#include "harvester/bitmap.h"
#include "harvester/room_support.h"
#include "harvester/saveload.h"

namespace Harvester {

class Flow;
class HarvesterEngine;
class InventorySystem;
class Interaction;
class SceneEntities;
struct IndexedBitmap;

extern const char *const kExitCloseupPendingRoomChange;

class InteractionCallbacks {
public:
	virtual ~InteractionCallbacks() {}

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
	virtual void notePlayerActivity() = 0;
	virtual void stopPlayerRegionInteraction() = 0;
	virtual void startPlayerDefeatSequence(const char *reason, const Common::String &sourceName,
		int damageType) = 0;
};

class ItemInteractions {
public:
	ItemInteractions(HarvesterEngine &engine, SceneState &scene,
		SceneEntities &sceneRuntime, InventorySystem &inventory, const Common::String &pendingRoomChange);

	const Common::String &getCarriedRoomItemLabel() const { return _carriedRoomItemLabel; }
	const IndexedBitmap &getCarriedRoomItemBitmap() const { return _carriedRoomItemBitmap; }
	bool hasCarriedRoomItem() const;
	bool stowCarriedRoomItemToInventory();
	Common::Error handleInventoryTargetInteraction(const ObjectRecord &target,
		Interaction &interactionProcessor, bool &needsRedraw);
	ObjectRecord *findSelectedInventoryRoomTarget(const Common::Point &point);
	Common::Error handleRoomItemPickup(const ObjectRecord &object, bool playerPresent,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw);

private:
	void clearCarriedRoomItem();
	void hideSceneObject(const Common::String &objectName, const Common::String *ownerOrRoom);
	bool shouldCancelDeniedPickup(const InteractionResult &interaction, bool didTransition) const;
	Common::Error restoreDeniedPickup(Script &script, const ObjectRecord &savedObject, bool clearCarryState,
		InteractionCallbacks &callbacks, bool &needsRedraw);
	Common::Error moveRoomItemDirectlyToInventory(const ObjectRecord &object,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw);
	Common::Error beginRoomItemCarry(const ObjectRecord &object,
		Interaction &interactionProcessor, InteractionCallbacks &callbacks, bool &needsRedraw);

	HarvesterEngine &_engine;
	SceneState &_scene;
	SceneEntities &_sceneRuntime;
	InventorySystem &_inventory;
	const Common::String &_pendingRoomChange;
	Common::String _carriedRoomItemName;
	Common::String _carriedRoomItemLabel;
	IndexedBitmap _carriedRoomItemBitmap;
};

class Interaction {
public:
	Interaction(HarvesterEngine &engine, Flow &flow, SceneState &scene,
		RoomPlayerState &playerState, Common::String &pendingRegionName,
		Common::String &pendingRoomChange, bool &pendingRoomChangeIsRoomName,
		bool &pendingRoomChangeUsesSavedRoomState, SaveRoomState &pendingRoomChangeSavedRoomState,
		bool canExitCloseupToParent, InteractionCallbacks &callbacks);

	void queueRegionInteraction(const RegionRecord &region, const Common::Point &mousePos);
	Common::Error runRegionInteraction(const RegionRecord &region,
		InventorySystem &inventory, bool &needsRedraw);
	Common::Error tryActivatePendingRegion(InventorySystem &inventory, bool &needsRedraw);
	Common::Error tryActivatePassiveRegion(InventorySystem &inventory, bool &needsRedraw);
	int getPendingRegionFacing() const;

	bool hasPendingMainMenuReturn() const;
	Common::Error handleCombatInteraction(const InteractionResult &interaction, bool &needsRedraw);
	Common::Error handleInteractionResult(const InteractionResult &interaction,
		bool &didTransition, const Common::String &usedItemName, bool interruptPlayerMovement = true);
	Common::Error runScriptedDialogue(const Common::String &npcName, const Common::String &usedItemName,
		const Common::String &continuationTag, bool &didTransition);

private:
	HarvesterEngine &_engine;
	Flow &_flow;
	SceneState &_scene;
	RoomPlayerState &_playerState;
	Common::String &_pendingRegionName;
	Common::String &_pendingRoomChange;
	bool &_pendingRoomChangeIsRoomName;
	bool &_pendingRoomChangeUsesSavedRoomState;
	SaveRoomState &_pendingRoomChangeSavedRoomState;
	bool _canExitCloseupToParent;
	InteractionCallbacks &_callbacks;
};

bool hasRoomEntryInteraction(const InteractionResult &interaction);

} // End of namespace Harvester

#endif // HARVESTER_INTERACTION_H
