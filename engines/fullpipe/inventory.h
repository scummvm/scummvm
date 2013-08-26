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

#ifndef FULLPIPE_INVENTORY_H
#define FULLPIPE_INVENTORY_H

namespace Fullpipe {

class Scene;
class BigPicture;

struct InventoryPoolItem {
	int16 id;
	int16 pictureObjectNormal;
	int16 pictureObjectId1;
	int16 pictureObjectHover;
	int16 pictureObjectSelected;
	int16 field_A;
	int field_C;
	int obj;
	int flags;
};

typedef Common::Array<InventoryPoolItem *> InventoryPoolItems;

class CInventory : public CObject {
 protected:
	int16 _sceneId;
	InventoryPoolItems _itemsPool;

 public:
	CInventory() { _sceneId = 0; }
	virtual bool load(MfcArchive &file);

	int getInventoryPoolItemIndexById(int itemId);
	bool setItemFlags(int itemId, int flags);
};

struct InventoryItem {
	int16 itemId;
	int16 count;

	InventoryItem() { itemId = count = 0; }
	InventoryItem(int id, int cnt) : itemId(id), count(cnt) {}
};

typedef Common::Array<InventoryItem *> InventoryItems;

class PictureObject;

struct InventoryIcon {
	PictureObject *pictureObjectNormal;
	PictureObject *pictureObjectHover;
	PictureObject *pictureObjectSelected;
	int x1;
	int y1;
	int x2;
	int y2;
	int16 inventoryItemId;
	int isSelected;
	int isMouseHover;
};

typedef Common::Array<InventoryIcon *> InventoryIcons;

class CInventory2 : public CInventory {
	InventoryItems _inventoryItems;
	InventoryIcons _inventoryIcons;
	int _selectedId;
	int _field_48;
	bool _isInventoryOut;
	bool _isLocked;
	int _topOffset;
	Scene *_scene;
	BigPicture *_picture;

 public:
	CInventory2();
	bool loadPartial(MfcArchive &file);
	void addItem(int itemId, int count);
	void addItem2(StaticANIObject *obj);
	void removeItem(int itemId, int count);
	void removeItem2(Scene *sceneObj, int itemId, int x, int y, int priority);

	int getInventoryPoolItemFieldCById(int itemId);
	int getCountItemsWithId(int itemId);

	void rebuildItemRects();

	Scene *getScene() { return _scene; }
	bool getIsLocked() { return _isLocked; }
	void setIsLocked(bool val) { _isLocked = val; }
	bool getIsInventoryOut() { return _isInventoryOut; }

	int getSelectedItemId() { return _selectedId < 0 ? 0 : _selectedId; }
	int getHoveredItem(Common::Point *point);
	void slideIn();
	void slideOut();

	int handleLeftClick(ExCommand *cmd);
	int unselectItem(bool flag);

	void draw();
};

} // End of namespace Fullpipe

#endif /* FULLPIPE_INVENTORY_H */
