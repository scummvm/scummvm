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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/bgen/invent.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

IMPLEMENT_DYNCREATE(CInventory, CObject)


/*************************************************************************
 *
 * CInventory()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for inventory class.  Initialize all fields
 *                  to logical nullptr.
 *
 ************************************************************************/

CInventory::CInventory(const char *lpsTitle) {
	m_lpsTitle = lpsTitle;
	m_nItemCount = 0;
	m_pEquipment = nullptr;
}


/*************************************************************************
 *
 * ~CInventory()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for inventory class.  Inventory item objects
 *                  are automatically purged from the equipment list.
 *
 ************************************************************************/

CInventory::~CInventory() {
	CItem   *pItem;

	m_nItemCount = 0;                               // zero the item count
	while (true) {                                  // flush the equipment list
		pItem = m_pEquipment;                       // ... one item at a time
		if (pItem == nullptr)
			break;
		m_pEquipment = (*pItem).m_pNext;            // make next item be first
		delete pItem;
	}
	m_pEquipment = nullptr;
}


/*************************************************************************
 *
 * AddItem()
 *
 * Parameters:
 *  CItem   *pItem  pointer to item to be added
 *
 * Return Value:    none
 *
 * Description:     add an item to the inventory, linking at tail of list.
 *
 ************************************************************************/

void CInventory::AddItem(CItem *pItem) {
	CItem   *pTemp;

	if (((*pItem).m_pPrev != nullptr) ||               // don't relink a linked item
	        ((*pItem).m_pNext != nullptr))
		return;

	pTemp = FindItem((*pItem).m_nID);               // see if it is already in the list
	if (pTemp != nullptr) {                            // ... and if so, just bump the quantity
		(*pTemp).m_nQuantity += (*pItem).m_nQuantity;   // ... of the existing item and
		delete pItem;                               // ... then purge the item given us
		return;
	}

	if (m_pEquipment == nullptr)                       // make it be first in the list
		m_pEquipment = pItem;                       // ... if no existing items
	else {
		pTemp = m_pEquipment;                       // otherwise hunt for the last item
		while ((*pTemp).m_pNext != nullptr)            // ... and insert it as the new last one
			pTemp = (*pTemp).m_pNext;
		(*pTemp).m_pNext = pItem;
		(*pItem).m_pPrev = pTemp;
	}

	m_nItemCount += 1;                              // increment item count accordingly
}


/*************************************************************************
 *
 * AddItem()
 *
 * Parameters:
 *  int nID         item identifier
 *  int nQuantity   number of units of the item
 *
 * Return Value:    none
 *
 * Description:     add an item to the inventory, linking at head of list.
 *
 ************************************************************************/

void CInventory::AddItem(int nID, long nQuantity) {
	CItem   *pItem;

	pItem = FindItem(nID);                          // see if we already have it
	if (pItem == nullptr) {                            // if not, then create it
		pItem = new CItem(nID);                     // ... initialize its values
		(*pItem).m_nQuantity = nQuantity;           // ... and link it into the list
		AddItem(pItem);
	} else
		(*pItem).m_nQuantity += nQuantity;          // just bump the quantity field
}


/*************************************************************************
 *
 * RemoveItem()
 *
 * Parameters:
 *  CItem   *pItem  pointer to item to be removed
 *
 * Return Value:    none
 *
 * Description:     remove an item from the inventory.
 *
 ************************************************************************/

void CInventory::RemoveItem(CItem *pItem) {
	CItem   *pTemp;

	if (((*pItem).m_pPrev == nullptr) &&               // don't unlink a lone item
	        ((*pItem).m_pNext == nullptr) &&
	        (m_pEquipment != pItem))
		return;

	if ((*pItem).m_pPrev == nullptr) {                 // handle being first item in list
		m_pEquipment = (*pItem).m_pNext;            // ... next item becomes new head
		if (m_pEquipment != nullptr)                   // ... if we weren't the only item
			(*m_pEquipment).m_pPrev = nullptr;         // ... then clear head's previous pointer
	} else {                                        // handle being not head of list
		pTemp = (*pItem).m_pPrev;                   // ... make previous item point to after us
		(*pTemp).m_pNext = (*pItem).m_pNext;
		if ((*pTemp).m_pNext != nullptr)               // ... if something was after us, then
			(*(*pTemp).m_pNext).m_pPrev = pTemp;    // ... have it point to what's previous
	}

	(*pItem).m_pNext = (*pItem).m_pPrev = nullptr;
	m_nItemCount -= 1;                              // decrement item count accordingly
}


/*************************************************************************
 *
 * DiscardItem()
 *
 * Parameters:
 *  CItem   *pItem  pointer to item to be discarded
 *
 * Return Value:    none
 *
 * Description:     remove an item from the inventory and delete it.
 *
 ************************************************************************/

void CInventory::DiscardItem(CItem *pItem) {
	RemoveItem(pItem);                                  // remove it
	delete pItem;                                   // delete it
}


/*************************************************************************
 *
 * DiscardItem()
 *
 * Parameters:
 *  CItem *pItem    pointer to item to be affected
 *  int nQuantity   number of item units to be discarded
 *
 * Return Value:    none
 *
 * Description:     reduce a quantity of an item from the inventory,
 *                  discarding the item itself if the requested amount
 *                  exceeds the supply on hand.
 *
 ************************************************************************/

void CInventory::DiscardItem(CItem *pItem, long nQuantity) {
	(*pItem).m_nQuantity -= nQuantity;              // debit the requested quantity

	if ((*pItem).m_nID == MG_OBJ_CROWN) {           // special case handling for crowns
		if ((*pItem).m_nQuantity < 0)               // ... can have zero count
			(*pItem).m_nQuantity = 0;
	} else if ((*pItem).m_nQuantity <= 0)               // else if we exhausted the supply
		DiscardItem(pItem);                         // ... then purge the item
}


/*************************************************************************
 *
 * DiscardItem()
 *
 * Parameters:
 *  int nID         identifier of item to be discarded
 *
 * Return Value:    none
 *
 * Description:     discard the specified item from the inventory.
 *
 ************************************************************************/

void CInventory::DiscardItem(int nItem) {
	CItem   *pItem;

	pItem = FindItem(nItem);                        // see if we have the specified item
	if (pItem != nullptr)                              // ... and if so, remove it from the list
		DiscardItem(pItem);
}


/*************************************************************************
 *
 * DiscardItem()
 *
 * Parameters:
 *  int nID         identifier of item to be affected
 *  int nQuantity   number of item units to be discarded
 *
 * Return Value:    none
 *
 * Description:     reduce a quantity of an item from the inventory,
 *                  discarding the item itself if the requested amount
 *                  exceeds the supply on hand.
 *
 ************************************************************************/

void CInventory::DiscardItem(int nItem, long nQuantity) {
	CItem   *pItem;

	pItem = FindItem(nItem);                        // see if we have the specified item
	if (pItem != nullptr)                              // ... and if so remove the specified quantity
		DiscardItem(pItem, nQuantity);
}


/*************************************************************************
 *
 * FetchItem()
 *
 * Parameters:
 *  int nIdx        index of the item to look for
 *
 * Return Value:
 *  CItem *         pointer to item or nullptr
 *
 * Description:     retrieve an item from the inventory, and return it pointer.
 *
 ************************************************************************/

CItem *CInventory::FetchItem(int nIdx) {
	int     i;
	CItem   *pItem;

	if ((nIdx >= m_nItemCount) ||                   // punt if the index is invalid
	        (nIdx < 0))
		return nullptr;

	pItem = m_pEquipment;                           // get head of equipment list
	for (i = 0; pItem != nullptr; i++) {               // scan through the list
		if (nIdx == i)                              // ... looking for item
			break;
		pItem = (*pItem).m_pNext;
	}

	return pItem;                                   // return what we found
}


/*************************************************************************
 *
 * FindItem()
 *
 * Parameters:
 *  int nID         identifier of the item to look for
 *
 * Return Value:
 *  CItem *         pointer to item or nullptr
 *
 * Description:     look for an item in the inventory, and return it pointer.
 *
 ************************************************************************/

CItem *CInventory::FindItem(int nID) {
	CItem   *pItem;

	pItem = m_pEquipment;                           // get head of equipment list
	while (pItem != nullptr) {                         // scan through the list
		if (nID == (*pItem).m_nID)                  // ... looking for a match
			break;
		pItem = (*pItem).m_pNext;
	}

	return pItem;                                   // return what we found
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

/////////////////////////////////////////////////////////////////////////////
