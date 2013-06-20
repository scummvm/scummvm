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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"

namespace Fullpipe {

bool CInventory::load(MfcArchive &file) {
	_sceneId = file.readUint16LE();
	int numInvs = file.readUint32LE();

	for (int i = 0; i < numInvs; i++) {
		InventoryPoolItem *t = new InventoryPoolItem();
		t->id = file.readUint16LE();
		t->pictureObjectNormalId = file.readUint16LE();
		t->pictureObjectId1 = file.readUint16LE();
		t->pictureObjectMouseInsideId = file.readUint16LE();
		t->pictureObjectId3 = file.readUint16LE();
		t->flags = file.readUint32LE();
		t->field_C = 0;
		t->field_A = -536;
		_itemsPool.push_back(*t);
	}

	return true;
}

int CInventory::getInventoryPoolItemIndexById(int itemId) {
	for (int i = 0; i < _itemsPool.size(); i++) {
		if (_itemsPool[i].id == itemId)
			return i;
	}

	return 0;
}

CInventory2::CInventory2() {
	_selectedId = -1;
	_field_48 = -1;
	_sceneObj = 0;
	_picture = 0;
	_isInventoryOut = 0;
	_isLocked = 0;
	_topOffset = -65;
}

bool CInventory2::loadPartial(MfcArchive &file) { // CInventory2_SerializePartially
	int numInvs = file.readUint32LE();

	for (int i = 0; i < numInvs; i++) {
		InventoryItem *t = new InventoryItem();
		t->itemId = file.readUint16LE();
		t->count = file.readUint16LE();
		_inventoryItems.push_back(*t);
	}

	return true;
}

} // End of namespace Fullpipe
