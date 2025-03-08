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

#ifndef HODJNPODJ_METAGAME_BGEN_INVENT_H
#define HODJNPODJ_METAGAME_BGEN_INVENT_H

#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define INVENT_MIN_ITEM_COUNT	2

class CInventory {
public:
	CInventory(char *lpsTitle = nullptr);
	~CInventory();

public:
	// Implementation
	const char *GetTitle() const {
		return m_lpsTitle;
	}
	int	ItemCount() const {
		return m_nItemCount;
	}

	/**
	 * Add an item to the inventory, linking at tail of list.
	 * @param pItem		Pointer to item to be added
	 */
	void AddItem(CItem *pItem);

	/**
	 * Add an item to the inventory, linking at head of list.
	 * @param nID		Item identifier
	 * @param nQuantity	Number of units of the item
	 */
	void AddItem(int nID, long nQuantity);

	/**
	 * Remove an item from the inventory.
	 * @param pItem		Pointer to item to be removed
	 */
	void RemoveItem(CItem *pItem);

	/**
	 * Remove an item from the inventory and delete it.
	 * @param pItem		Pointer to item to be discarded
	 */
	void DiscardItem(CItem *pItem);

	/**
	 * Reduce a quantity of an item from the inventory,
	 * discarding the item itself if the requested amount
	 * exceeds the supply on hand.
	 * @param pItem			Pointer to item to be affected
	 * @param nQuantity		Number of item units to be discarded
	 */
	void DiscardItem(CItem *pItem, long nQuantity);

	/**
	 * Discard the specified item from the inventory.
	 * @param nID	Identifier of item to be discarded
	 */
	void DiscardItem(int nID);

	/**
	 * Reduce a quantity of an item from the inventory,
	 * discarding the item itself if the requested amount
	 * exceeds the supply on hand.
	 * @param nID		Identifier of item to be affected
	 * @param nQuantity	Number of item units to be discarded
	 */
	void DiscardItem(int nID, long nQuantity);

	/**
	 * Retrieve an item from the inventory, and return it pointer.
	 * @param nIdx		Index of the item to look for
	 * @returns			Pointer to item or nullptr
	 */
	CItem *FetchItem(int nIdx);

	/**
	 * Look for an item in the inventory, and return it pointer.
	 * @param nID		Identifier of the item to look for
	 * @returns			Pointer to item or nullptr
	 */
	CItem *FindItem(int nID);

	CItem *FirstItem() const {
		return m_pEquipment;
	}

private:
	char  *m_lpsTitle;		// Title string for inventory
	int    m_nItemCount;	// Number of items in the inventory
	CItem *m_pEquipment;	// Linked list of inventory items
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
