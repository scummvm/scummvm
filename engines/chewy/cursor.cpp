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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/events.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "chewy/cursor.h"
#include "chewy/resource.h"

namespace Chewy {

const byte _cursorFrames[] = {
	4, 1, 1, 1,		// walk
	4, 1, 1, 1,		// pick up / use
	1, 1, 1, 1, 1,
	4, 1, 1, 1,		// look
	4, 1, 1, 1,		// talk
	4, 1, 1, 1,		// open
	1,
	1, 1, 1, 1,		// left, right, up, down
	1,				// save
	1,
	5, 1, 1, 1, 1,
	1,
	1,				// use (inventory)
	1,				// look (inventory)
	1				// gun
};

Cursor::Cursor(ChewyEngine *vm) : _vm(vm) {
	_curCursor = 0;
	_curCursorFrame = 0;
	_cursorSprites = new SpriteResource("cursor.taf");
}

Cursor::~Cursor() {
	delete _cursorSprites;
}

void Cursor::setCursor(uint num, bool newCursor) {
	TAFChunk *cursor = _cursorSprites->getSprite(num);
	if (newCursor)
		_curCursor = num;

	CursorMan.replaceCursor(cursor->data, cursor->width, cursor->height, 0, 0, 0);

	delete[] cursor->data;
	delete cursor;
}

void Cursor::showCursor() {
	CursorMan.showMouse(true);
}

void Cursor::hideCursor() {
	CursorMan.showMouse(false);
}

void Cursor::animateCursor() {
	if (_cursorFrames[_curCursor] > 1) {
		_curCursorFrame++;

		if (_curCursorFrame >= _cursorFrames[_curCursor])
			_curCursorFrame = 0;

		setCursor(_curCursor + _curCursorFrame, false);
	}
}

void Cursor::nextCursor() {
	uint maxCursors = ARRAYSIZE(_cursorFrames);

	if (_cursorFrames[_curCursor] > 0)
		_curCursor += _cursorFrames[_curCursor];
	else
		_curCursor++;

	if (_curCursor >= maxCursors)
		_curCursor = 0;

	_curCursorFrame = 0;
	setCursor(_curCursor);
}

} // End of namespace Chewy
