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

#include "bagel/hodjnpodj/metagame/bgen/invent.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {


CInventory::CInventory(char *lpsTitle) {
	m_lpsTitle = lpsTitle;
	m_nItemCount = 0;
	m_pEquipment = nullptr;
}

CInventory::~CInventory() {
	CItem *pItem;

	m_nItemCount = 0;			// Zero the item count

	while (true) {				// Flush the equipment list
		pItem = m_pEquipment;	// ... one item at a time
		if (pItem == nullptr)
			break;
		m_pEquipment = (*pItem).m_pNext;	// Make next item be first
		delete pItem;
	}

	m_pEquipment = nullptr;
}

void CInventory::AddItem(CItem *pItem) {
	CItem *pTemp;

	if (((*pItem).m_pPrev != nullptr) ||		// Don't relink a linked item
		((*pItem).m_pNext != nullptr))
		return;

	pTemp = FindItem((*pItem).m_nID);					// See if it is already in the list
	if (pTemp != nullptr) {								// ... and if so, just bump the quantity
		(*pTemp).m_nQuantity += (*pItem).m_nQuantity;	// ... of the existing item and
		delete pItem;									// ... then purge the item given us
		return;
	}

	if (m_pEquipment == nullptr) {			// Make it be first in the list
		m_pEquipment = pItem;				// ... if no existing items
	} else {
		pTemp = m_pEquipment;				// Otherwise hunt for the last item
		while ((*pTemp).m_pNext != nullptr)	// ... and insert it as the new last one
			pTemp = (*pTemp).m_pNext;

		(*pTemp).m_pNext = pItem;
		(*pItem).m_pPrev = pTemp;
	}

	m_nItemCount += 1;						// Increment item count accordingly
}

void CInventory::AddItem(int nID, long nQuantity) {
	CItem *pItem;

	pItem = FindItem(nID);					// See if we already have it
	if (pItem == nullptr) {					// If not, then create it
		pItem = new CItem(nID);				// ... initialize its values
		(*pItem).m_nQuantity = nQuantity;	// ... and link it into the list
		AddItem(pItem);
	} else {
		(*pItem).m_nQuantity += nQuantity;	// Just bump the quantity field
	}
}

void CInventory::RemoveItem(CItem *pItem) {
	CItem *pTemp;

	if (((*pItem).m_pPrev == nullptr) &&		// Don't unlink a lone item
		((*pItem).m_pNext == nullptr) &&
		(m_pEquipment != pItem))
		return;

	if ((*pItem).m_pPrev == nullptr) {				// Handle being first item in list
		m_pEquipment = (*pItem).m_pNext;			// ... next item becomes new head
		if (m_pEquipment != nullptr)				// ... if we weren't the only item
			(*m_pEquipment).m_pPrev = nullptr;		// ... then clear head's previous pointer
	} else {										// Handle being not head of list
		pTemp = (*pItem).m_pPrev;					// ... make previous item point to after us
		(*pTemp).m_pNext = (*pItem).m_pNext;
		if ((*pTemp).m_pNext != nullptr)			// ... if something was after us, then
			(*(*pTemp).m_pNext).m_pPrev = pTemp;	// ... have it point to what's previous
	}

	(*pItem).m_pNext = (*pItem).m_pPrev = nullptr;
	m_nItemCount -= 1;								// Decrement item count accordingly
}

void CInventory::DiscardItem(CItem *pItem) {
	RemoveItem(pItem);		// Remove it
	delete pItem;			// Delete it
}

void CInventory::DiscardItem(CItem *pItem, long nQuantity) {
	(*pItem).m_nQuantity -= nQuantity;		// Debit the requested quantity

	if ((*pItem).m_nID == MG_OBJ_CROWN) {	// Special case handling for crowns
		if ((*pItem).m_nQuantity < 0)		// ... can have zero count
			(*pItem).m_nQuantity = 0;
	} else
		if ((*pItem).m_nQuantity <= 0)		// Else if we exhausted the supply
			DiscardItem(pItem);				// ... then purge the item
}

void CInventory::DiscardItem(int nItem) {
	CItem *pItem;

	pItem = FindItem(nItem);	// See if we have the specified item
	if (pItem != nullptr)		// ... and if so, remove it from the list
		DiscardItem(pItem);
}

void CInventory::DiscardItem(int nItem, long nQuantity) {
	CItem *pItem;

	pItem = FindItem(nItem);			// See if we have the specified item
	if (pItem != nullptr)				// ... and if so remove the specified quantity
		DiscardItem(pItem, nQuantity);
}

CItem *CInventory::FetchItem(int nIdx) {
	int     i;
	CItem *pItem;

	if ((nIdx >= m_nItemCount) ||			// Punt if the index is invalid
		(nIdx < 0))
		return(nullptr);

	pItem = m_pEquipment;					// Get head of equipment list
	for (i = 0; pItem != nullptr; i++) {	// Scan through the list
		if (nIdx == i)						// ... looking for item
			break;
		pItem = (*pItem).m_pNext;
	}

	return pItem;							// Return what we found
}

CItem *CInventory::FindItem(int nID) {
	CItem *pItem;

	pItem = m_pEquipment;			// Get head of equipment list
	while (pItem != nullptr) {		// Scan through the list
		if (nID == (*pItem).m_nID)	// ... looking for a match
			break;
		pItem = (*pItem).m_pNext;
	}

	return pItem;					// Return what we found
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
