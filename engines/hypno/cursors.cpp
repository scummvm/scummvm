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

#include "common/rect.h"
#include "graphics/cursorman.h"

#include "hypno/hypno.h"

namespace Hypno {

static const byte MOUSECURSOR_SCI[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 3, 3, 3, 1, 0, 0, 0, 0, 0, 0,
	1, 3, 3, 3, 3, 1, 0, 0, 0, 0, 0,
	1, 3, 3, 3, 3, 3, 1, 0, 0, 0, 0,
	1, 3, 3, 3, 3, 3, 3, 1, 0, 0, 0,
	1, 3, 3, 3, 3, 3, 3, 3, 1, 0, 0,
	1, 3, 3, 3, 3, 3, 3, 3, 3, 1, 0,
	1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1,
	1, 3, 3, 3, 3, 3, 1, 0, 0, 0, 0,
	1, 3, 1, 0, 1, 3, 3, 1, 0, 0, 0,
	1, 1, 0, 0, 1, 3, 3, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 3, 3, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 3, 3, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 0};

static const byte circleCursor[] = {
	0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0,
	0, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 0, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
	2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	0, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 0, 0,
	0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0};

static const byte targetCursor[] = {
	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 1, 0, 0, 2, 2, 2, 2, 2, 0, 0, 1, 0, 0,
	0, 1, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 0, 1, 0,
	1, 0, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 2, 0, 1,
	1, 0, 2, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 0, 1,
	1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 1,
	1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 1,
	1, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 1,
	1, 0, 2, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 0, 1,
	1, 0, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 2, 0, 1,
	0, 1, 0, 2, 2, 0, 0, 0, 0, 0, 2, 2, 0, 1, 0,
	0, 0, 1, 0, 0, 2, 2, 2, 2, 2, 0, 0, 1, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0};

static const byte crosshairCursor[] = {
	0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 2, 2, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0,
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
	0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 0, 2, 2, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0};

static const byte cursorPalette[] = {
	0x00, 0x00, 0x00, // Transparent
	0x00, 0x00, 0xff, // Blue
	0xff, 0x00, 0x00, // Red
	0xff, 0xff, 0xff  // White
};

static const byte sciCursorPalette[] = {
	0x00, 0x00, 0x00, // Transparent
	0xff, 0xff, 0xff, // Black
	0xff, 0x00, 0x00, // Red
	0xff, 0xff, 0xff  // White
};

struct CursorTable {
	const char *name;
	const void *buf;
	int w;
	int h;
	int hotspotX;
	int hotspotY;
};

static const CursorTable cursorTable[] = {
	{"default", MOUSECURSOR_SCI, 11, 16, 0, 0},
	{"arcade", circleCursor, 13, 11, 7, 5},
	{"target", targetCursor, 15, 13, 8, 6},
	{"crosshair", crosshairCursor, 15, 13, 8, 6},
	{nullptr, nullptr, 0, 0, 0, 0}};

void HypnoEngine::disableCursor() {
	CursorMan.showMouse(false);
}

void HypnoEngine::defaultCursor() {
	if (!_defaultCursor.empty()) {
		if (_defaultCursorIdx == uint32(-1))
			changeCursor(_defaultCursor);
		else
			changeCursor(_defaultCursor, _defaultCursorIdx);
	} else
		changeCursor("default");
}

void HypnoEngine::changeCursor(const Common::String &cursor) {
	const CursorTable *entry = cursorTable;
	while (entry->name) {
		if (cursor == entry->name)
			break;
		entry++;
	}
	assert(entry->name);

	if (cursor == "default")
		CursorMan.replaceCursorPalette(sciCursorPalette, 0, 3);
	else
		CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
	CursorMan.replaceCursor(entry->buf, entry->w, entry->h, entry->hotspotX, entry->hotspotY, 0);
	CursorMan.showMouse(true);
}

Graphics::Surface *CursorCache::getCursor(const Common::String &cursor, uint32 n, byte **palette) {
	if (cursor == _filename && n == _frame) {
		*palette = _palette;
		return _surface;
	}

	free(_palette);
	_palette = nullptr;

	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	_filename = cursor;
	_frame = n;
	_surface = _vm->decodeFrame(cursor, n, &_palette);
	*palette = _palette;
	return _surface;
}

void HypnoEngine::changeCursor(const Common::String &cursor, uint32 n, bool centerCursor) {
	byte *palette;
	Graphics::Surface *entry = _cursorCache->getCursor(cursor, n, &palette);
	uint32 hotspotX = centerCursor ? entry->w / 2 : 0;
	uint32 hotspotY = centerCursor ? entry->h / 2 : 0;
	CursorMan.replaceCursor(*entry, hotspotX, hotspotY, 0, false);
	CursorMan.replaceCursorPalette(palette, 0, 256);
	CursorMan.showMouse(true);
}

void HypnoEngine::changeCursor(const Graphics::Surface &entry, byte *palette, bool centerCursor) {
	uint32 hotspotX = centerCursor ? entry.w / 2 : 0;
	uint32 hotspotY = centerCursor ? entry.h / 2 : 0;
	CursorMan.replaceCursor(entry, hotspotX, hotspotY, 0, false);
	CursorMan.replaceCursorPalette(palette, 0, 256);
	CursorMan.showMouse(true);
}


} // End of namespace Hypno

