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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/script_manager.h"

namespace ZVision {

int8 ScriptManager::inventoryGetCount() {
	return getStateValue(StateKey_Inv_Cnt_Slot);
}

void ScriptManager::inventorySetCount(int8 cnt) {
	setStateValue(StateKey_Inv_Cnt_Slot, cnt);
}

int16 ScriptManager::inventoryGetItem(int8 id) {
	if (id < 49 && id >= 0)
		return getStateValue(StateKey_Inv_1_Slot + id);
	return -1;
}

void ScriptManager::inventorySetItem(int8 id, int16 item) {
	if (id < 49 && id >= 0)
		setStateValue(StateKey_Inv_1_Slot + id, item);
}

void ScriptManager::inventoryAdd(int16 item) {
	int8 cnt = inventoryGetCount();

	if (cnt < 49) {
		bool notExist = true;

		if (cnt == 0) {
			inventorySetItem(0, 0);
			inventorySetCount(1); // we needed empty item for cycle code
			cnt = 1;
		}

		for (int8 cur = 0; cur < cnt; cur++)
			if (inventoryGetItem(cur) == item) {
				notExist = false;
				break;
			}

		if (notExist) {
			for (int8 i = cnt; i > 0; i--)
				inventorySetItem(i, inventoryGetItem(i - 1));

			inventorySetItem(0, item);

			setStateValue(StateKey_InventoryItem, item);

			inventorySetCount(cnt + 1);
		}
	}
}

void ScriptManager::inventoryDrop(int16 item) {
	int8 itemCount = inventoryGetCount();

	// if items in inventory > 0
	if (itemCount != 0) {
		int8 index = 0;

		// finding needed item
		while (index < itemCount) {
			if (inventoryGetItem(index) == item)
				break;

			index++;
		}

		// if item in the inventory
		if (itemCount != index) {
			// shift all items left with rewrite founded item
			for (int8 v = index; v < itemCount - 1 ; v++)
				inventorySetItem(v, inventoryGetItem(v + 1));

			// del last item
			inventorySetItem(itemCount - 1, 0);
			inventorySetCount(inventoryGetCount() - 1);

			setStateValue(StateKey_InventoryItem, inventoryGetItem(0));
		}
	}
}
void ScriptManager::inventoryCycle() {
	int8 itemCount = inventoryGetCount();
	int8 curItem = inventoryGetItem(0);
	if (itemCount > 1) {
		for (int8 i = 0; i < itemCount - 1; i++)
			inventorySetItem(i, inventoryGetItem(i + 1));

		inventorySetItem(itemCount - 1, curItem);

		setStateValue(StateKey_InventoryItem, inventoryGetItem(0));

	}
}

} // End of namespace ZVision
