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

Cursor::Cursor(CurBlk *curblkp) {
	_curblk = curblkp;

	_curAniCountdown = 0;
	_aniCount = 0;
}

Cursor::~Cursor() {
}

void Cursor::plot_cur() {
	if (CursorMan.isVisible()) {
		_cursorMoveFl = false;

		--_curAniCountdown;
		if (_curAniCountdown <= 0) {
			_curAniCountdown = _animDelay;
			++_aniCount;
			if (_aniCount > _animEnd)
				_aniCount = _animStart;
		}

		const uint16 w = READ_LE_INT16(_curblk->sprite[_aniCount]);
		const uint16 h = READ_LE_INT16(_curblk->sprite[_aniCount] + 2);
		CursorMan.replaceCursor(_curblk->sprite[_aniCount] + 4,	w, h, 0, 0, 0);
	}
}

void Cursor::show_cur() {
	CursorMan.showMouse(true);
	_cursorMoveFl = true;
	plot_cur();
}

void Cursor::hide_cur() {
	CursorMan.showMouse(false);
}

void Cursor::setAnimation(uint8 start, uint8 end, int16 delay) {
	_aniCount = _animStart = start;
	_animEnd = end;
	if (delay >= 0)
		_animDelay = delay;
	_curAniCountdown = 0;
}

void Cursor::move(int16 x, int16 y) {
	g_events->warpMouse(Common::Point(x, y));
	_cursorMoveFl = CursorMan.isVisible();
}

} // namespace Chewy
