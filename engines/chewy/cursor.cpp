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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"

namespace Chewy {

Cursor::Cursor() {
	const auto res = new SpriteResource(CURSOR_TAF);
	const auto invRes = new SpriteResource(INVENTORY_TAF);
	_cursorCount = res->getChunkCount();
	_invCursorCount = invRes->getChunkCount();
	_curSprites = new CursorSprite[_cursorCount + _invCursorCount];

	for (uint32 i = 0; i < _cursorCount + _invCursorCount; i++) {
		const TAFChunk *sprite = (i < _cursorCount) ? res->getSprite(i) : invRes->getSprite(i - _cursorCount);
		_curSprites[i].width = sprite->width;
		_curSprites[i].height = sprite->height;
		_curSprites[i].data = new byte[sprite->width * sprite->height];
		memcpy(_curSprites[i].data, sprite->data, sprite->width * sprite->height);
		delete sprite;
	}

	delete invRes;
	delete res;

	_currentCursor.data = _customCursor.data = nullptr;
	_currentCursor.width = _customCursor.width = 0;
	_currentCursor.height = _customCursor.height = 0;

	clearCustomCursor();
}

Cursor::~Cursor() {
	for (uint32 i = 0; i < _cursorCount + _invCursorCount; i++) {
		delete[] _curSprites[i].data;
		_curSprites[i].data = nullptr;
	}

	delete[] _curSprites;
}

void Cursor::updateCursor() {
	if (!CursorMan.isVisible())
		return;

	--_curAniCountdown;
	if (_curAniCountdown <= 0) {
		_curAniCountdown = _animDelay;
		++_aniCount;
		if (_aniCount > _animEnd)
			_aniCount = _animStart;
	}

	if (_customCursor.data != nullptr) {
		CursorMan.replaceCursor(_customCursor.data, _customCursor.width, _customCursor.height, 0, 0, 0);
		_currentCursor.data = _customCursor.data;
		_currentCursor.width = _customCursor.width;
		_currentCursor.height = _customCursor.height;
	} else {
		const uint32 cursorOffset = _invCursor >= 0 ? _cursorCount : 0;
		const CursorSprite s = _curSprites[_aniCount + cursorOffset];
		CursorMan.replaceCursor(s.data, s.width, s.height, 0, 0, 0);
		_currentCursor.data = s.data;
		_currentCursor.width = s.width;
		_currentCursor.height = s.height;
	}
}

void Cursor::showCursor() {
	CursorMan.showMouse(true);
	updateCursor();
}

void Cursor::hideCursor() {
	CursorMan.showMouse(false);
}

bool Cursor::isCursorVisible() const {
	return CursorMan.isVisible();
}

void Cursor::setAnimation(uint8 start, uint8 end, int16 delay) {
	_aniCount = _animStart = start;
	_animEnd = end;
	if (delay >= 0)
		_animDelay = delay;
	_curAniCountdown = 0;
}

void Cursor::setCustomRoomCursor(byte *roomSprite) {
	const uint16 width = READ_LE_INT16(roomSprite);
	const uint16 height = READ_LE_INT16(roomSprite + 2);
	setCustomCursor(roomSprite + 4, width, height);
}

void Cursor::setCustomCursor(byte *data, uint16 width, uint16 height) {
	_currentCursor.data = _customCursor.data = data;
	_currentCursor.width = _customCursor.width = width;
	_currentCursor.height = _customCursor.height = height;

	CursorMan.replaceCursor(_customCursor.data, _customCursor.width, _customCursor.height, 0, 0, 0);
}

void Cursor::clearCustomCursor() {
	if (_customCursor.data) {
		_customCursor.data = nullptr;
		_customCursor.width = 0;
		_customCursor.height = 0;

		updateCursor();
	}
}

void Cursor::move(int16 x, int16 y) {
	g_events->warpMouse(Common::Point(x, y));
}

} // namespace Chewy
