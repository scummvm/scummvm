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

#ifndef MUTATIONOFJB_INVENTORY_H
#define MUTATIONOFJB_INVENTORY_H

#include "common/array.h"
#include "common/serializer.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace MutationOfJB {

class Game;

class InventoryObserver {
public:
	virtual void onInventoryChanged() = 0;
	virtual ~InventoryObserver() {}
};

class Inventory : public Common::Serializable {
public:
	enum {
		VISIBLE_ITEMS = 6
	};

	typedef Common::Array<Common::String> Items;

	Inventory() : _observer(nullptr) {}

	const Items &getItems() const;
	bool hasItem(const Common::String &item) const;
	void addItem(const Common::String &item);
	void removeItem(const Common::String &item);
	void removeAllItems();
	void renameItem(const Common::String &oldName, const Common::String &newName);

	void scrollLeft();
	void scrollRight();

	void setObserver(InventoryObserver *observer);

	void saveLoadWithSerializer(Common::Serializer &sz) override;

private:
	void rotateItemsRight(uint n);
	void rotateItemsLeft(uint n);
	void reverseItems(uint from, uint to);

	Items _items;
	InventoryObserver *_observer;
};

}

#endif
