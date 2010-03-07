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
 * $URL$
 * $Id$
 *
 */

#ifndef M4_MADS_VIEWS_H
#define M4_MADS_VIEWS_H

#include "m4/gui.h"
#include "m4/viewmgr.h"
#include "common/rect.h"
#include "common/list.h"
#include "common/ptr.h"

namespace M4 {

#define CHEAT_SEQUENCE_MAX 8

class IntegerList : public Common::Array<int> {
public:
	int indexOf(int v) {
		for (uint i = 0; i < size(); ++i)
			if (operator [](i) == v)
				return i;
		return -1;
	}
};

enum InterfaceFontMode {ITEM_NORMAL, ITEM_HIGHLIGHTED, ITEM_SELECTED};

enum InterfaceObjects {ACTIONS_START = 0, SCROLL_UP = 10, SCROLL_SCROLLER = 11, SCROLL_DOWN = 12,
		INVLIST_START = 13, VOCAB_START = 18};

class MadsInterfaceView : public GameInterfaceView {
private:
	IntegerList _inventoryList;
	RectList _screenObjects;
	int _highlightedElement;
	int _topIndex;
	uint32 _nextScrollerTicks;
	int _cheatKeyCtr;

	// Object display fields
	int _selectedObject;
	SpriteAsset *_objectSprites;
	RGBList *_objectPalData;
	int _objectFrameNumber;

	void setFontMode(InterfaceFontMode newMode);
	bool handleCheatKey(int32 keycode);
	bool handleKeypress(int32 keycode);
public:
	MadsInterfaceView(MadsM4Engine *vm);
	~MadsInterfaceView();

	virtual void initialise();
	virtual void setSelectedObject(int objectNumber);
	virtual void addObjectToInventory(int objectNumber);
	int getSelectedObject() { return _selectedObject; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

}

#endif
