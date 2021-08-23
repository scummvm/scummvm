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

#include "common/rect.h"
#include "graphics/cursorman.h"

#include "hypno/hypno.h"

namespace Hypno {

static const byte MOUSECURSOR_SCI[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1,
	1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 1, 2, 2, 1, 0, 0, 0,
	1, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0};

static const byte cursorPalette[] = {
	0x00, 0x00, 0x00, // Black / Transparent
	0x01, 0x01, 0x01, // Gray
	0xff, 0xff, 0xff, // White
	0xff, 0x00, 0x00  // Red
};

struct CursorTable {
	const char *name;
	const char *aname;
	const void *buf;
	int w;
	int h;
	int hotspotX;
	int hotspotY;
};

static const CursorTable cursorTable[] = {
	{"default", "", MOUSECURSOR_SCI, 11, 16, 0, 0},
	{nullptr, nullptr, nullptr, 0, 0, 0, 0}};

void HypnoEngine::disableCursor() {
	CursorMan.showMouse(false);
}

void HypnoEngine::defaultCursor() {
	if (!_defaultCursor.empty())
		changeCursor(_defaultCursor, 0);
	else {
		CursorMan.showMouse(true);
		const CursorTable *entry = cursorTable;
		CursorMan.replaceCursor(entry->buf, entry->w, entry->h, entry->hotspotX, entry->hotspotY, 0);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
		CursorMan.showMouse(true);
	}
}

void HypnoEngine::changeCursor(const Common::String &cursor, uint32 n) {
	Graphics::Surface *entry = decodeFrame(cursor, n, false);
	CursorMan.replaceCursor(entry->getPixels(), entry->w, entry->h, 0, 0, 0);
	CursorMan.showMouse(true);
}

} // End of namespace Hypno
