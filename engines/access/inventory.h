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

#ifndef ACCESS_INVENTORY_H
#define ACCESS_INVENTORY_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "access/data.h"
#include "access/asurface.h"

namespace Access {

enum ItemState {
	ITEM_NOT_FOUND = 0, ITEM_IN_INVENTORY = 1, ITEM_USED = 2
};

class InventoryEntry {
public:
	Common::String _name;
	int _value;

	int _otherItem1;
	int _newItem1;
	int _otherItem2;
	int _newItem2;

	void load(const Common::String &name, const int *data);

	int checkItem(int itemId);
};

class InventoryManager : public Manager {
	struct SavedFields {
		int _vWindowHeight;
		int _vWindowLinesTall;
		int _vWindowWidth;
		int _vWindowBytesWide;
		int _playFieldHeight;
		int _playFieldWidth;
		int _windowXAdd;
		int _windowYAdd;
		int _screenYOff;
		int _scrollX;
		int _scrollY;
		int _clipWidth;
		int _clipHeight;
		Common::Point _bufferStart;
		int _scrollCol;
		int _scrollRow;
	};
private:
	Common::Array<int> _items;
	Common::Array<Common::Rect> _invCoords;
	ASurface _savedBuffer1;
	ASurface _savedScreen;
	SavedFields _fields;
	bool _iconDisplayFlag;
	Common::Array<int> _tempLPtr;
	Common::StringArray _tempLOff;
	int _boxNum;

	void savedFields();

	void restoreFields();

	void initFields();

	void getList();

	void showAllItems();

	void putInvIcon(int itemIndex, int itemId);

	void chooseItem();

	void freeInvCells();

	int coordIndexOf();

	void saveScreens();

	void restoreScreens();

	void outlineIcon(int itemIndex);

	void combineItems();

	void zoomIcon(int zoomItem, int backItem, int zoomBox, bool shrink);
public:
	Common::Array<InventoryEntry> _inv;
	int _startInvItem;
	int _startInvBox;
	bool _invChangeFlag;
	bool _invRefreshFlag;
	bool _invModeFlag;
	int _startAboutItem;
	int _startTravelItem;
public:
	InventoryManager(AccessEngine *vm);

	int &operator[](int idx);

	int useItem();
	void setUseItem(int itemId);

	void refreshInventory();

	int newDisplayInv();
	int displayInv();

	/**
	* Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s);
};

} // End of namespace Access

#endif /* ACCESS_INVENTORY_H */
