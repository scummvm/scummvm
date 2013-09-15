/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_INVENTORY_WINDOW_H
#define BURIED_INVENTORY_WINDOW_H

#include "buried/invdata.h"
#include "buried/sprtdata.h"
#include "buried/window.h"

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
	int getCurrentItemID() { return _itemArray[_curItem]; }
	bool isItemInInventory(int itemID);
	InventoryElement getItemStaticData(int itemID);
	int getItemCount() { return _itemArray.size(); }
	int getItemID(int itemIndex) { return _itemArray[itemIndex]; }
	// getItemArray
	// setItemArray

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
	Graphics::Font *_textFont;
	Graphics::Surface *_background;
	Common::Array<int> _itemArray;
	int _curItem;

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
