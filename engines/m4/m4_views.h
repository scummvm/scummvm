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

#ifndef M4_M4_VIEWS_H
#define M4_M4_VIEWS_H

#include "m4/gui.h"
#include "m4/viewmgr.h"
#include "common/rect.h"
#include "common/list.h"
#include "common/ptr.h"

namespace M4 {

class GUIInventoryItem {
public:
	const char *name;
	const char *verb;
	M4Surface *icon;
	int iconIndex;

	GUIInventoryItem(const char *_name, const char *_verb, M4Surface *_icon, int _iconIndex) {
		name = _name; verb = _verb; icon = _icon; iconIndex = _iconIndex;
	}
};

class GUIInventory : public GUIRect {
	typedef Common::List<Common::SharedPtr<GUIInventoryItem> > InventoryList;
	typedef InventoryList::iterator ItemsIterator;
private:
	InventoryList _inventoryItems;
	Common::Point _cellSize;
	Common::Point _cellCount;
	bool _visible;
	bool _scrollable;
	int _scrollPosition;
	int _highlightedIndex;
	int _selectedIndex;
	MadsM4Engine *_vm;
public:
	GUIInventory(View *owner, MadsM4Engine *vm, const Common::Rect &bounds,
				 int horizCells, int vertCells, int cellWidth, int cellHeight, int tag);

	void onRefresh();
	bool onEvent(M4EventType eventType, int32 param, int x, int y, GUIObject *&currentItem);

	void add(const char *name, const char *verb, M4Surface *icon, int iconIndex);
	bool remove(const char *name);
	int getInsideIndex(int x, int y);
	int getSelectedIndex() { return _selectedIndex; }
	const char *getSelectedObjectName();
	void clearSelected() {
		_selectedIndex = -1;
		setHighlight(-1);
	}
	const Common::Point &getCellPosition(int index);
	void setHighlight(int index);
	bool needLeftButton() { return _scrollPosition != 0; }
	bool needRightButton() {
		return (uint)(_inventoryItems.size() - _scrollPosition) > (uint)(_cellCount.x * _cellCount.y);
	}
	void setScrollPosition(int value);
	void scrollLeft() { setScrollPosition(_scrollPosition - 1); }
	void scrollRight() { setScrollPosition(_scrollPosition + 1); }
	void setVisible(bool value) { _visible = value; }
};

class M4InterfaceView : public GameInterfaceView {
	typedef Common::List<Common::SharedPtr<GUIButton> > ButtonList;
	typedef ButtonList::iterator ButtonsIterator;
public:
	SpriteAsset *_sprites;
	ButtonList _buttons;
	GUITextField _statusText;
	GUIInventory _inventory;
	int _highlightedIndex;
	bool _selected;
private:
	void setHighlightedButton(int index);
public:
	M4InterfaceView(MadsM4Engine *vm);
	~M4InterfaceView();

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);
	void setStatusText(const char *text) { _statusText.setText(text); }
	void cancelSentence() { setStatusText(NULL); }
	void inventoryAdd(const char *name, const char *verb, int iconIndex) {
		_inventory.add(name, verb, _sprites->getFrame(iconIndex), iconIndex);
	}
	bool inventoryRemove(const char *name) { return _inventory.remove(name); }
};

}

#endif
