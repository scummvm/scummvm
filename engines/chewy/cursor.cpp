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
	4, 1, 1, 1,     //   0-3: walk
	4, 1, 1, 1,     //   5-7: use (+ no use, 4)
	1,              //     8: go to
	4, 1, 1, 1,     //  9-12: nope
	4, 1, 1, 1,     // 13-16: look
	4, 1, 1, 1,     // 17-20: talk (+ no talk, 17)
	4, 1, 1, 1,     // 21-24: inventory
	1,              // 25: save
	1,              // 26: exit left
	1,              // 27: exit right
	1,              // 28: exit up
	1,              // 29: exit down
	1,              // 30: disk
	1,              // 31: Howard
	5, 1, 1, 1, 1,  // 32: animated arrow
	1,              // 37: Nichelle
	1,              // 38: use (inventory)
	1,              // 39: look (inventory)
	1               // 40: gun
};

Cursor::Cursor() {
	_curCursor = 0;
	_curCursorFrame = 0;
	_cursorSprites = new SpriteResource("cursor.taf");
}

Cursor::~Cursor() {
	delete _cursorSprites;
}

// TODO: This may need to be refactored, since in the original the user
// selects the cursor to use from a pop-up menu
CurrentCursor Cursor::getCurrentCursor() const {
	switch (_curCursor) {
	case 0:
	case 1:
	case 2:
	case 3:
		return kWalk;
	case 4:
	case 5:
	case 6:
	case 7:
		return kUse;
	case 13:
	case 14:
	case 15:
	case 16:
		return kLook;
	case 17:
	case 18:
	case 19:
	case 20:
		return kTalk;
	default:
		return kOther;
	}
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
