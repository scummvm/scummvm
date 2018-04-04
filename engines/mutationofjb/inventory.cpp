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

#include "mutationofjb/inventory.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gui.h"
#include "common/algorithm.h"
#include "common/debug.h"

namespace MutationOfJB {

const Inventory::Items &Inventory::getItems() const {
	return _items;
}

bool Inventory::hasItem(const Common::String &item) const {
	Items::const_iterator it = find(_items.begin(), _items.end(), item);
	return (it != _items.end());
}

void Inventory::addItem(const Common::String &item) {
	_items.push_back(item);

	if (_items.size() > VISIBLE_ITEMS) {
		rotateItemsRight(VISIBLE_ITEMS);
	}
	if (_observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::removeItem(const Common::String &item) {
	Items::iterator it = find(_items.begin(), _items.end(), item);
	if (it == _items.end()) {
		debug("Item '%s' not in inventory.", item.c_str());
		return;
	}

	_items.remove_at(it - _items.begin());
	if (_observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::removeAllItems() {
	_items.clear();
	if (_observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::rotateItemsRight(uint n) {
	if (_items.size() < 2) {
		return;
	}

	n %= _items.size();
	reverseItems(0, _items.size() - 1);
	reverseItems(0, n - 1);
	reverseItems(n, _items.size() - 1);
	if (_observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::rotateItemsLeft(uint n) {
	if (_items.size() < 2) {
		return;
	}

	n %= _items.size();
	reverseItems(0, _items.size() - 1);
	reverseItems(_items.size() - n, _items.size() - 1);
	reverseItems(0, _items.size() - n - 1);
	if (_observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::setObserver(InventoryObserver *observer) {
	_observer = observer;
}

void Inventory::reverseItems(uint from, uint to) {
	assert(from <= to);
	if (from == to) {
		return;
	}

	const uint size = to - from + 1;
	for (uint i = 0; i < size / 2; ++i) {
		SWAP(_items[i], _items[size - i - 1]);
	}
}

}
