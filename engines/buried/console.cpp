/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "buried/buried.h"
#include "buried/console.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/inventory_window.h"

namespace Buried {

BuriedConsole::BuriedConsole(BuriedEngine *vm) : _vm(vm) {
	registerCmd("giveitem", WRAP_METHOD(BuriedConsole, cmdGiveItem));
	registerCmd("removeitem", WRAP_METHOD(BuriedConsole, cmdRemoveItem));
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

	FrameWindow *frameWindow = (FrameWindow *)_vm->_mainWindow;

	if (!frameWindow) {
		debugPrintf("Main window not yet created!\n");
		return true;
	}

	if (!frameWindow->isGameInProgress()) {
		debugPrintf("The game is currently not in progress!\n");
		return true;
	}

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

	FrameWindow *frameWindow = (FrameWindow *)_vm->_mainWindow;

	if (!frameWindow) {
		debugPrintf("Main window not yet created!\n");
		return true;
	}

	if (!frameWindow->isGameInProgress()) {
		debugPrintf("The game is currently not in progress!\n");
		return true;
	}

	InventoryWindow *inventory = ((GameUIWindow *)frameWindow->getMainChildWindow())->_inventoryWindow;
	if (!inventory->isItemInInventory(itemID)) {
		debugPrintf("Item %d is not in the inventory\n", itemID);
		return true;
	}

	inventory->removeItem(itemID);
	debugPrintf("Removed item %d to the inventory\n", itemID);
	return true;
}

} // End of namespace Buried
