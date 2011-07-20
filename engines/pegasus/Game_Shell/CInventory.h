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

#ifndef PEGASUS_GAMESHELL_CINVENTORY_H
#define PEGASUS_GAMESHELL_CINVENTORY_H

#include "pegasus/Game_Shell/CItemList.h"
#include "pegasus/Game_Shell/Headers/Game_Shell_Types.h"

namespace Pegasus {

class CItem;

//	Inventories have a "current item". This item is the default item the player can
//	use. In a text adventure system, the current item would be "it", as in
//	"Hit the troll with it," where "it" would refer to some weapon which is the current
//	item. In a graphic adventure, the current item would be the item the user selects
//	to use with the mouse or other pointing device.

class CInventory {
public:
	CInventory();
	virtual ~CInventory();
	
	tWeightType GetWeightLimit();
	void SetWeightLimit(tWeightType limit);
	tWeightType GetWeight();
	
	virtual tInventoryResult AddItem(CItem *item);
	virtual tInventoryResult RemoveItem(CItem *item);
	virtual tInventoryResult RemoveItem(tItemID id);
	virtual bool ItemInInventory(CItem *item);
	virtual bool ItemInInventory(tItemID id);
	virtual CItem *GetItemAt(int32 index);
	virtual tItemID GetItemIDAt(int32 index);
	virtual CItem *FindItemByID(tItemID id);
	virtual int32 FindIndexOf(CItem *item);
	virtual int32 FindIndexOf(tItemID id);
	int32 GetNumItems();
	virtual void RemoveAllItems();
	
	void SetOwnerID(const tActorID id);
	tActorID GetOwnerID() const;
	
	uint32 GetReferenceCount() { return fReferenceCount; }

protected:
	tWeightType fWeightLimit;
	tActorID fOwnerID;
	CItemList fInventoryList;

private:
	uint32 fReferenceCount;
};

} // End of namespace Pegasus

#endif
