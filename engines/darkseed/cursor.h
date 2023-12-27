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

#include "nsp.h"
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
	int _x = 0;
	int _y = 0;
	enum CursorType _currentCursorType = Pointer;

public:

	void setCursorType(enum CursorType newType);
	CursorType getCursorType() { return _currentCursorType; }

	int getX() { return _x; }
	int getY() { return _y; }

	int getWidth();
	int getHeight();
	const Sprite &getSprite();

	void updatePosition(int x, int y);
	void draw();
};

} // End of namespace Darkseed

#endif // DARKSEED_CURSOR_H
