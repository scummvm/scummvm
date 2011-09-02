/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_ITEMS_INVENTORY_H
#define PEGASUS_ITEMS_INVENTORY_H

#include "pegasus/types.h"
#include "pegasus/items/itemlist.h"

namespace Pegasus {

class Item;

//	Inventories have a "current item". This item is the default item the player can
//	use. In a text adventure system, the current item would be "it", as in
//	"Hit the troll with it," where "it" would refer to some weapon which is the current
//	item. In a graphic adventure, the current item would be the item the user selects
//	to use with the mouse or other pointing device.

class Inventory {
public:
	Inventory();
	virtual ~Inventory();
	
	tWeightType getWeightLimit();
	void setWeightLimit(tWeightType limit);
	tWeightType getWeight();
	
	virtual tInventoryResult addItem(Item *item);
	virtual tInventoryResult removeItem(Item *item);
	virtual tInventoryResult removeItem(tItemID id);
	virtual bool itemInInventory(Item *item);
	virtual bool itemInInventory(tItemID id);
	virtual Item *getItemAt(int32 index);
	virtual tItemID getItemIDAt(int32 index);
	virtual Item *findItemByID(tItemID id);
	virtual int32 findIndexOf(Item *item);
	virtual int32 findIndexOf(tItemID id);
	int32 getNumItems();
	virtual void removeAllItems();
	
	void setOwnerID(const tActorID id);
	tActorID getOwnerID() const;
	
	uint32 getReferenceCount() { return _referenceCount; }

protected:
	tWeightType _weightLimit;
	tActorID _ownerID;
	ItemList _inventoryList;

private:
	uint32 _referenceCount;
};

} // End of namespace Pegasus

#endif
