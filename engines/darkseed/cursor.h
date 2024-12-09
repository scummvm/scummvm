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

#ifndef DARKSEED_CURSOR_H
#define DARKSEED_CURSOR_H

#include "common/rect.h"
#include "darkseed/nsp.h"

namespace Darkseed {

enum CursorType {
	Pointer = 0,
	Hand = 2,
	Look = 3,
	HourGlass = 91,
	ConnectorEntrance = 92,
	HandPointing = 93,
	ExclamationMark = 94,
};

class Cursor {
private:
	Common::Point _position;
	enum CursorType _currentCursorType = Pointer;
	bool _hasLoadedCursor = false;

public:

	void showCursor(bool showCursor);
	void setCursorType(enum CursorType newType);
	CursorType getCursorType() const {
		return _currentCursorType;
	}

	Common::Point &getPosition() {
		return _position;
	}
	void setPosition(Common::Point &position) {
		_position = position;
	}

	int16 getX() const {
		return _position.x;
	}
	int16 getY() const {
		return _position.y;
	}

	Common::Rect getRect() const {
		return Common::Rect(_position, getWidth(), getHeight());
	}

	int getWidth() const;
	int getHeight() const;
	const Sprite &getSprite() const;
	const Sprite &getSpriteForType(CursorType cursorType) const;
	void updatePosition(int16 x, int16 y);
};

} // End of namespace Darkseed

#endif // DARKSEED_CURSOR_H
