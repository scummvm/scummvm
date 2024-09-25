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

#include "graphics/cursorman.h"
#include "darkseed/cursor.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

void Cursor::setCursorType(enum CursorType newType) {
	bool loadCursor = !_hasLoadedCursor || _currentCursorType != newType;
	_currentCursorType = newType;

	if (loadCursor) {
		const Sprite cursorSprite = g_engine->_baseSprites.getSpriteAt(_currentCursorType);
		Graphics::Surface surf;
		surf.create(cursorSprite._width, cursorSprite._height, Graphics::PixelFormat::createFormatCLUT8());
		surf.copyRectToSurface(cursorSprite._pixels.data(), cursorSprite._pitch, 0, 0, cursorSprite._width, cursorSprite._height);
		CursorMan.replaceCursor(surf, 0, 0, 0xf);
	}
}

void Cursor::updatePosition(int16 x, int16 y) {
	_position.x = x;
	_position.y = y;
	//	debug("mouse at (%d,%d)", _x, _y);
}

int Cursor::getWidth() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType)._width;
}

int Cursor::getHeight() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType)._height;
}

const Sprite &Darkseed::Cursor::getSprite() {
	return g_engine->_baseSprites.getSpriteAt(_currentCursorType);
}

const Sprite &Darkseed::Cursor::getSpriteForType(Darkseed::CursorType cursorType) {
	return g_engine->_baseSprites.getSpriteAt(cursorType);
}

void Cursor::showCursor(bool showCursor) {
	CursorMan.showMouse(showCursor);
}

} // End of namespace Darkseed
