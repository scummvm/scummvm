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
#include "mutationofjb/gamescreen.h"

#include "common/algorithm.h"
#include "common/debug.h"

namespace MutationOfJB {

const Inventory::Items &Inventory::getItems() const {
	return _items;
}

bool Inventory::hasItem(const Common::String &item) const {
	Items::const_iterator it = Common::find(_items.begin(), _items.end(), item);
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
	Items::iterator it = Common::find(_items.begin(), _items.end(), item);
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

void Inventory::renameItem(const Common::String &oldName, const Common::String &newName) {
	bool renamed = false;
	for (Items::iterator it = _items.begin(); it != _items.end(); ++it) {
		if (*it == oldName) {
			*it = newName;
			renamed = true;
		}
	}
	if (renamed && _observer) {
		_observer->onInventoryChanged();
	}
}

void Inventory::scrollLeft() {
	if (_items.size() > VISIBLE_ITEMS) {
		rotateItemsRight(1);
	}
}

void Inventory::scrollRight() {
	if (_items.size() > VISIBLE_ITEMS) {
		rotateItemsLeft(1);
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
		SWAP(_items[from + i], _items[to - i]);
	}
}

void Inventory::saveLoadWithSerializer(Common::Serializer &sz) {
	if (sz.isLoading()) {
		uint32 length = 0;
		sz.syncAsUint32LE(length);
		_items.resize(length);
	} else {
		uint32 length = static_cast<uint32>(_items.size());
		sz.syncAsUint32LE(length);
	}

	for (Items::size_type i = 0; i < _items.size(); ++i) {
		sz.syncString(_items[i]);
	}
}

}
