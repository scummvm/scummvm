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

static const byte ARROW_CURSOR[8 * 16] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff,
	0x00, 0x01, 0x01, 0x01, 0x00, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x01, 0x00, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x00, 0x01, 0x00, 0xff, 0xff,
	0xff, 0xff, 0xff, 0x00, 0x01, 0x00, 0xff, 0xff
};
static byte CURSOR_PALETTE[6] = { 0, 0, 0, 0xff, 0xff, 0xff };

void CBofCursor::initStatics() {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.pushCursorPalette(CURSOR_PALETTE, 0, 2);
	CursorMan.pushCursor(ARROW_CURSOR, 8, 16, 0, 0, 0xff, &format);
}

CBofCursor::CBofCursor(SHORT nResID) {
	Load(nResID);
}

CBofCursor::~CBofCursor() {
	Assert(IsValidObject(this));

	UnLoad();
}

ERROR_CODE CBofCursor::Load(SHORT nResId) {
	Assert(IsValidObject(this));

	// kill any previous cursor
	UnLoad();

	warning("Load cursor %d", nResId);

	if (nResId == 0) {
		// Arrow cursor
		Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
		CursorMan.replaceCursor(ARROW_CURSOR, 8, 16, 0, 0, 0xff, &format);

	} else {
		error("Unhandled resource %d", nResId);
	}

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
	CursorMan.showMouse(false);
}

VOID CBofCursor::Show() {
	CursorMan.showMouse(true);
}

} // namespace Bagel
