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

#include "common/scummsys.h"

#include "zvision/scripting/script_manager.h"


namespace ZVision {

int8 ScriptManager::invertory_getCount() {
	return getStateValue(StateKey_Inv_Cnt_Slot);
}

void ScriptManager::invertory_setCount(int8 cnt) {
	setStateValue(StateKey_Inv_Cnt_Slot, cnt);
}

int16 ScriptManager::invertory_getItem(int8 id) {
	if (id < 49 && id >= 0)
		return getStateValue(StateKey_Inv_1_Slot + id);
	return -1;
}

void ScriptManager::invertory_setItem(int8 id, int16 item) {
	if (id < 49 && id >= 0)
		setStateValue(StateKey_Inv_1_Slot + id, item);
}

void ScriptManager::invertory_add(int16 item) {
	int8 cnt = invertory_getCount();

	if (cnt < 49) {
		bool not_exist = true;

		if (cnt == 0) {
			invertory_setItem(0, 0);
			invertory_setCount(1); // we needed empty item for cycle code
			cnt = 1;
		}

		for (int8 cur = 0; cur < cnt; cur++)
			if (invertory_getItem(cur) == item) {
				not_exist = false;
				break;
			}

		if (not_exist) {
			for (int8 i = cnt; i > 0; i--)
				invertory_setItem(i, invertory_getItem(i - 1));

			invertory_setItem(0, item);

			setStateValue(StateKey_InventoryItem, item);

			invertory_setCount(cnt + 1);
		}
	}
}

void ScriptManager::invertory_drop(int16 item) {
	int8 items_cnt = invertory_getCount();

	// if items in inventory > 0
	if (items_cnt != 0) {
		int8 index = 0;

		// finding needed item
		while (index < items_cnt) {
			if (invertory_getItem(index) == item)
				break;

			index++;
		}

		// if item in the inventory
		if (items_cnt != index) {
			// shift all items left with rewrite founded item
			for (int8 v = index; v < items_cnt - 1 ; v++)
				invertory_setItem(v, invertory_getItem(v + 1));

			// del last item
			invertory_setItem(items_cnt - 1, 0);
			invertory_setCount(invertory_getCount() - 1);

			setStateValue(StateKey_InventoryItem, invertory_getItem(0));
		}
	}
}
void ScriptManager::invertory_cycle() {
	int8 item_cnt = invertory_getCount();
	int8 cur_item = invertory_getItem(0);
	if (item_cnt > 1) {
		for (int8 i = 0; i < item_cnt - 1; i++)
			invertory_setItem(i, invertory_getItem(i + 1));

		invertory_setItem(item_cnt - 1, cur_item);

		setStateValue(StateKey_InventoryItem, invertory_getItem(0));

	}
}

} // End of namespace ZVision
