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

#ifndef HARVESTER_FLOW_H
#define HARVESTER_FLOW_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/dialogue.h"
#include "harvester/inventory.h"
#include "harvester/menu.h"
#include "harvester/room.h"
#include "harvester/script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;
class DialogueFlowAccess;
class RoomInteractionProcessor;
struct IndexedBitmap;

class Flow {
public:
	explicit Flow(HarvesterEngine &engine);

	bool load();
	bool buildDialogueSaveStateBlob(Common::Array<byte> &blob, uint32 saveVersion);
	bool loadDialogueSaveStateBlob(const Common::Array<byte> &blob, uint32 saveVersion);
	Common::Error run();

private:
	friend class HarvesterEngine;
	friend class DialogueFlowAccess;
	friend class DialogueSystem;
	friend class InventorySystem;
	friend class MenuSystem;
	friend class RoomInteractionProcessor;
	friend class RoomSystem;

	bool loadQuickTips();
	bool loadMenuItems();
	Common::Error runQuickTips();
	Common::Error runMainMenuStub();
	Common::Error runRoomMenuStub(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, bool canSaveGame);
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const NpcRecord &npc, const Common::String &usedItemName);
	Common::Error runTownMapSelector(const Common::String &mapEntryName, Common::String &destinationEntranceName);
	Common::Error runRoomLoop(const Common::String &entranceName);
	Common::Error resolveRoomTransitionTarget(const Common::String &targetName, Common::String &resolvedTargetName);
	bool ensureCursorEntity();
	bool populateRoomSceneEntities(RoomSetupState &state,
		const Common::Array<ObjectRecord> &drawableObjects,
		const Common::Array<AnimRecord> &drawableAnimations);
	Common::Error beginRoomSetupTransition();
	Common::Error waitForRoomSetupTransitionHold();
	Common::Error fadeInRoomScene(const byte *palette, float targetBrightness);
	bool pumpTransitionEvents(Common::Error &result);
	void executeStartupAudioCommands(const Common::Array<AudioCommand> &commands);
	void queueDialogueInteraction(const InteractionResult &interaction);
	bool takeQueuedDialogueInteraction(InteractionResult &interaction);
	bool hasQueuedDebugInteraction() const { return _hasQueuedDebugInteraction; }
	bool queueDebugInteraction(const InteractionResult &interaction);
	bool takeQueuedDebugInteraction(InteractionResult &interaction);
	void prepareForNewGame();
	void requestNewGameRestart();
	bool hasPendingNewGameRestart() const;
	bool takePendingNewGameRestart();
	void clearPendingNewGameRestart();
	void requestGameOverReturn();
	bool hasPendingGameOverReturn() const;
	bool takePendingGameOverReturn();
	void clearPendingGameOverReturn();
	void requestMainMenuReturn();
	bool hasPendingMainMenuReturn() const;
	bool takePendingMainMenuReturn();
	void clearPendingMainMenuReturn();
	void requestCloseupParentRestart();
	bool hasPendingCloseupParentRestart() const;
	bool takePendingCloseupParentRestart();
	void clearPendingCloseupParentRestart();
	bool requestDebugRoomChange(const Common::String &roomName);
	bool hasPendingDebugRoomChange() const { return !_pendingDebugRoomName.empty(); }
	Common::String takePendingDebugRoomChange();
	void resetRoomNpcDialogueState();
	void resetCursorAnimationSequence();
	bool tickRuntimeEntities();
	bool handleSystemEvent(const Common::Event &event, Common::Error &result);

	HarvesterEngine &_engine;
	Common::Array<Common::String> _quickTips;
	Common::Array<Common::String> _menuItems;
	Common::Point _mousePos;
	DialogueSystem _dialogue;
	InventorySystem _inventory;
	MenuSystem _menu;
	RoomSystem _room;
	InteractionResult _queuedDialogueInteraction;
	bool _hasQueuedDialogueInteraction = false;
	InteractionResult _queuedDebugInteraction;
	bool _hasQueuedDebugInteraction = false;
	bool _pendingNewGameRestart = false;
	bool _pendingGameOverReturn = false;
	bool _pendingMainMenuReturn = false;
	bool _pendingCloseupParentRestart = false;
	Common::String _pendingDebugRoomName;
	uint _roomLoopDepth = 0;
	uint32 _roomSetupTransitionShownTick = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_FLOW_H
