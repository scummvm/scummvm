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
#include "bagel/boflib/gfx/cursor.h"

namespace Bagel {

static const int CURSOR_W = 12;
static const int CURSOR_H = 20;
static const byte ARROW_CURSOR[CURSOR_W * CURSOR_H] = {
	1,1,0,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,1,1,1,1,1,
	1,2,2,2,1,2,2,1,0,0,0,0,
	1,2,2,1,1,2,2,1,0,0,0,0,
	1,2,1,0,0,1,2,2,1,0,0,0,
	1,1,0,0,0,1,2,2,1,0,0,0,
	1,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,1,2,2,1,0,
	0,0,0,0,0,0,0,0,1,1,0,0,
};
static const byte CURSOR_PALETTE[] = { 0x80, 0x80, 0x80, 0, 0, 0, 0xff, 0xff, 0xff };
int CBofCursor::_showCount;

void CBofCursor::initialize() {
	_showCount = 0;
	Show();
}

CBofCursor::~CBofCursor() {
	Assert(IsValidObject(this));

	UnLoad();
}

ERROR_CODE CBofCursor::Load() {
	Assert(IsValidObject(this));

	// kill any previous cursor
	UnLoad();

	return m_errCode;
}

ERROR_CODE CBofCursor::UnLoad() {
	Assert(IsValidObject(this));

	return m_errCode;
}

ERROR_CODE CBofCursor::Set() {
	Assert(IsValidObject(this));

	return m_errCode;
}

VOID CBofCursor::Hide() {
	if (_showCount == 0) {
		CursorMan.showMouse(false);
	}
	_showCount--;
}

VOID CBofCursor::Show() {
	if (_showCount == 0) {
		Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		CursorMan.replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
		CursorMan.replaceCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
		CursorMan.showMouse(true);
	}
	_showCount++;
}

} // namespace Bagel
