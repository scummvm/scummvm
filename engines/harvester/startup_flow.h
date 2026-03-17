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

#ifndef HARVESTER_STARTUP_FLOW_H
#define HARVESTER_STARTUP_FLOW_H

#include "common/array.h"
#include "common/error.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/dialogue.h"
#include "harvester/startup_inventory.h"
#include "harvester/startup_menu.h"
#include "harvester/startup_room.h"
#include "harvester/startup_script.h"

namespace Common {
struct Event;
}

namespace Harvester {

class HarvesterEngine;
struct IndexedBitmap;

class StartupFlow {
public:
	explicit StartupFlow(HarvesterEngine &engine);

	bool load();
	Common::Error run();

private:
	friend class DialogueSystem;
	friend class StartupInventorySystem;
	friend class StartupMenuSystem;
	friend class StartupRoomSystem;

	bool loadQuickTips();
	bool loadMenuItems();
	Common::Error runQuickTips();
	Common::Error runMainMenuStub();
	Common::Error runRoomMenuStub(const IndexedBitmap &backdrop);
	Common::Error runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const StartupNpcRecord &npc, const Common::String &usedItemName);
	Common::Error runRoomLoop(const Common::String &entranceName);
	bool ensureCursorEntity();
	bool populateRoomSceneEntities(const StartupRoomSetupState &state,
		const Common::Array<StartupObjectRecord> &drawableObjects,
		const Common::Array<StartupAnimRecord> &drawableAnimations);
	Common::Error beginRoomSetupTransition();
		Common::Error fadeInRoomScene(const byte *palette, float targetBrightness);
		bool pumpTransitionEvents(Common::Error &result);
		void executeStartupAudioCommands(const Common::Array<StartupAudioCommand> &commands);
		void queueDialogueInteraction(const StartupInteractionResult &interaction);
		bool takeQueuedDialogueInteraction(StartupInteractionResult &interaction);
		void requestMainMenuReturn();
		bool hasPendingMainMenuReturn() const;
		bool takePendingMainMenuReturn();
		void clearPendingMainMenuReturn();
		void resetRoomNpcDialogueState();
		void resetCursorAnimationSequence();
		bool tickRuntimeEntities();
		bool handleSystemEvent(const Common::Event &event, Common::Error &result);

	HarvesterEngine &_engine;
	Common::Array<Common::String> _quickTips;
	Common::Array<Common::String> _menuItems;
	Common::Point _mousePos;
	DialogueSystem _dialogue;
	StartupInventorySystem _inventory;
	StartupMenuSystem _menu;
	StartupRoomSystem _room;
	StartupInteractionResult _queuedDialogueInteraction;
	bool _hasQueuedDialogueInteraction = false;
	bool _pendingMainMenuReturn = false;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_FLOW_H
