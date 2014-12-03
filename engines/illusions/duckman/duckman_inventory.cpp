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

void DuckmanInventory::initInventory() {
	_inventorySlots.push_back(DMInventorySlot( 64,  52));
	_inventorySlots.push_back(DMInventorySlot(112,  52));
	_inventorySlots.push_back(DMInventorySlot(160,  52));
	_inventorySlots.push_back(DMInventorySlot(208,  52));
	_inventorySlots.push_back(DMInventorySlot(255,  52));
	_inventorySlots.push_back(DMInventorySlot( 64,  84));
	_inventorySlots.push_back(DMInventorySlot(112,  84));
	_inventorySlots.push_back(DMInventorySlot(160,  84));
	_inventorySlots.push_back(DMInventorySlot(208,  84));
	_inventorySlots.push_back(DMInventorySlot(255,  84));
	_inventorySlots.push_back(DMInventorySlot( 64, 116));
	_inventorySlots.push_back(DMInventorySlot(112, 116));
	_inventorySlots.push_back(DMInventorySlot(160, 116));
	_inventorySlots.push_back(DMInventorySlot(208, 116));
	_inventorySlots.push_back(DMInventorySlot(255, 116));
	_inventorySlots.push_back(DMInventorySlot( 64, 148));
	_inventorySlots.push_back(DMInventorySlot(112, 148));
	_inventorySlots.push_back(DMInventorySlot(160, 148));
	_inventorySlots.push_back(DMInventorySlot(208, 148));
	_inventorySlots.push_back(DMInventorySlot(255, 148));
	_inventoyItems.push_back(DMInventoryItem(0x40011, 0xE005B));
	_inventoyItems.push_back(DMInventoryItem(0x40099, 0xE001B));
	_inventoyItems.push_back(DMInventoryItem(0x4000F, 0xE000C));
	_inventoyItems.push_back(DMInventoryItem(0x40042, 0xE0012));
	_inventoyItems.push_back(DMInventoryItem(0x40044, 0xE000F));
	_inventoyItems.push_back(DMInventoryItem(0x40029, 0xE000D));
	_inventoyItems.push_back(DMInventoryItem(0x400A7, 0xE005D));
	_inventoyItems.push_back(DMInventoryItem(0x40096, 0xE001C));
	_inventoyItems.push_back(DMInventoryItem(0x40077, 0xE0010));
	_inventoyItems.push_back(DMInventoryItem(0x4008A, 0xE0033));
	_inventoyItems.push_back(DMInventoryItem(0x4004B, 0xE0045));
	_inventoyItems.push_back(DMInventoryItem(0x40054, 0xE0021));
	_inventoyItems.push_back(DMInventoryItem(0x400C6, 0xE005A));
	_inventoyItems.push_back(DMInventoryItem(0x4000B, 0xE005E));
	_inventoyItems.push_back(DMInventoryItem(0x4005F, 0xE0016));
	_inventoyItems.push_back(DMInventoryItem(0x40072, 0xE0017));
	_inventoyItems.push_back(DMInventoryItem(0x400AA, 0xE005F));
	_inventoyItems.push_back(DMInventoryItem(0x400B8, 0xE0050));
	_inventoyItems.push_back(DMInventoryItem(0x4001F, 0xE001A));
	_inventoyItems.push_back(DMInventoryItem(0x40095, 0xE0060));
	_inventoyItems.push_back(DMInventoryItem(0x40041, 0xE0053));
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

	for (uint i = 0; i < _inventoyItems.size(); ++i) {
		DMInventoryItem *inventoryItem = &_inventoyItems[i];
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
	for (uint i = 0; i < _inventorySlots.size(); ++i)
		if (_inventorySlots[i]._objectId == objectId)
			_inventorySlots[i]._objectId = 0;
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
	for (uint i = 0; i < _inventorySlots.size(); ++i)
		if (_inventorySlots[i]._objectId == objectId)
			return &_inventorySlots[i];
	return 0;
}

DMInventoryItem *DuckmanInventory::findInventoryItem(uint32 objectId) {
	for (uint i = 0; i < _inventoyItems.size(); ++i)
		if (_inventoyItems[i]._objectId == objectId)
			return &_inventoyItems[i];
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
