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
#include "graphics/wincursor.h"
#include "image/bmp.h"
#include "bagel/mfc/gfx/cursor.h"
#include "bagel/mfc/wingdi.h"

namespace Bagel {
namespace MFC {
namespace Gfx {


constexpr int CURSOR_W = 12;
constexpr int CURSOR_H = 20;

static const byte ARROW_CURSOR[CURSOR_W * CURSOR_H] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0,
	1, 2, 2, 1, 1, 2, 2, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0,
	1, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0
};

static const byte HOURGLASS_CURSOR[12 * 12] = {
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0
};

static const byte CURSOR_PALETTE[9] = { 0x80, 0x80, 0x80, 0, 0, 0, 0xff, 0xff, 0xff };

/*--------------------------------------------*/

Cursors::Cursors(Libs::Resources &res) : _resources(res) {
	_cursors[IDC_ARROW] = new Cursor(ARROW_CURSOR);
	_cursors[IDC_WAIT] =  new Cursor(HOURGLASS_CURSOR);

	_arrowCursor = (HCURSOR)_cursors[IDC_ARROW];
	_waitCursor = (HCURSOR)_cursors[IDC_WAIT];
}

Cursors::~Cursors() {
	for (CursorHash::iterator it = _cursors.begin();
	        it != _cursors.end(); ++it)
		delete it->_value;
}

HCURSOR Cursors::loadCursor(LPCSTR cursorId) {
	if (_cursors.contains(cursorId))
		return (HCURSOR)_cursors[cursorId];

	Cursor *c = new Cursor(*_resources.getCoreResources(), cursorId);
	_cursors[cursorId] = c;
	return (HCURSOR)c;
}

/*--------------------------------------------*/

Cursor::Cursor(const byte *pixels) : _isBuiltIn(true) {
	_surface.create(CURSOR_W, CURSOR_H,
	                Graphics::PixelFormat::createFormatCLUT8());
}

Cursor::Cursor(Common::WinResources &res, LPCSTR cursorId) :
	_isBuiltIn(false) {
	Image::BitmapDecoder decoder;

	intptr id = (intptr)cursorId;
	bool success = false;

	Common::SeekableReadStream *bmp = res.getResource(
	                                      Common::kWinBitmap,
	                                      (id < 65536) ? Common::WinResourceID(id) :
	                                      Common::WinResourceID(cursorId));
	if (bmp) {
		success = decoder.loadStream(*bmp);

		if (success) {
			const Graphics::Surface &s = *decoder.getSurface();
			_surface.copyFrom(s);
		}
	}

	if (_surface.empty()) {
		Graphics::WinCursorGroup *group =
		    Graphics::WinCursorGroup::createCursorGroup(&res, id);
		if (group) {
			const auto &cursor = group->cursors[0].cursor;
			CursorMan.replaceCursor(cursor);

			delete group;
			success = true;
		}
	}

	if (!success)
		error("Could not load cursor resource");
}

void Cursor::showCursor() {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	CursorMan.disableCursorPalette(!_isBuiltIn);
	if (_isBuiltIn)
		CursorMan.replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);

	CursorMan.replaceCursor(_surface.getPixels(),
	                        _surface.w, _surface.h, 0, 0, 0, true, &format);
	CursorMan.showMouse(true);
}

void Cursor::hide() {
	CursorMan.showMouse(false);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
