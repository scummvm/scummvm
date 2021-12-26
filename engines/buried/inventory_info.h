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

#ifndef BURIED_INVENTORY_INFO_H
#define BURIED_INVENTORY_INFO_H

#include "buried/navdata.h"
#include "buried/window.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Buried {

class AVIFrames;

class InventoryInfoWindow : public Window {
public:
	InventoryInfoWindow(BuriedEngine *vm, Window *parent, int currentItemID);
	~InventoryInfoWindow();

	bool changeCurrentItem(int newItemID);

	void onPaint();
	bool onEraseBackground();
	void onLButtonUp(const Common::Point &point, uint flags);
	void onTimer(uint timer);

private:
	Graphics::Font *_textFont;
	int _fontHeight;
	int _currentItemID;
	VideoWindow *_videoWindow;
	int32 _spinStart;
	int32 _spinLength;
	uint _timer;
};

class BurnedLetterViewWindow : public Window {
public:
	BurnedLetterViewWindow(BuriedEngine *vm, Window *parent, const LocationStaticData &curSceneStaticData);
	~BurnedLetterViewWindow();

	void onPaint();
	void onLButtonUp(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);
	bool onSetCursor(uint message);

private:
	LocationStaticData _curSceneStaticData;
	int _viewCount;
	int _curView;
	Common::Rect _top, _bottom, _left, _right, _putDown;
	int _translatedTextResourceID;
	int _curLineIndex;
	Graphics::Surface *_preBuffer;
	AVIFrames *_stillFrames;
	Common::Point _curMousePos;
	int _viewLineCount[3];
	bool _rebuildPage;
};

} // End of namespace Buried

#endif
