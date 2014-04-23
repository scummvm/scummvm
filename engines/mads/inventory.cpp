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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/inventory.h"

namespace MADS {

void InventoryObject::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_descId);
	s.syncAsUint16LE(_roomNumber);
	s.syncAsByte(_article);
	s.syncAsByte(_vocabCount);
	
	for (int i = 0; i < 3; ++i) {
		s.syncAsByte(_vocabList[i]._verbType);
		s.syncAsByte(_vocabList[i]._prepType);
		s.syncAsUint16LE( _vocabList[i]._vocabId);
	}

	s.skip(4);	// field12
	s.syncBytes((byte *)&_mutilateString[0], 10);
	s.skip(16);
}

/*------------------------------------------------------------------------*/

void InventoryObjects::load() {
	File f("*OBJECTS.DAT");
	Common::Serializer s(&f, nullptr);

	// Load the objects data
	synchronize(s);
}

void InventoryObjects::synchronize(Common::Serializer &s) {
	int count = size();
	s.syncAsUint16LE(count);

	if (s.isSaving()) {
		// Store the data for each object in the inventory lsit
		for (int idx = 0; idx < count; ++idx)
			(*this)[idx].synchronize(s);
	} else {
		clear();
		_inventoryList.clear();
		reserve(count);

		// Read in each object
		for (int i = 0; i < count; ++i) {
			InventoryObject obj;
			obj.synchronize(s);
			push_back(obj);

			// If it's for the player's inventory, add the index to the inventory list
			if (obj._roomNumber == PLAYER_INVENTORY) {
				_inventoryList.push_back(i);
				assert(_inventoryList.size() <= 32);
			}
		}
	}
}

void InventoryObjects::setQuality(int objIndex, int id, const byte *p) {
	// TODO: This whole method seems weird. Check it out more thoroughly once
	// more of the engine is implemented
	for (int i = 0; i < (int)size(); ++i) {
		InventoryObject &obj = (*this)[i];
		if (obj._vocabList[0]._verbType <= i)
			break;

		if (obj._mutilateString[6 + i] == id) {
			(*this)[objIndex]._objFolder = p;
		}
	}
}

void InventoryObjects::setRoom(int objectId, int sceneNumber) {
	InventoryObject &obj = (*this)[objectId];

	if (obj._roomNumber == PLAYER_INVENTORY)
		removeFromInventory(objectId, 1);

	if (sceneNumber == PLAYER_INVENTORY)
		addToInventory(objectId);
	else
		obj._roomNumber = sceneNumber;
}

bool InventoryObjects::isInRoom(int objectId) const {
	return (*this)[objectId]._roomNumber == _vm->_game->_scene._currentSceneId;
}

bool InventoryObjects::isInInventory(int objectId) const {
	return (*this)[objectId]._roomNumber == PLAYER_INVENTORY;
}

void InventoryObjects::addToInventory(int objectId) {
	assert(_inventoryList.size() < 32);
	UserInterface &userInterface = _vm->_game->_scene._userInterface;

	if (!isInInventory(objectId)) {
		_inventoryList.push_back(objectId);
		userInterface._selectedInvIndex = _inventoryList.size() - 1;
		userInterface._inventoryTopIndex = CLIP(userInterface._inventoryTopIndex,
			0, (int)_inventoryList.size() - 1);

		if ((userInterface._inventoryTopIndex + 5) <= (int)_inventoryList.size())
			userInterface._inventoryTopIndex = size() - 4;
		userInterface._inventoryChanged = true;

		(*this)[objectId]._roomNumber = PLAYER_INVENTORY;

		if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE && 
				_vm->_game->_screenObjects._inputMode == kInputBuildingSentences) {
			userInterface.categoryChanged();
			userInterface.selectObject(userInterface._selectedInvIndex);
		}
	}
}

void InventoryObjects::removeFromInventory(int objectId, int newScene) {
	Scene &scene = _vm->_game->_scene;
	UserInterface &userInterface = scene._userInterface;

	// Scan the inventory list for the object
	int invIndex = -1;
	for (int idx = 0; idx < (int)_inventoryList.size() && invIndex == -1; ++idx) {
		if (_inventoryList[idx] == objectId)
			invIndex = idx;
	}

	int selectedIndex = userInterface._selectedInvIndex;
	bool noSelection = selectedIndex < 0;

	if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE && 
			_vm->_game->_screenObjects._inputMode == kInputBuildingSentences)
		userInterface.selectObject(-1);

	// Remove the item from the inventory list
	_inventoryList.remove_at(invIndex);

	if (invIndex > userInterface._inventoryTopIndex) {
		userInterface._inventoryTopIndex = MAX(userInterface._inventoryTopIndex, 0);
	}

	userInterface._inventoryChanged = true;
	(*this)[objectId]._roomNumber = newScene;

	int newIndex = selectedIndex;
	if (!noSelection) {
		if (newIndex >= invIndex)
			--newIndex;
		if (newIndex < 0 && size() > 0)
			newIndex = 0;
	}

	if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE && 
			_vm->_game->_screenObjects._inputMode == kInputBuildingSentences) {
		userInterface.categoryChanged();
		userInterface.selectObject(newIndex);
	}
}

int InventoryObjects::getIdFromDesc(int descId) {
	for (int i = 0; i < (int)size(); ++i) {
		InventoryObject &obj = (*this)[i];
		if (obj._descId == descId)
			return i;
	}

	return -1;
}

} // End of namespace MADS
