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

#include "xeen/items.h"
#include "xeen/resources.h"

namespace Xeen {

XeenItem::XeenItem() {
	clear();
}

void XeenItem::clear() {
	_material = _id = _bonusFlags = 0;
	_frame = 0;
}

void XeenItem::synchronize(Common::Serializer &s) {
	s.syncAsByte(_material);
	s.syncAsByte(_id);
	s.syncAsByte(_bonusFlags);
	s.syncAsByte(_frame);
}

int XeenItem::getElementalCategory() const {
	int idx;
	for (idx = 0; ELEMENTAL_CATEGORIES[idx] < _material; ++idx)
		;

	return idx;
}

int XeenItem::getAttributeCategory() const {
	int m = _material - 59;
	int idx;
	for (idx = 0; ATTRIBUTE_CATEGORIES[idx] < m; ++idx)
		;

	return idx;
}

/*------------------------------------------------------------------------*/

InventoryItems::InventoryItems() : Common::Array<XeenItem>((XeenItem *)nullptr, INV_ITEMS_TOTAL) {
}

void InventoryItems::discardItem(int itemIndex) {
	operator[](itemIndex).clear();
	sort();
}

void InventoryItems::sort() {
	for (uint idx = 0; idx < size(); ++idx) {
		if (operator[](idx)._id == 0) {
			// Found empty slot
			operator[](idx).clear();

			// Scan through the rest of the list to find any item
			for (uint idx2 = idx + 1; idx2 < size(); ++idx2) {
				if (operator[](idx2)._id) {
					// Found an item, so move it into the blank slot
					operator[](idx) = operator[](idx2);
					operator[](idx2).clear();
					break;
				}
			}
		}
	}
}

void InventoryItems::equipItem(int itemIndex) {
	error("TODO");
}

void InventoryItems::removeItem(int itemIndex) {
	error("TODO");
}

/*------------------------------------------------------------------------*/

InventoryItemsGroup::InventoryItemsGroup(InventoryItems &weapons, InventoryItems &armor,
		InventoryItems &accessories, InventoryItems &misc) {
	_itemSets[0] = &weapons;
	_itemSets[1] = &armor;
	_itemSets[2] = &accessories;
	_itemSets[3] = &misc;
}

InventoryItems &InventoryItemsGroup::operator[](ItemCategory category) {
	return *_itemSets[category];
}

/*------------------------------------------------------------------------*/

Treasure::Treasure() {
	_hasItems = false;
}

} // End of namespace Xeen
