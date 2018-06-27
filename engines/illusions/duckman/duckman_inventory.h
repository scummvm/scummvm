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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_DUCKMAN_DUCKMAN_INVENTORY_H
#define ILLUSIONS_DUCKMAN_DUCKMAN_INVENTORY_H

#include "illusions/illusions.h"

namespace Illusions {

struct DMInventorySlot {
	Common::Point _position;
	uint32 _objectId;
	DMInventorySlot() : _objectId(0) {}
	DMInventorySlot(int16 x, int16 y) : _objectId(0), _position(x, y) {}
};

struct DMInventoryItem {
	uint32 _objectId;
	uint32 _propertyId;
};

class DuckmanInventory {
public:
	DuckmanInventory(IllusionsEngine_Duckman *vm);
	~DuckmanInventory();
public:
	IllusionsEngine_Duckman *_vm;
	Common::Array<DMInventorySlot> _inventorySlots;
	Common::Array<DMInventoryItem> _inventoryItems;
	void initInventory();
	void openInventory();
	void addInventoryItem(uint32 objectId);
	void clearInventorySlot(uint32 objectId);
	void putBackInventoryItem();
	DMInventorySlot *findInventorySlot(uint32 objectId);
	DMInventoryItem *findInventoryItem(uint32 objectId);
	DMInventorySlot *findClosestInventorySlot(Common::Point pos);
};

} // End of namespace Illusions

#endif // ILLUSIONS_DUCKMAN_DUCKMAN_INVENTORY_H
