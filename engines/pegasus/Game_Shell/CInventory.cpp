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

#include "pegasus/Game_Shell/CItem.h"
#include "pegasus/Game_Shell/CInventory.h"
#include "pegasus/Game_Shell/Headers/Game_Shell_Constants.h"

namespace Pegasus {

CInventory::CInventory() {
	fWeightLimit = 100;
	fOwnerID = kNoActorID;
	fReferenceCount = 0;
}

CInventory::~CInventory() {
}

void CInventory::SetWeightLimit(tWeightType limit) {
	fWeightLimit = limit;
	//	*** What to do if the new weight limit is greater than the current weight?
}

tWeightType CInventory::GetWeight() {
	tWeightType	result = 0;

	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++)
		result += (*it)->GetItemWeight();

	return result;
}

//	If the item already belongs, just return kInventoryOK.
tInventoryResult CInventory::AddItem(CItem *item) {
	if (ItemInInventory(item))
		return kInventoryOK;

	if (GetWeight() + item->GetItemWeight() > fWeightLimit)
		return kTooMuchWeight;

	fInventoryList.push_back(item);
	item->SetItemOwner(fOwnerID);

	++fReferenceCount;
	return kInventoryOK;
}

tInventoryResult CInventory::RemoveItem(CItem *item) {
	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++) {
		if (*it == item) {
			fInventoryList.erase(it);
			item->SetItemOwner(kNoActorID);

			++fReferenceCount;
			return kInventoryOK;
		}
	}

	return kItemNotInInventory;
}

tInventoryResult CInventory::RemoveItem(tItemID id) {
	CItem *item = FindItemByID(id);
	
	if (item) {
		fInventoryList.remove(item);
		item->SetItemOwner(kNoActorID);

		++fReferenceCount;
		return kInventoryOK;
	}

	return kItemNotInInventory;
}

void CInventory::RemoveAllItems() {
	fInventoryList.clear();
	++fReferenceCount;
}

bool CInventory::ItemInInventory(CItem *item) {
	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++)
		if (*it == item)
			return true;

	return false;
}

bool CInventory::ItemInInventory(tItemID id) {
	return FindItemByID(id) != NULL;
}

CItem *CInventory::GetItemAt(int32 index) {
	int32 i = 0;
	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++, i++)
		if (i == index)
			return *it;

	return 0;
}

tItemID CInventory::GetItemIDAt(int32 index) {
	CItem *item = GetItemAt(index);

	if (item)
		return item->GetObjectID();

	return kNoItemID;
}

CItem *CInventory::FindItemByID(tItemID id) {
	return fInventoryList.FindItemByID(id);
}

//	Return -1 if not found.

int32 CInventory::FindIndexOf(CItem *item) {
	uint32 i = 0;
	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++, i++)
		if (*it == item)
			return i;

	return -1;
}

//	Return -1 if not found.

int32 CInventory::FindIndexOf(tItemID id) {
	uint32 i = 0;
	for (CItemIterator it = fInventoryList.begin(); it != fInventoryList.end(); it++, i++)
		if ((*it)->GetObjectID() == id)
			return i;

	return -1;
}

tWeightType CInventory::GetWeightLimit() {
	return fWeightLimit;
}

int32 CInventory::GetNumItems() {
	return fInventoryList.size();
}

void CInventory::SetOwnerID(const tActorID id) {
	fOwnerID = id;
}

tActorID CInventory::GetOwnerID() const {
	return fOwnerID;
}

} // End of namespae Pegasus
