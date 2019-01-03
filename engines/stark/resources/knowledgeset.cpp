/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/knowledgeset.h"

#include "engines/stark/formats/xrc.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {
namespace Resources {

KnowledgeSet::~KnowledgeSet() {
}

KnowledgeSet::KnowledgeSet(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name) {
	_type = TYPE;
}

void KnowledgeSet::printData() {
}

Gfx::RenderEntryArray KnowledgeSet::getInventoryRenderEntries() const {
	Common::Array<Resources::Item *> inventoryItems = listChildren<Resources::Item>(Resources::Item::kItemInventory);

	// First add the inventory items from old saves which don't have an order
	Gfx::RenderEntryArray result;
	for (uint i = 0; i < inventoryItems.size(); i++) {
		// The first 4 elements are UI elements (Eye, Mouth, Hand, ...)
		if (i < 4 || !inventoryItems[i]->isEnabled()) continue;

		bool orderFound = false;
		for (uint j = 0; j < _inventoryItemOrder.size(); j++) {
			if (_inventoryItemOrder[j] == inventoryItems[i]->getIndex()) {
				orderFound = true;
				break;
			}
		}

		if (!orderFound) {
			result.push_back(inventoryItems[i]->getRenderEntry(Common::Point(0, 0)));
		}
	}

	// Then add the inventory items for which an order has been recorded
	for (uint i = 0; i < _inventoryItemOrder.size(); i++) {
		for (uint j = 0; j < inventoryItems.size(); j++) {
			if (inventoryItems[j]->isEnabled() && inventoryItems[j]->getIndex() == _inventoryItemOrder[i]) {
				result.push_back(inventoryItems[j]->getRenderEntry(Common::Point(0, 0)));
			}
		}
	}

	return result;
}

void KnowledgeSet::addItem(InventoryItem *item) {
	_inventoryItemOrder.push_back(item->getIndex());
}

void KnowledgeSet::removeItem(InventoryItem *item) {
	Common::Array<uint16>::iterator it;
	for (it = _inventoryItemOrder.begin(); it != _inventoryItemOrder.end(); it++) {
		if (*it == item->getIndex()) {
			_inventoryItemOrder.erase(it);
			break;
		}
	}
}

void KnowledgeSet::saveLoad(ResourceSerializer *serializer) {
	if (_subType == kInventory) {
		serializer->syncArraySize(_inventoryItemOrder);
		for (uint i = 0; i < _inventoryItemOrder.size(); i++) {
			serializer->syncAsUint16LE(_inventoryItemOrder[i]);
		}
	}
}

Visual *KnowledgeSet::getInventoryItemVisual(uint16 itemIndex) {
	InventoryItem *item = findChildWithIndex<InventoryItem>(itemIndex, Item::kItemInventory);
	assert(item);

	return item->getCursorVisual();
}

int16 KnowledgeSet::getNeighborInventoryItem(int16 selectedIndex, bool forward) {
	if (selectedIndex < 0 && !_inventoryItemOrder.empty()) {
		return forward ? _inventoryItemOrder.front() : _inventoryItemOrder.back();
	}

	for (uint i = 0; i < _inventoryItemOrder.size(); ++i) {
		if (_inventoryItemOrder[i] == selectedIndex) {
			if ((i == 0 && !forward) || (i == _inventoryItemOrder.size() - 1 && forward)) {
				return -1;
			} else {
				i += forward ? 1 : -1;
				return _inventoryItemOrder[i];
			}
		}
	}

	return -1;
}

} // End of namespace Resources
} // End of namespace Stark
