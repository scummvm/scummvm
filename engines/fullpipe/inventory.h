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
 public:
	int16 id;
	int16 pictureObjectNormalId;
	int16 pictureObjectId1;
	int16 pictureObjectMouseInsideId;
	int16 pictureObjectId3;
	int16 field_A;
	int field_C;
	int obj;
	int flags;
};

typedef Common::Array<InventoryPoolItem> InventoryPoolItems;

class CInventory : public CObject {
 protected:
	int16 _sceneId;
	int16 _field_6;
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
};

typedef Common::Array<InventoryItem> InventoryItems;

class PictureObject;

class InventoryIcon {
	PictureObject *_pictureObjectNormal;
	InventoryIcon **_icons;
	int _numIcons;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
	int16 _inventoryItemId;
	int16 _field_1E;
	int _isSelected;
	int _isMouseInside;
};

typedef Common::Array<InventoryIcon> InventoryIcons;

class CInventory2 : public CInventory {
	InventoryItems _inventoryItems;
	InventoryIcons _inventoryIcons;
	int _selectedId;
	int _field_48;
	int _isInventoryOut;
	int _isLocked;
	int _topOffset;
	Scene *_scene;
	BigPicture *_picture;

 public:
	CInventory2();
	bool loadPartial(MfcArchive &file);
	void addItem(int itemId, int value);
	void rebuildItemRects();

	Scene *getScene() { return _scene; }

};

CInventory2 *getGameLoaderInventory();

} // End of namespace Fullpipe

#endif /* FULLPIPE_INVENTORY_H */
