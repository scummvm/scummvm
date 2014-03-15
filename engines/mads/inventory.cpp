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

void InventoryObject::load(Common::SeekableReadStream &f) {
	_descId = f.readUint16LE();
	_roomNumber = f.readUint16LE();
	_article = f.readByte();
	_vocabCount = f.readByte();
	
	for (int i = 0; i < 3; ++i) {
		_vocabList[i]._actionFlags1 = f.readByte();
		_vocabList[i]._actionFlags2 = f.readByte();
		_vocabList[i]._vocabId = f.readUint16LE();
	}

	f.skip(4);	// field12
	f.read(&_mutilateString[0], 10);
	f.skip(16);
}

/*------------------------------------------------------------------------*/

void InventoryObjects::load() {
	File f("*OBJECTS.DAT");

	// Get the total numer of inventory objects
	int count = f.readUint16LE();
	reserve(count);

	// Read in each object
	for (int i = 0; i < count; ++i) {
		InventoryObject obj;
		obj.load(f);
		push_back(obj);

		// If it's for the player's inventory, add the index to the inventory list
		if (obj._roomNumber == PLAYER_INVENTORY) {
			_inventoryList.push_back(i);
			assert(_inventoryList.size() <= 32);
		}
	}
}

void InventoryObjects::setData(int objIndex, int id, const byte *p) {
	// TODO: This whole method seems weird. Check it out more thoroughly once
	// more of the engine is implemented
	for (int i = 0; i < (int)size(); ++i) {
		InventoryObject &obj = (*this)[i];
		if (obj._vocabList[0]._actionFlags1 <= i)
			break;

		if (obj._mutilateString[6 + i] == id) {
			(*this)[objIndex]._objFolder = p;
		}
	}
}

void InventoryObjects::setRoom(int objectId, int roomNumber) {
	warning("TODO: setObjectRoom");
}

bool InventoryObjects::isInRoom(int objectId) const {
	return (*this)[objectId]._roomNumber == _vm->_game->_scene._currentSceneId;
}


} // End of namespace MADS
