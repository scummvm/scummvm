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

#include "fullpipe/utils.h"
#include "fullpipe/inventory.h"
#include "fullpipe/gameloader.h"

namespace Fullpipe {

bool CInventory::load(MfcArchive &file) {
	debug(5, "CInventory::load()");

	_sceneId = file.readUint16LE();
	int numInvs = file.readUint32LE();

	for (int i = 0; i < numInvs; i++) {
		InventoryPoolItem *t = new InventoryPoolItem();
		t->id = file.readUint16LE();
		t->pictureObjectNormalId = file.readUint16LE();
		t->pictureObjectId1 = file.readUint16LE();
		t->pictureObjectMouseHover = file.readUint16LE();
		t->pictureObjectId3 = file.readUint16LE();
		t->flags = file.readUint32LE();
		t->field_C = 0;
		t->field_A = -536;
		_itemsPool.push_back(t);
	}

	return true;
}

int CInventory::getInventoryPoolItemIndexById(int itemId) {
	if (_itemsPool.size() <= 0)
		return -1;

	for (uint i = 0; i < _itemsPool.size(); i++) {
		if (_itemsPool[i]->id == itemId)
			return i;
	}

	return 0;
}

bool CInventory::setItemFlags(int itemId, int flags) {
	int idx = getInventoryPoolItemIndexById(itemId);

	if (idx < 0)
		return false;
	else
		_itemsPool[idx]->flags = flags;

	return true;
}

CInventory2::CInventory2() {
	_selectedId = -1;
	_field_48 = -1;
	_scene = 0;
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
		_inventoryItems.push_back(t);
	}

	return true;
}

void CInventory2::addItem(int itemId, int count) {
	if (getInventoryPoolItemIndexById(itemId) >= 0)
		_inventoryItems.push_back(new InventoryItem(itemId, count));
}

void CInventory2::rebuildItemRects() {
	_scene = g_fullpipe->accessScene(_sceneId);

	if (!_scene)
		return;

	_picture = _scene->getBigPicture(0, 0);
	_picture->setAlpha(50);

	int itemX = 9;
	int itemY = 0;

	for (uint i = 0; i < _scene->_picObjList.size(); i++) {
		PictureObject *pic = (PictureObject *)_scene->_picObjList[i];

		for (uint j = 0; j < _itemsPool.size(); j++) {
			if (_itemsPool[j]->pictureObjectNormalId == pic->_id) {
				if (pic->_okeyCode)
					_scene->deletePictureObject(pic);
				else
					pic->_flags &= 0xFFFB;
			}
		}
	}

	for (uint i = 0; i < _inventoryItems.size(); i++) {
		Common::Point point;

		int idx = getInventoryPoolItemIndexById(_inventoryItems[i]->itemId);

		InventoryIcon *icn = new InventoryIcon();

		icn->inventoryItemId = _itemsPool[idx]->id;
		
		icn->pictureObjectNormal = _scene->getPictureObjectById(_itemsPool[idx]->pictureObjectNormalId, 0);
		icn->pictureObjectHover = _scene->getPictureObjectById(_itemsPool[idx]->pictureObjectMouseHover, 0);
		icn->pictureObjectId3 = _scene->getPictureObjectById(_itemsPool[idx]->pictureObjectId3, 0);

		icn->pictureObjectNormal->getDimensions(&point);

		if (_itemsPool[idx]->flags & 0x10000) {
			icn->x1 = 730;
			icn->y1 = itemY;
			icn->x2 = point.x + 730;
			icn->y2 = point.y + itemY + 10;
		} else {
			icn->x1 = itemX;
			icn->y1 = itemY;
			icn->x2 = itemX + point.x;
			itemX = icn->x2 + 1;
			icn->y2 = point.y + itemY + 10;
		}

		_inventoryIcons.push_back(icn);

		if (itemX >= 2 * (icn->x1 - icn->x2) + 800) {
			itemX = 9;
			itemY = icn->y2 + 1;
		}
	}
}

void CInventory2::draw() {
	warning("STUB: CInventory2::draw()");
}

void CInventory2::slideIn() {
	_isInventoryOut = false;

	ExCommand *ex = new ExCommand(0, 17, 65, 0, 0, 0, 1, 0, 0, 0);

	ex->_field_2C = 10;
	ex->_field_14 = _isInventoryOut;
	ex->_field_20 = !_isInventoryOut;
	ex->_excFlags |= 3;
	ex->postMessage();
}

void CInventory2::slideOut() {
	_isInventoryOut = true;

	ExCommand *ex = new ExCommand(0, 17, 65, 0, 0, 0, 1, 0, 0, 0);

	ex->_field_2C = 10;
	ex->_field_14 = _isInventoryOut;
	ex->_field_20 = !_isInventoryOut;
	ex->_excFlags |= 3;
	ex->postMessage();
}

int CInventory2::handleLeftClick(ExCommand *cmd) {
	warning("STUB: CInventory2::handleLeftClick()");

	return 0;
}

int CInventory2::unselectItem(bool flag) {
	warning("STUB: CInventory2::unselectItem()");

	return 0;
}

int CInventory2::getHoveredItem(Common::Point *point) {
	warning("STUB: CInventory2::getHoveredItem()");

	return 0;
}

} // End of namespace Fullpipe
