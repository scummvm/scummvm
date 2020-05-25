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

#ifndef ULTIMA4_GAME_ITEM_H
#define ULTIMA4_GAME_ITEM_H

#include "ultima/ultima4/core/types.h"
#include "common/str.h"

namespace Ultima {
namespace Ultima4 {

class Coords;
class Map;
struct Portal;

enum SearchCondition {
	SC_NONE         = 0x00,
	SC_NEWMOONS     = 0x01,
	SC_FULLAVATAR   = 0x02,
	SC_REAGENTDELAY = 0x04
};

class Items;
typedef bool (Items::*IsInInventoryProc)(int item);
typedef void (Items::*InventoryActionProc)(int item);

#include "common/pack-start.h"	// START STRUCT PACKING
struct ItemLocation {
	const char *_name;
	const char *_shortName;
	const char *_locationLabel;
	IsInInventoryProc _isItemInInventory;
	InventoryActionProc _putItemInInventory;
	InventoryActionProc _useItem;
	int _data;
	byte _conditions;
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

typedef void (*DestroyAllCreaturesCallback)();
#define N_ITEMS 34

class Items {
private:

	static const ItemLocation ITEMS[N_ITEMS];
	DestroyAllCreaturesCallback destroyAllCreaturesCallback;
	int needStoneNames;
	byte stoneMask;
private:
	bool isRuneInInventory(int virt);
	void putRuneInInventory(int virt);
	bool isStoneInInventory(int virt);
	void putStoneInInventory(int virt);
	bool isItemInInventory(int item);
	bool isSkullInInventory(int item);
	void putItemInInventory(int item);

	/**
	 * Use bell, book, or candle on the entrance to the Abyss
	 */
	void useBBC(int item);

	/**
	 * Uses the silver horn
	 */
	void useHorn(int item);

	/**
	 * Uses the wheel (if on board a ship)
	 */
	void useWheel(int item);

	/**
	 * Uses or destroys the skull of Mondain
	 */
	void useSkull(int item);

	/**
	 * Handles using the virtue stones in dungeon altar rooms and on dungeon altars
	 */
	void useStone(int item);
	void useKey(int item);
	bool isMysticInInventory(int mystic);
	void putMysticInInventory(int mystic);
	bool isWeaponInInventory(int weapon);
	void putWeaponInInventory(int weapon);
	void useTelescope(int notused);
	bool isReagentInInventory(int reag);
	void putReagentInInventory(int reag);

	/**
	 * Handles naming of stones when used
	 */
	void itemHandleStones(const Common::String &color);

	/**
	 * Returns true if the specified conditions are met to be able to get the item
	 */
	bool itemConditionsMet(byte conditions);
public:
	Items();
	~Items();

	void setDestroyAllCreaturesCallback(DestroyAllCreaturesCallback callback);

	/**
	 * Returns an item location record if a searchable object exists at
	 * the given location. nullptr is returned if nothing is there.
	 */
	const ItemLocation *itemAtLocation(const Map *map, const Coords &coords);

	/**
	 * Uses the item indicated by 'shortname'
	 */
	void itemUse(const Common::String &shortName);

	/**
	 * Checks to see if the abyss was opened
	 */
	static bool isAbyssOpened(const Portal *p);
};

extern Items *g_items;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
