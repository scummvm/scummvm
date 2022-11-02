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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/buried.h"
#include "buried/console.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_base.h"

namespace Buried {

BuriedConsole::BuriedConsole(BuriedEngine *vm) : _vm(vm) {
	registerCmd("giveitem", WRAP_METHOD(BuriedConsole, cmdGiveItem));
	registerCmd("removeitem", WRAP_METHOD(BuriedConsole, cmdRemoveItem));

	// This feature, while present in the demo and trial, has 99% bad
	// entries and is generally useless.
	if (!_vm->isDemo() && !_vm->isTrial())
		registerCmd("jumpentry", WRAP_METHOD(BuriedConsole, cmdJumpEntry));

	registerCmd("curloc", WRAP_METHOD(BuriedConsole, cmdCurLocation));
	registerCmd("aicommentinfo", WRAP_METHOD(BuriedConsole, cmdAiCommentInfo));
}

BuriedConsole::~BuriedConsole() {
}

bool BuriedConsole::cmdGiveItem(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <item ID>\n", argv[0]);
		return true;
	}

	int itemID = atoi(argv[1]);

	if (itemID < 0 || itemID > kItemWoodenPegs) {
		debugPrintf("Invalid item ID %d!\n", itemID);
		return true;
	}

	FrameWindow *frameWindow = getFrameWindow();
	if (!frameWindow)
		return true;

	InventoryWindow *inventory = ((GameUIWindow *)frameWindow->getMainChildWindow())->_inventoryWindow;
	if (inventory->isItemInInventory(itemID)) {
		debugPrintf("Item %d is already in the inventory\n", itemID);
		return true;
	}

	inventory->addItem(itemID);
	debugPrintf("Added item %d to the inventory\n", itemID);
	return true;
}

bool BuriedConsole::cmdRemoveItem(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <item ID>\n", argv[0]);
		return true;
	}

	int itemID = atoi(argv[1]);

	if (itemID < 0 || itemID > kItemWoodenPegs) {
		debugPrintf("Invalid item ID %d!\n", itemID);
		return true;
	}

	FrameWindow *frameWindow = getFrameWindow();
	if (!frameWindow)
		return true;

	InventoryWindow *inventory = ((GameUIWindow *)frameWindow->getMainChildWindow())->_inventoryWindow;
	if (!inventory->isItemInInventory(itemID)) {
		debugPrintf("Item %d is not in the inventory\n", itemID);
		return true;
	}

	inventory->removeItem(itemID);
	debugPrintf("Removed item %d to the inventory\n", itemID);
	return true;
}

bool BuriedConsole::cmdJumpEntry(int argc, const char **argv) {
	loadJumpEntryList();

	if (argc < 2) {
		debugPrintf("Usage: %s <index>\n\nEntries:\n", argv[0]);
		debugPrintf("# |Time Zone       |Environment            \n");
		debugPrintf("--|----------------|-----------------------\n");

		for (uint32 i = 0; i < _jumpEntryList.size(); i++) {
			const JumpEntry &entry = _jumpEntryList[i];
			debugPrintf("%2d|%-16s|%-23s\n", i + 1, entry.timeZoneName.c_str(), entry.locationName.c_str());
		}

		return true;
	}

	// Bail if not playing
	if (!isPlaying())
		return true;

	int entry = atoi(argv[1]) - 1;
	if (entry < 0 || entry >= (int)_jumpEntryList.size()) {
		debugPrintf("Invalid entry!\n");
		return true;
	}

	// Store the location to make the jump after we close the console
	_jump = _jumpEntryList[entry].location;
	return false;
}

bool BuriedConsole::cmdCurLocation(int argc, const char **argv) {
	FrameWindow *frameWindow = getFrameWindow();
	if (!frameWindow)
		return true;

	const SceneBase *scene = ((GameUIWindow *)frameWindow->getMainChildWindow())->_sceneViewWindow->getCurrentScene();
	if (!scene) {
		debugPrintf("No scene!\n");
		return true;
	}

	const LocationStaticData &staticData = scene->_staticData;
	debugPrintf("Time Zone: %d\n", staticData.location.timeZone);
	debugPrintf("Environment: %d\n", staticData.location.environment);
	debugPrintf("Node: %d\n", staticData.location.node);
	debugPrintf("Facing: %d\n", staticData.location.facing);
	debugPrintf("Orientation: %d\n", staticData.location.orientation);
	debugPrintf("Depth: %d\n", staticData.location.depth);
	debugPrintf("Class: %d\n", staticData.classID);

	return true;
}

bool BuriedConsole::cmdAiCommentInfo(int argc, const char **argv) {
	loadJumpEntryList();

	if (argc < 2) {
		debugPrintf("Usage: %s <index>\n\nEntries:\n", argv[0]);
		debugPrintf("# |Time Zone       |Environment            \n");
		debugPrintf("--|----------------|-----------------------\n");

		for (uint32 i = 0; i < _jumpEntryList.size(); i++) {
			const JumpEntry &entry = _jumpEntryList[i];
			debugPrintf("%2d|%-16s|%-23s\n", i + 1, entry.timeZoneName.c_str(), entry.locationName.c_str());
		}

		return true;
	}

	// Bail if not playing
	if (!isPlaying())
		return true;

	int entry = atoi(argv[1]) - 1;
	if (entry < 0 || entry >= (int)_jumpEntryList.size()) {
		debugPrintf("Invalid entry!\n");
		return true;
	}

	FrameWindow *frameWindow = getFrameWindow();
	if (!frameWindow)
		return true;

	SceneViewWindow *sceneView = ((GameUIWindow *)frameWindow->getMainChildWindow())->_sceneViewWindow;
	if (!sceneView) {
		debugPrintf("No scene view!\n");
		return true;
	}

	// Check if the location has AI comment data
	Location &loc = _jumpEntryList[entry].location;
	bool found = false;

	for (const AICommentInfo *info = s_aiCommentInfo; info->timeZone; ++info) {
		if (info->timeZone == loc.timeZone && info->environment == loc.environment) {
			found = true;
			break;
		}
	}

	if (found) {
		Common::Array<AIComment> commentDatabase = sceneView->getAICommentDatabase(loc.timeZone, loc.environment);

		if (!commentDatabase.empty()) {
			debugPrintf("ID  |Flags |DFlagA |non-base |DValA |DFlagB |non-base |DValB |SFlag |non-base\n");
			debugPrintf("----|------|-------|---------|------|-------|---------|------|------|--------\n");
			for (uint32 i = 0; i < commentDatabase.size(); i++) {
				const AIComment &comment = commentDatabase[i];
				debugPrintf("%4d|%-6d|%-7d|%-9d|%-6d|%-7d|%-9d|%-6d|%-6d|%-8d\n",
							comment.commentID, comment.commentFlags,
							comment.dependencyFlagOffsetA,
							comment.commentFlags & AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_A,
							comment.dependencyValueA,
							comment.dependencyFlagOffsetB,
							comment.commentFlags & AI_DEPENDENCY_FLAG_NON_BASE_DERIVED_B,
							comment.dependencyValueB,
							comment.statusFlagOffset,
							comment.commentFlags & AI_STATUS_FLAG_NON_BASE_DERIVED);
			}

		} else {
			debugPrintf("Location has no AI comment data");
		}
	} else {
		debugPrintf("Location has no AI comment data");
	}

	return true;
}

void BuriedConsole::postEnter() {
	GUI::Debugger::postEnter();

	if (_jump.timeZone >= 0) {
		// Perform a requested jump
		FrameWindow *frameWindow = (FrameWindow *)_vm->_mainWindow;
		SceneViewWindow *sceneView = ((GameUIWindow *)frameWindow->getMainChildWindow())->_sceneViewWindow;
		if (!sceneView->jumpToScene(_jump))
			error("Failed to jump to requested time zone");

		_jump = Location();
	}
}

static int getNextLocationInt(const char *&ptr) {
	if (!ptr || *ptr == 0)
		return -1;

	int value = atoi(ptr);
	ptr = strchr(ptr, ',');
	if (ptr)
		ptr++;

	return value;
}

void BuriedConsole::loadJumpEntryList() {
	// Check if loaded already
	if (!_jumpEntryList.empty())
		return;

	for (uint32 i = IDS_MOVEMENT_DATA_BASE_ID; ; i++) {
		Common::String text = _vm->getString(i);
		if (text.empty())
			break;

		const char *pipeStr1 = strchr(text.c_str(), '|');
		if (!pipeStr1)
			break;

		JumpEntry entry;
		entry.timeZoneName = Common::String(text.c_str(), pipeStr1);

		const char *pipeStr2 = strchr(pipeStr1 + 1, '|');
		if (!pipeStr2)
			break;

		entry.locationName = Common::String(pipeStr1 + 1, pipeStr2);

		pipeStr2++;
		entry.location.timeZone = getNextLocationInt(pipeStr2);
		entry.location.environment = getNextLocationInt(pipeStr2);
		entry.location.node = getNextLocationInt(pipeStr2);
		entry.location.facing = getNextLocationInt(pipeStr2);
		entry.location.orientation = getNextLocationInt(pipeStr2);
		entry.location.depth = getNextLocationInt(pipeStr2);

		// Failed to parse
		if (entry.location.timeZone < 0 || entry.location.environment < 0 ||
				entry.location.node < 0 || entry.location.facing < 0 ||
				entry.location.orientation < 0 || entry.location.depth < 0)
			break;

		_jumpEntryList.push_back(entry);
	}
}

FrameWindow *BuriedConsole::getFrameWindow() {
	FrameWindow *frameWindow = (FrameWindow *)_vm->_mainWindow;

	if (!frameWindow) {
		debugPrintf("Main window not yet created!\n");
		return nullptr;
	}

	if (!frameWindow->isGameInProgress()) {
		debugPrintf("The game is currently not in progress!\n");
		return nullptr;
	}

	return frameWindow;
}

} // End of namespace Buried
