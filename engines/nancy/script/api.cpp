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

#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/convo.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/types.h"

namespace Nancy {
namespace Script {

static const char *const kInventoryType = "ND.Inventory";
static const char *const kInventoryItemType = "ND.InventoryItem";

void pushInventoryItem(lua_State *L, int16 itemID) {
	pushValue(L, kInventoryItemType, itemID);
}

int16 checkInventoryItem(lua_State *L, int index) {
	return *(int16 *)checkUserData(L, index, kInventoryItemType);
}

// Inventory.<name of an item>
static int inventoryIndex(lua_State *L) {
	const char *name = luaL_checkstring(L, 2);

	const INVD *inventory = GetEngineData(INVD);
	if (inventory) {
		for (const INVD::Item &item : inventory->items) {
			if (item.name.equalsIgnoreCase(name)) {
				pushInventoryItem(L, item.id);
				return 1;
			}
		}
	}

	return luaL_error(L, "There is no inventory item %s", name);
}

void registerInventoryAPI(lua_State *L) {
	static const luaL_Reg kInventoryFunctions[] = {
		{ "__index", inventoryIndex },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kInventoryItemFunctions[] = {
		{ nullptr, nullptr }
	};

	registerType(L, kInventoryItemType, kInventoryItemFunctions);
	registerGlobal(L, "Inventory", kInventoryType, kInventoryFunctions);
}

void registerAPI(lua_State *L) {
	registerTypes(L);
	registerMathTypes(L);
	registerRefAPI(L);
	registerBootAPI(L);
	registerInventoryAPI(L);
	registerGameAPI(L);
	registerUIAPI(L);
	registerConvoAPI(L);
	registerRecordAPI(L);
}

} // End of namespace Script
} // End of namespace Nancy
