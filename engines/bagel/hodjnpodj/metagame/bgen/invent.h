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

#ifndef BAGEL_METAGAME_BGEN_INVENT_H
#define BAGEL_METAGAME_BGEN_INVENT_H

#include "bagel/hodjnpodj/metagame/bgen/item.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define INVENT_MIN_ITEM_COUNT   2

class CInventory : public CObject {
	DECLARE_DYNCREATE(CInventory)

// Constructors
public:
	CInventory(const char *lpsTitle = nullptr);

// Destructors
public:
	~CInventory();

// Implementation
public:
	const char *GetTitle() const {
		return m_lpsTitle;
	}
	int ItemCount() const {
		return m_nItemCount;
	}
	void AddItem(CItem *pItem);
	void AddItem(int nID, long nQuantity);
	void RemoveItem(CItem *pItem);
	void DiscardItem(CItem *pItem);
	void DiscardItem(CItem *pItem, long nQuantity);
	void DiscardItem(int nID);
	void DiscardItem(int nID, long nQuantity);
	CItem *FetchItem(int nIdx);
	CItem *FindItem(int nID);
	CItem *FirstItem() const {
		return m_pEquipment;
	}

private:
	const char *m_lpsTitle; // title string for inventory
	int m_nItemCount;       // number of items in the inventory
	CItem *m_pEquipment;    // linked list of inventory items
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
