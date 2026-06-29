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

#include "hollywood/console.h"

#include "common/str.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

Console::Console(HollywoodEngine *vm) :
		GUI::Debugger(),
		_vm(vm) {
	registerCmd("get", WRAP_METHOD(Console, cmdGet));
}

bool Console::cmdGet(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <item id|all>\n", argv[0]);
		return true;
	}

	GameplayState &state = _vm->gameState();
	const byte owner = inventoryOwner();
	state.currentInventoryOwnerIndex = owner;
	if (owner == 1)
		state.initializeSueItemResourcePages();

	if (Common::String(argv[1]).equalsIgnoreCase("all")) {
		const byte itemCount = state.giveInventoryItemsWithResourcePages(owner);
		debugPrintf("Added %u inventory items with resource pages to owner %u\n",
			itemCount, owner);
		return true;
	}

	uint itemId = 0;
	if (!parseItemId(argv[1], itemId)) {
		debugPrintf("Invalid inventory item id '%s'\n", argv[1]);
		debugPrintf("Usage: %s <item id|all>\n", argv[0]);
		return true;
	}

	if (itemId == 0 || itemId >= GameplayState::kInventoryOwnerSlotStride) {
		debugPrintf("Inventory item id %u is out of range; valid range is 1..%u\n",
			itemId, GameplayState::kInventoryLastSlot);
		return true;
	}

	if (state.inventoryItemResourcePageByOwnerAndItemId[owner][itemId] == 0) {
		debugPrintf("Inventory item %u does not belong to owner %u or has no resource page\n",
			itemId, owner);
		return true;
	}

	if (state.hasInventoryItem(owner, (byte)itemId)) {
		debugPrintf("Inventory owner %u already has item %u\n", owner, itemId);
		return true;
	}

	state.addInventoryItem(owner, (byte)itemId);
	if (state.hasInventoryItem(owner, (byte)itemId))
		debugPrintf("Added inventory item %u to owner %u\n", itemId, owner);
	else
		debugPrintf("Could not add inventory item %u to owner %u\n", itemId, owner);
	return true;
}

bool Console::parseItemId(const char *argument, uint &itemId) {
	char *endPtr = nullptr;
	const long parsedValue = strtol(argument, &endPtr, 0);
	if (endPtr == argument || *endPtr != 0 || parsedValue < 0)
		return false;

	itemId = (uint)parsedValue;
	return true;
}

byte Console::inventoryOwner() const {
	const GameplayState &state = _vm->gameState();
	if (state.currentInventoryOwnerIndex < GameplayState::kInventoryOwnerCount)
		return state.currentInventoryOwnerIndex;

	return 0;
}

} // End of namespace Hollywood
