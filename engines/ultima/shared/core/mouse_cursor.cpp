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

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "ultima/shared/core/mouse_cursor.h"
#include "ultima/shared/core/file.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Shared {

void MouseCursor::MouseCursorData::load(Common::SeekableReadStream &s) {
	_hotspot.x = s.readSint16LE();
	_hotspot.y = s.readSint16LE();
	for (int idx = 0; idx < 16; ++idx)
		_pixels[idx] = s.readUint16LE();
	for (int idx = 0; idx < 16; ++idx)
		_mask[idx] = s.readUint16LE();
}

/*-------------------------------------------------------------------*/

MouseCursor::MouseCursor() : _cursorId(-1) {
/*
	loadCursors();
	_cursorId = -1;
	setCursor(0);
*/
}

void MouseCursor::loadCursors() {
	_cursors.clear();

	File f("TODO");
	while (f.pos() < f.size()) {
		_cursors.push_back(MouseCursorData());
		MouseCursorData &mc = _cursors.back();

		mc._hotspot.x = f.readSint16LE();
		mc._hotspot.y = f.readSint16LE();
		for (int idx = 0; idx < 16; ++idx)
			mc._mask[idx] = f.readUint16LE();
		for (int idx = 0; idx < 16; ++idx)
			mc._pixels[idx] = f.readUint16LE();
	}
}

void MouseCursor::setCursor(int cursorId) {
	// No need to do anything if we're already showing the desired cursor
	if (cursorId == _cursorId)
		return;
	_cursorId = cursorId;

	// Set up a temporary surface for rendering the cursor onto
	Graphics::ManagedSurface s(16, 16);
	s.fillRect(s.getBounds(), 0xff);

	const MouseCursorData &data = _cursors[cursorId];
	const uint16 *pixelsP = data._pixels, *maskP = data._mask;

	// Iterate trhough the lines to build up the cursor
	for (int y = 0; y < CURSOR_HEIGHT; ++y) {
		uint16 pixVal = *pixelsP++, maskVal = *maskP++;
		int bitMask = 0x8000;
		byte *destP = (byte *)s.getBasePtr(0, y);

		for (int x = 0; x < CURSOR_WIDTH; ++x, ++destP, bitMask >>= 1) {
			if (pixVal & bitMask)
				*destP = 15;
			else if (!(maskVal & bitMask))
				*destP = 0;
		}
	}

	// Pass the generated surface onto the ScummVM cursor manager
	CursorMan.replaceCursor(s.getPixels(), CURSOR_WIDTH, CURSOR_HEIGHT,
		data._hotspot.x, data._hotspot.y, 0xff);
}

void MouseCursor::show() {
	CursorMan.showMouse(true);
}

void MouseCursor::hide() {
	CursorMan.showMouse(false);
}

} // End of namespace Shared
} // End of namespace Ultima
