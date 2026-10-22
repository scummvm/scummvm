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
#include "harvester/cheats.h"
#include "harvester/dialogue/dialogue.h"
#include "harvester/inventory.h"
#include "harvester/menu/menu.h"
#include "harvester/scene/scene.h"
#include "harvester/script/script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;
struct IndexedBitmap;

class Flow {
public:
	explicit Flow(HarvesterEngine &engine);

	bool load();
	bool buildDialogueSaveStateBlob(Common::Array<byte> &blob, uint32 saveVersion);
	bool loadDialogueSaveStateBlob(const Common::Array<byte> &blob, uint32 saveVersion);
	Common::Error run();
	Common::Error showCdChangePrompt(int discNumber);
	bool handleSystemEvent(const Common::Event &event, Common::Error &result);

	const MenuTextConfig &getMenuTextConfig() const { return _menuTextConfig; }
	const Common::Point &getMousePos() const { return _mousePos; }
	uint getQuickTipCount() const { return _quickTips.size(); }
	const Common::String &getQuickTip(uint index) const { return _quickTips[index]; }
	bool hasRoomNpcHandler(const Common::String &npcName) const { return _dialogue.hasRoomNpcHandler(npcName); }
	bool isNestedRoomLoop() const { return _roomLoopDepth > 1; }
	RoomHoverState getRoomHoverState(const SceneState &scene) const;
	void logStartupRoomProbe(const SceneState &scene, const Common::String &entranceName);

	void executeStartupAudioCommands(const Common::Array<AudioCommand> &commands);
	void queueDialogueInteraction(const InteractionResult &interaction);
	void requestGameOverReturn();

	bool hasQueuedDebugInteraction() const { return _hasQueuedDebugInteraction; }
	bool queueDebugInteraction(const InteractionResult &interaction);
	bool requestDebugRoomChange(const Common::String &roomName, int discNumber);

	Common::Error runDemoEnding();
	Common::Error runHelpScreen(const byte *palette, float paletteBrightness);
	Common::Error runRoomMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, bool canSaveGame);
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const ScriptExecutionContext &executionContext,
		const NpcRecord &npc, const Common::String &usedItemName);
	Common::Error runRoomLoop(const Common::String &entranceName);
	Common::Error resolveRoomTransitionTarget(const Common::String &targetName, Common::String &resolvedTargetName);

	bool populateRoomSceneEntities(RoomSetupState &state,
		const Common::Array<ObjectRecord> &drawableObjects,
		const Common::Array<AnimRecord> &drawableAnimations);
	Common::Error beginRoomSetupTransition();
	Common::Error waitForRoomSetupTransitionHold();
	Common::Error fadeInRoomScene(const byte *palette, float targetBrightness);
	void resetCursorAnimationSequence();
	bool tickRuntimeEntities();

	bool takeQueuedDialogueInteraction(InteractionResult &interaction);
	bool takeQueuedDebugInteraction(InteractionResult &interaction);

	void prepareForNewGame();
	void requestNewGameRestart();
	bool takePendingNewGameRestart();
	bool takePendingGameOverReturn();
	void requestMainMenuReturn();
	bool hasPendingMainMenuReturn() const;
	bool takePendingMainMenuReturn();
	void requestCloseupParentRestart();
	bool takePendingCloseupParentRestart();
	bool hasPendingDebugRoomChange() const { return !_pendingDebugRoomName.empty(); }
	void resetRoomNpcDialogueState();

private:
	bool loadQuickTips();
	bool loadMenuItems();
	Common::Error runQuickTips();
	Common::Error runQuickTipsOverlay(const SceneState &scene);
	Common::Error runDemoIntroduction();
	Common::Error runMainMenu();
	Common::Error runTownMapSelector(const Common::String &mapEntryName, Common::String &destinationEntranceName);
	bool ensureCursorEntity();
	Common::Error fadePalette(const byte *palette, float fromBrightness, float toBrightness);
	bool waitForPaletteFadeTick(uint32 startTick, Common::Error &result);
	bool pumpTransitionEvents(Common::Error &result);
	bool hasPendingNewGameRestart() const;
	void clearPendingNewGameRestart();
	bool hasPendingGameOverReturn() const;
	void clearPendingGameOverReturn();
	void clearPendingMainMenuReturn();
	bool hasPendingCloseupParentRestart() const;
	void clearPendingCloseupParentRestart();
	Common::String takePendingDebugRoomChange(int &discNumber);

	HarvesterEngine &_engine;
	Common::Array<Common::String> _quickTips;
	Common::Array<Common::String> _menuItems;
	MenuTextConfig _menuTextConfig;
	Common::Point _mousePos;
	DialogueSystem _dialogue;
	InventorySystem _inventory;
	MenuSystem _menu;
	CheatSystem _cheats;
	SceneRunner _room;
	InteractionResult _queuedDialogueInteraction;
	bool _hasQueuedDialogueInteraction = false;
	InteractionResult _queuedDebugInteraction;
	bool _hasQueuedDebugInteraction = false;
	bool _pendingNewGameRestart = false;
	bool _pendingGameOverReturn = false;
	bool _pendingMainMenuReturn = false;
	bool _pendingCloseupParentRestart = false;
	Common::String _pendingDebugRoomName;
	int _pendingDebugRoomDisc = 0;
	uint _roomLoopDepth = 0;
	uint32 _roomSetupTransitionShownTick = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_FLOW_H
