/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_CURSOR_H
#define CHEWY_CURSOR_H

#include "chewy/globals.h"

namespace Chewy {

struct CursorSprite {
	uint16 width;
	uint16 height;
	byte *data;
};

class Cursor {
public:
	Cursor();
	~Cursor();

	void updateCursor();
	void showCursor();
	void hideCursor();
	bool isCursorVisible() const;
	void setAnimation(uint8 start, uint8 end, int16 delay);
	void setCustomCursor(byte *data, uint16 width, uint16 height);
	void setCustomRoomCursor(byte *roomSprite);
	void clearCustomCursor();
	void move(int16 x, int16 y);
	uint8 getAnimStart() const { return _animStart; }

	byte *getCursorSprite() const { return _currentCursor.data; }
	uint16 getCursorWidth() const { return _currentCursor.width; }
	uint16 getCursorHeight() const { return _currentCursor.height; }
	byte *getCursorSprite(uint num) const { return _curSprites[num].data; }
	uint16 getCursorWidth(uint num) const { return _curSprites[num].width; }
	uint16 getCursorHeight(uint num) const { return _curSprites[num].height; }

	void setInventoryCursor(int num) {
		_invCursor = num;
		if (num >= 0)
			setAnimation(num, num, (1 + _G(gameState).DelaySpeed) * 5);
	}
	int getInventoryCursor() const { return _invCursor; }
	bool usingInventoryCursor() const { return _invCursor >= 0; }

private:
	CursorSprite *_curSprites = nullptr;
	CursorSprite _customCursor;
	CursorSprite _currentCursor;
	uint32 _cursorCount = 0;
	uint32 _invCursorCount = 0;
	int _invCursor = 0;

	int16 _curAniCountdown = 0;
	int16 _aniCount = 0;

	uint8 _animStart = 0;
	uint8 _animEnd = 0;
	int16 _animDelay = 0;
};

} // namespace Chewy

#endif
