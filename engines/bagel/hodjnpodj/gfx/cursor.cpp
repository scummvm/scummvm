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

#include "common/system.h"
#include "common/file.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/cursor.h"
#include "bagel/hodjnpodj/gfx/gfx_surface.h"
#include "bagel/hodjnpodj/events.h"

namespace Bagel {
namespace HodjNPodj {

void Cursor::loadCursors() {
	if (!_resources.loadFromEXE("meta/hnpmeta.dll"))
		error("Could not load meta/hodjpodj.exe");
}

void Cursor::setCursor(int cursorId) {
	if (cursorId != _cursorId) {
		_cursorId = cursorId;
		CursorMan.showMouse(_cursorId != IDC_NONE);

		if (cursorId == 1) {
			setArrowCursor();

		} else if (cursorId != IDC_NONE) {
			setCursorResource();
		}
	}
}

void Cursor::setArrowCursor() {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.disableCursorPalette(false);
	CursorMan.replaceCursorPalette(CURSOR_PALETTE, 0, ARRAYSIZE(CURSOR_PALETTE) / 3);
	CursorMan.replaceCursor(ARROW_CURSOR, CURSOR_W, CURSOR_H, 0, 0, 0, true, &format);
}

void Cursor::setCursorResource() {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();

	Image::BitmapDecoder decoder;
	Common::SeekableReadStream *bmp = _resources.getResource(
		Common::kWinBitmap, _cursorId);

	CursorMan.disableCursorPalette(true);

	bool success = false;
	if (bmp) {
		success = decoder.loadStream(*bmp);

		if (success) {
			const Graphics::Surface &s = *decoder.getSurface();
			CursorMan.replaceCursor(s.getPixels(), s.w, s.h, 0, 0, 0, true, &format);
		}
	} else {
		Common::WinResources *res = g_events->getResources();
		if (res) {
			Graphics::WinCursorGroup *group =
				Graphics::WinCursorGroup::createCursorGroup(res, _cursorId);
			if (group) {
				const auto &cursor = group->cursors[0].cursor;
				CursorMan.replaceCursor(cursor);

				delete group;
				success = true;
			}
		}
	}

	if (!success)
		error("Could not load cursor resource - %d", _cursorId);

}

} // namespace HodjNPodj
} // namespace Bagel
