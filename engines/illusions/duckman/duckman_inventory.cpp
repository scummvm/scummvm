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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/duckman_inventory.h"
#include "illusions/actor.h"
#include "illusions/cursor.h"
#include "illusions/input.h"
#include "illusions/resources/scriptresource.h"
#include "engines/util.h"

namespace Illusions {

// DuckmanInventory

DuckmanInventory::DuckmanInventory(IllusionsEngine_Duckman *vm)
	: _vm(vm) {
	initInventory();
}

DuckmanInventory::~DuckmanInventory() {
}

static const struct DMInventoryItem kInventoryItems[21] = {
	{0x40011, 0xE005B},
	{0x40099, 0xE001B},
	{0x4000F, 0xE000C},
	{0x40042, 0xE0012},
	{0x40044, 0xE000F},
	{0x40029, 0xE000D},
	{0x400A7, 0xE005D},
	{0x40096, 0xE001C},
	{0x40077, 0xE0010},
	{0x4008A, 0xE0033},
	{0x4004B, 0xE0045},
	{0x40054, 0xE0021},
	{0x400C6, 0xE005A},
	{0x4000B, 0xE005E},
	{0x4005F, 0xE0016},
	{0x40072, 0xE0017},
	{0x400AA, 0xE005F},
	{0x400B8, 0xE0050},
	{0x4001F, 0xE001A},
	{0x40095, 0xE0060},
	{0x40041, 0xE0053}
};

void DuckmanInventory::initInventory() {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 5; x++) {
			_inventorySlots.push_back(DMInventorySlot( 64 + x * 48,  52 + y * 32));
		}
	}

	for (int i = 0; i < 21; i++) {
		_inventoryItems.push_back(kInventoryItems[i]);
	}
}

void DuckmanInventory::openInventory() {

	for (uint i = 0; i < _inventorySlots.size(); ++i) {
		DMInventorySlot *inventorySlot = &_inventorySlots[i];
		if (inventorySlot->_objectId) {
			DMInventoryItem *inventoryItem = findInventoryItem(inventorySlot->_objectId);
			if (!_vm->_scriptResource->_properties.get(inventoryItem->_propertyId))
				inventorySlot->_objectId = 0;
		}
	}

	for (uint i = 0; i < _inventoryItems.size(); ++i) {
		DMInventoryItem *inventoryItem = &_inventoryItems[i];
		if (_vm->_scriptResource->_properties.get(inventoryItem->_propertyId)) {
			DMInventorySlot *inventorySlot = findInventorySlot(inventoryItem->_objectId);
			if (inventorySlot) {
				Control *control = _vm->getObjectControl(inventoryItem->_objectId);
				control->setActorPosition(inventorySlot->_position);
				control->appearActor();
			} else {
				addInventoryItem(inventoryItem->_objectId);
			}
		}
	}

}

void DuckmanInventory::addInventoryItem(uint32 objectId) {
	DMInventorySlot *DMInventorySlot = findInventorySlot(0);
	DMInventorySlot->_objectId = objectId;
	Control *control = _vm->getObjectControl(objectId);
	control->setActorPosition(DMInventorySlot->_position);
	control->appearActor();
}

void DuckmanInventory::clearInventorySlot(uint32 objectId) {
	for (uint i = 0; i < _inventorySlots.size(); ++i) {
		if (_inventorySlots[i]._objectId == objectId)
			_inventorySlots[i]._objectId = 0;
	}
}

void DuckmanInventory::putBackInventoryItem() {
	Common::Point mousePos = _vm->_input->getCursorPosition();
	if (_vm->_cursor._objectId) {
		DMInventorySlot *inventorySlot = findInventorySlot(_vm->_cursor._objectId);
		if (inventorySlot)
			inventorySlot->_objectId = 0;
		inventorySlot = findClosestInventorySlot(mousePos);
		inventorySlot->_objectId = _vm->_cursor._objectId;
		Control *control = _vm->getObjectControl(_vm->_cursor._objectId);
		control->setActorPosition(inventorySlot->_position);
		control->appearActor();
		_vm->_cursor._actorIndex = 7;
		_vm->stopCursorHoldingObject();
		_vm->_cursor._actorIndex = 2;
		_vm->_cursor._control->startSequenceActor(_vm->_cursor._sequenceId1, 2, 0);
		if (_vm->_cursor._currOverlappedControl)
			_vm->setCursorActorIndex(_vm->_cursor._actorIndex, 2, 0);
		else
			_vm->setCursorActorIndex(_vm->_cursor._actorIndex, 1, 0);
	}
}

DMInventorySlot *DuckmanInventory::findInventorySlot(uint32 objectId) {
	for (uint i = 0; i < _inventorySlots.size(); ++i) {
		if (_inventorySlots[i]._objectId == objectId)
			return &_inventorySlots[i];
	}
	return 0;
}

DMInventoryItem *DuckmanInventory::findInventoryItem(uint32 objectId) {
	for (uint i = 0; i < _inventoryItems.size(); ++i) {
		if (_inventoryItems[i]._objectId == objectId)
			return &_inventoryItems[i];
	}
	return 0;
}

DMInventorySlot *DuckmanInventory::findClosestInventorySlot(Common::Point pos) {
	int minDistance = 0xFFFFFF;
	DMInventorySlot *minInventorySlot = 0;
	for (uint i = 0; i < _inventorySlots.size(); ++i) {
		DMInventorySlot *inventorySlot = &_inventorySlots[i];
		if (inventorySlot->_objectId == 0) {
			int16 deltaX = ABS(inventorySlot->_position.x - pos.x);
			int16 deltaY = ABS(inventorySlot->_position.y - pos.y);
			int distance = deltaX * deltaX + deltaY * deltaY;
			if (inventorySlot->_objectId == 0 && distance < minDistance) {
				minDistance = distance;
				minInventorySlot = inventorySlot;
			}
		}
	}
	return minInventorySlot;
}

} // End of namespace Illusions
