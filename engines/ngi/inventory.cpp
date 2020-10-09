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

#include "ngi/ngi.h"

#include "ngi/utils.h"
#include "ngi/inventory.h"
#include "ngi/gameloader.h"
#include "ngi/statics.h"
#include "ngi/input.h"

namespace NGI {

bool Inventory::load(MfcArchive &file) {
	debugC(5, kDebugLoading | kDebugInventory, "Inventory::load()");

	_sceneId = file.readUint16LE();
	int numInvs = file.readUint32LE();

	_itemsPool.resize(numInvs);
	for (int i = 0; i < numInvs; i++) {
		InventoryPoolItem &t = _itemsPool[i];
		t.id = file.readUint16LE();
		t.pictureObjectNormal = file.readUint16LE();
		t.pictureObjectId1 = file.readUint16LE();
		t.pictureObjectHover = file.readUint16LE();
		t.pictureObjectSelected = file.readUint16LE();
		t.flags = file.readUint32LE();
		t.field_C = 0;
		t.field_A = -536;
	}

	return true;
}

int Inventory::getInventoryPoolItemIndexById(int itemId) {
	if (_itemsPool.size() <= 0)
		return -1;

	for (uint i = 0; i < _itemsPool.size(); i++) {
		if (_itemsPool[i].id == itemId)
			return i;
	}

	return 0;
}

bool Inventory::setItemFlags(int itemId, int flags) {
	int idx = getInventoryPoolItemIndexById(itemId);

	if (idx < 0)
		return false;
	else
		_itemsPool[idx].flags = flags;

	return true;
}

Inventory2::Inventory2() {
	_selectedId = -1;
	_field_48 = -1;
	_scene = 0;
	_picture = 0;
	_isInventoryOut = false;
	_isLocked = 0;
	_topOffset = -65;
}

Inventory2::~Inventory2() {
	removeMessageHandler(125, -1);
}

bool Inventory2::loadPartial(MfcArchive &file) { // Inventory2_SerializePartially
	_inventoryItems.clear();

	int numInvs = file.readUint32LE();

	for (int i = 0; i < numInvs; i++) {
		int16 itemId = file.readUint16LE();
		int16 count = file.readUint16LE();
		_inventoryItems.push_back(InventoryItem(itemId, count));
	}

	return true;
}

bool Inventory2::savePartial(MfcArchive &file) {
	file.writeUint32LE(_inventoryItems.size());

	for (uint i = 0; i < _inventoryItems.size(); i++) {
		file.writeUint16LE(_inventoryItems[i].itemId);
		file.writeUint16LE(_inventoryItems[i].count);
	}

	return true;
}

void Inventory2::addItem(int itemId, int count) {
	if (getInventoryPoolItemIndexById(itemId) >= 0)
		_inventoryItems.push_back(InventoryItem(itemId, count));
}

void Inventory2::addItem2(StaticANIObject *obj) {
	if (getInventoryPoolItemIndexById(obj->_id) >= 0 && getInventoryPoolItemFieldCById(obj->_id) != 2) {
		addItem(obj->_id, 1);
		obj->hide();
	}
}

void Inventory2::removeItem(int itemId, int count) {
	debugC(2, kDebugInventory, "Inventory2::removeItem(%d, %d)", itemId, count);

	while (count) {
		int i;
		for (i = _inventoryItems.size() - 1; i >= 0; i--) {
			InventoryItem &item = _inventoryItems[i];
			if (item.itemId == itemId) {
				if (_selectedId == itemId)
					unselectItem(false);

				if (item.count > count) {
					item.count -= count;
				} else {
					count -= item.count;
					_inventoryItems.remove_at(i);
				}

				if (getCountItemsWithId(itemId) < 0)
					getInventoryPoolItemFieldCById(itemId);

				break;
			}
		}
	}
}

void Inventory2::removeItem2(Scene *sceneObj, int itemId, int x, int y, int priority) {
	int idx = getInventoryItemIndexById(itemId);

	debugC(2, kDebugInventory, "removeItem2(*, %d, %d, %d, %d)", itemId, x, y, priority);

	if (idx >= 0) {
		if (_inventoryItems[idx].count) {
			removeItem(itemId, 1);

			Scene *sc = g_nmi->accessScene(_sceneId);

			if (sc) {
				StaticANIObject *ani = new StaticANIObject(sc->getStaticANIObject1ById(itemId, -1));

				sceneObj->addStaticANIObject(ani, 1);

				ani->_statics = ani->_staticsList[0];
				ani->setOXY(x, y);
				ani->_priority = priority;
			}
		}
	}
}

int Inventory2::getCountItemsWithId(int itemId) {
	int res = 0;

	for (uint i = 0; i < _inventoryItems.size(); i++) {
		if (_inventoryItems[i].itemId == itemId)
			res += _inventoryItems[i].count;
	}

	return res;
}

int Inventory2::getInventoryItemIndexById(int itemId) {
	for (uint i = 0; i < _inventoryItems.size(); i++) {
		if (_inventoryItems[i].itemId == itemId)
			return i;
	}

	return -1;
}

int Inventory2::getInventoryPoolItemIdAtIndex(int itemId) {
	return _itemsPool[itemId].id;
}

int Inventory2::getInventoryPoolItemFieldCById(int itemId) {
	for (uint i = 0; i < _itemsPool.size(); i++) {
		if (_itemsPool[i].id == itemId)
			return _itemsPool[i].field_C;
	}

	return 0;
}

int Inventory2::getItemFlags(int itemId) {
	int idx = getInventoryPoolItemIndexById(itemId);

	if (idx < 0)
		return 0;

	return _itemsPool[idx].flags;
}

void Inventory2::rebuildItemRects() {
	debugC(2, kDebugInventory, "rebuildItemRects()");

	_scene = g_nmi->accessScene(_sceneId);

	if (!_scene)
		return;

	_inventoryIcons.clear();

	_picture = _scene->getBigPicture(0, 0);
	_picture->setAlpha(50);

	int itemX = 9;
	int itemY = 0;

	for (uint i = 0; i < _scene->_picObjList.size(); i++) {
		PictureObject *pic = _scene->_picObjList[i];

		for (uint j = 0; j < _itemsPool.size(); j++) {
			if (_itemsPool[j].pictureObjectNormal == pic->_id) {
				if (pic->_odelay)
					_scene->deletePictureObject(pic);
				else
					pic->_flags &= 0xFFFB;
			}
		}
	}

	for (uint i = 0; i < _inventoryItems.size(); i++) {
		int idx = getInventoryPoolItemIndexById(_inventoryItems[i].itemId);

		_inventoryIcons.push_back(InventoryIcon());
		InventoryIcon &icn = _inventoryIcons.back();

		icn.inventoryItemId = _itemsPool[idx].id;

		icn.pictureObjectNormal = _scene->getPictureObjectById(_itemsPool[idx].pictureObjectNormal, 0);
		icn.pictureObjectHover = _scene->getPictureObjectById(_itemsPool[idx].pictureObjectHover, 0);
		icn.pictureObjectSelected = _scene->getPictureObjectById(_itemsPool[idx].pictureObjectSelected, 0);

		const Dims dims = icn.pictureObjectNormal->getDimensions();

		if (_itemsPool[idx].flags & 0x10000) {
			icn.x1 = 730;
			icn.y1 = itemY;
			icn.x2 = dims.x + 730;
			icn.y2 = dims.y + itemY + 10;
		} else {
			icn.x1 = itemX;
			icn.y1 = itemY;
			icn.x2 = itemX + dims.x;
			itemX = icn.x2 + 1;
			icn.y2 = dims.y + itemY + 10;
		}

		if (itemX >= 2 * (icn.x1 - icn.x2) + 800) {
			itemX = 9;
			itemY = icn.y2 + 1;
		}
	}
}

void Inventory2::draw() {
	if (!_scene)
		return;

	int oldScLeft = g_nmi->_sceneRect.left;
	int oldScTop = g_nmi->_sceneRect.top;

	g_nmi->_sceneRect.top = -_topOffset;
	g_nmi->_sceneRect.left = 0;

	_picture->draw(-1, -1, 0, 0);

	for (uint i = 0; i < _inventoryIcons.size(); i++) {
		const InventoryIcon &icn = _inventoryIcons[i];

		if (icn.isSelected) {
			icn.pictureObjectSelected->drawAt(icn.x1, icn.y1 + 10);
		} else {
			if (icn.isMouseHover)
				icn.pictureObjectHover->drawAt(icn.x1, icn.y1 + 10);
			else
				icn.pictureObjectNormal->drawAt(icn.x1, icn.y1 + 10);
		}
	}

	if (!_isInventoryOut)
		goto LABEL_30;

	int v10, v11, v12;

	if (_topOffset != -10) {
		if (_topOffset < -10) {
			v10 = -10;
			goto LABEL_13;
		}
		if (_topOffset + 10 >= 20) {
			v11 = -20;
cont:
			_topOffset += v11;
			goto reset;
		}
		v12 = -10;
		goto LABEL_25;
	}
	if (!_isInventoryOut) {
LABEL_30:
		if (_topOffset != -65) {
			if (_topOffset < -65) {
				v10 = -65;
LABEL_13:
				v11 = v10 - _topOffset;
				if (v11 >= 20)
					v11 = 20;
				goto cont;
			}
			if (_topOffset + 65 >= 20) {
				v11 = -20;
				goto cont;
			}
			v12 = -65;
LABEL_25:
			v11 = v12 - _topOffset;
			goto cont;
		}
	}

reset:

	g_nmi->_sceneRect.top = oldScTop;
	g_nmi->_sceneRect.left = oldScLeft;

}

void Inventory2::slideIn() {
	_isInventoryOut = false;

	ExCommand *ex = new ExCommand(0, 17, 65, 0, 0, 0, 1, 0, 0, 0);

	ex->_field_2C = 10;
	ex->_z = _isInventoryOut;
	ex->_invId = !_isInventoryOut;
	ex->_excFlags |= 3;
	ex->postMessage();
}

void Inventory2::slideOut() {
	_isInventoryOut = true;

	ExCommand *ex = new ExCommand(0, 17, 65, 0, 0, 0, 1, 0, 0, 0);

	ex->_field_2C = 10;
	ex->_z = _isInventoryOut;
	ex->_invId = !_isInventoryOut;
	ex->_excFlags |= 3;
	ex->postMessage();
}

bool Inventory2::handleLeftClick(ExCommand *cmd) {
	if (!_isInventoryOut)
		return false;

	bool res = false;

	for (uint i = 0; i < _inventoryIcons.size(); i++) {
		InventoryIcon &icon = _inventoryIcons[i];
		if (cmd->_x >= icon.x1 && cmd->_x <= icon.x2 &&
			cmd->_y >= icon.y1 && cmd->_y <= icon.y2) {
			if (getSelectedItemId()) {
				if (getSelectedItemId() != icon.inventoryItemId)
					unselectItem(0);
			}
			if (getItemFlags(icon.inventoryItemId) & 1) {
				ExCommand *ex = new ExCommand(0, 17, 65, 0, 0, 0, 1, 0, 0, 0);
				ex->_field_2C = 11;
				ex->_z = _inventoryIcons[i].inventoryItemId;
				ex->_excFlags |= 3;
				ex->postMessage();
			}
			if (!(getItemFlags(icon.inventoryItemId) & 2)) {
				selectItem(icon.inventoryItemId);
				icon.isSelected = true;
			}
			res = true;
		}
	}

	if (!res)
		unselectItem(0);

	return res;
}

int Inventory2::selectItem(int itemId) {
	if (getInventoryItemIndexById(itemId) < 0)
		return -1;

	unselectItem(0);

	_selectedId = itemId;

	if (_scene) {
		int idx = getInventoryPoolItemIndexById(itemId);

		Picture *pic = _scene->getPictureObjectById(_itemsPool[idx].pictureObjectId1, 0)->_picture;
		g_nmi->getGameLoaderInputController()->setCursorItemPicture(pic);
	}

	return _selectedId;
}

bool Inventory2::unselectItem(bool flag) {
	if (_selectedId < 0)
		return false;

	_selectedId = -1;

	for (uint i = 0; i < _inventoryIcons.size(); i++) {
		if (_inventoryIcons[i].isSelected)
			_inventoryIcons[i].isSelected = false;
	}

	g_nmi->getGameLoaderInputController()->setCursorItemPicture(0);

	return true;
}

int Inventory2::getHoveredItem(Common::Point *point) {
	int selId = getSelectedItemId();

	if (point->y <= 20 && !_isInventoryOut && !_isLocked)
		slideOut();

	if (!selId && point->y >= 55) {
		if (!_isInventoryOut)
			return 0;

		if (!_isLocked)
			slideIn();
	}

	if (!_isInventoryOut)
		return 0;

	int res = 0;

	for (uint i = 0; i < _inventoryIcons.size(); i++) {
		InventoryIcon &icn = _inventoryIcons[i];
		if (selId ||
			point->x < icn.x1 ||
			point->x > icn.x2 ||
			point->y < _topOffset + icn.y1 ||
			point->y > _topOffset + icn.y2) {
			icn.isMouseHover = false;
		} else {
			icn.isMouseHover = true;
			res = icn.inventoryItemId;
		}
	}

	return res;
}

void Inventory2::clear() {
	unselectItem(0);

	for (uint i = 0; i < _inventoryItems.size(); i++)
		getInventoryPoolItemFieldCById(_inventoryItems[i].itemId);

	_inventoryItems.clear();
}

void NGIEngine::getAllInventory() {
	Inventory2 *inv = getGameLoaderInventory();

	for (uint i = 0; i < inv->getItemsPoolCount(); ++i ) {
		int id = inv->getInventoryPoolItemIdAtIndex(i);

		if (inv->getCountItemsWithId(id) < 1)
			inv->addItem(id, 1);
	}

	inv->rebuildItemRects();
}

} // End of namespace NGI
