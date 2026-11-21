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

#include "common/str.h"
#include "hollywood/runaway/console.h"
#include "hollywood/runaway/engine.h"
#include "hollywood/runaway/inventory.h"

namespace Hollywood {

RunawayConsole::RunawayConsole(RunawayEngine *vm) : _vm(vm) {
	registerCmd("get", WRAP_METHOD(RunawayConsole, cmdGet));
	registerCmd("unlock", WRAP_METHOD(RunawayConsole, cmdUnlockChapters));
}

bool RunawayConsole::cmdGet(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Usage: %s [item id|all]\n", argv[0]);
		return true;
	}
	if (!_vm->_inventory) {
		debugPrintf("The inventory is not available yet.\n");
		return true;
	}

	RunawayInventory &inventory = *_vm->_inventory;
	if (argc == 1) {
		debugPrintf("Usage: %s [item id|all]\n", argv[0]);
		debugPrintf("Chapter %u items (* = already in inventory):\n", _vm->_state.chapter());
		for (uint item = 1; item <= inventory.maxItemId(); ++item) {
			debugPrintf("%2u %c %s\n", item, inventory.contains(item) ? '*' : ' ', inventory.name(item).encode().c_str());
		}
		return true;
	}
	if (!_vm->canSaveGameStateCurrently()) {
		debugPrintf("Items can only be added during idle gameplay.\n");
		return true;
	}

	const Common::String argument(argv[1]);
	if (argument.equalsIgnoreCase("all")) {
		uint added = 0;
		for (uint item = 1; item <= inventory.maxItemId(); ++item) {
			if (inventory.add(item)) {
				++added;
			}
		}
		debugPrintf("Added %u items for chapter %u.\n", added, _vm->_state.chapter());
		return true;
	}

	char *end = nullptr;
	const long item = strtol(argv[1], &end, 0);
	if (end == argv[1] || *end || item < 1 || item > (long)inventory.maxItemId()) {
		debugPrintf("Invalid item '%s'; valid IDs for chapter %u are 1..%u.\n",
			argv[1], _vm->_state.chapter(), inventory.maxItemId());
		return true;
	}
	if (inventory.add(item)) {
		debugPrintf("Added item %u: %s.\n", (uint)item, inventory.name(item).encode().c_str());
	} else {
		debugPrintf("Item %u is already in the inventory.\n", (uint)item);
	}
	return true;
}

bool RunawayConsole::cmdUnlockChapters(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}
	if (_vm->_game != kRunawayFullGame) {
		debugPrintf("Chapter selection is only available in the full game.\n");
		return true;
	}
	_vm->unlockChapter(6);
	debugPrintf("All six chapters are unlocked for this game target.\n");
	debugPrintf("The chapter selection menu is shown when starting the game without --boot-param.\n");
	return true;
}

} // End of namespace Hollywood
