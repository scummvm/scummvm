/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_INVENTORY_WINDOW_H
#define BURIED_INVENTORY_WINDOW_H

#include "buried/invdata.h"
#include "buried/sprtdata.h"
#include "buried/window.h"

#include "common/array.h"

namespace Graphics {
class Font;
}

namespace Buried {

class AVIFrames;
class InventoryInfoWindow;
class BurnedLetterViewWindow;

class InventoryWindow : public Window {
public:
	InventoryWindow(BuriedEngine *vm, Window *parent);
	~InventoryWindow();

	bool rebuildPreBuffer();

	bool addItem(int itemID);
	bool removeItem(int itemID);

	bool startDraggingNewItem(int itemID, const Common::Point &pointStart);
	bool isItemInInventory(int itemID);
	InventoryElement getItemStaticData(int itemID);
	int getItemCount() { return _itemArray.size(); }
	int getItemID(int itemIndex) { return _itemArray[itemIndex]; }
	void setItemArray(const Common::Array<int> &array);
	Common::Array<int> &getItemArray() { return _itemArray; }

	bool destroyInfoWindow();
	InventoryInfoWindow *getInfoWindow() { return _infoWindow; }

	bool displayBurnedLetterWindow();
	bool destroyBurnedLetterWindow();
	BurnedLetterViewWindow *getBurnedLetterWindow() { return _letterViewWindow; }

	void onPaint();
	void onLButtonDown(const Common::Point &point, uint flags);
	void onLButtonUp(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);
	bool onSetCursor(uint message);
	void onTimer(uint timer);

private:
	void setCurItem(uint16 itemId);
	uint16 getCurItem() const;

	Graphics::Font *_textFont;
	int _fontHeight;
	Graphics::Surface *_background;
	Common::Array<int> _itemArray;

	bool _magSelected;
	bool _upSelected;
	bool _downSelected;
	int _textSelected;

	bool _draggingObject;
	bool _itemComesFromInventory;
	int _draggingItemID;
	Sprite _draggingItemSpriteData;
	int _draggingIconIndex;
	bool _draggingItemInInventory;

	Common::Point _curMousePos;
	int _curCursor;

	InventoryInfoWindow *_infoWindow;
	BurnedLetterViewWindow *_letterViewWindow;

	AVIFrames *_dragFrames;

	uint _scrollTimer;

	// TODO: Item data
};

} // End of namespace Buried

#endif
