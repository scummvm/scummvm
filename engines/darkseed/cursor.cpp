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

#include "cursor.h"
#include "darkseed.h"

void Darkseed::Cursor::setCursorType(enum CursorType newType) {
	_currentCursorType = newType;
}

void Darkseed::Cursor::updatePosition(int16 x, int16 y) {
	_position.x = x;
	_position.y = y;
//	debug("mouse at (%d,%d)", _x, _y);
}

void Darkseed::Cursor::draw() {
	const Sprite cursorSprite = g_engine->_baseSprites.getSpriteAt(_currentCursorType);
	int drawX = _position.x;
	if (drawX + cursorSprite.width > 640) {
		drawX = 640 - cursorSprite.width;
	}
	if (drawX < 0) {
		drawX = 0;
	}

	int drawY = _position.y;
	if (drawY + cursorSprite.height > 350) {
		drawY = 350 - cursorSprite.height;
	}
	if (drawY < 0) {
		drawY = 0;
	}

	if (_currentCursorType == HourGlass) {
		g_engine->_baseSprites.getSpriteAt(_currentCursorType).draw(310, 20);
	} else {
		g_engine->_baseSprites.getSpriteAt(_currentCursorType).draw(drawX, drawY);
	}
}

int Darkseed::Cursor::getWidth() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType).width;
}

int Darkseed::Cursor::getHeight() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType).height;
}

const Darkseed::Sprite &Darkseed::Cursor::getSprite() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType);
}

const Darkseed::Sprite &Darkseed::Cursor::getSpriteForType(Darkseed::CursorType cursorType) {
	return g_engine->_baseSprites.getSpriteAt(cursorType);
}
