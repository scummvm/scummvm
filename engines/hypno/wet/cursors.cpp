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
	0, 0, 0, 0, 250, 250, 250, 250, 250, 0, 0, 0, 0,
	0, 0, 250, 250, 0, 0, 0, 0, 0, 250, 250, 0, 0,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250,
	250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250,
	250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	0, 0, 250, 250, 0, 0, 0, 0, 0, 250, 250, 0, 0,
	0, 0, 0, 0, 250, 250, 250, 250, 250, 0, 0, 0, 0};

static const byte targetCursor[] = {
	0, 0, 0, 252, 252, 252, 252, 252, 252, 252, 252, 252, 0, 0, 0,
	0, 0, 252, 0, 0, 250, 250, 250, 250, 250, 0, 0, 252, 0, 0,
	0, 252, 0, 250, 250, 0, 0, 0, 0, 0, 250, 250, 0, 252, 0,
	252, 0, 250, 0, 0, 252, 252, 252, 252, 252, 0, 0, 250, 0, 252,
	252, 0, 250, 0, 252, 0, 0, 0, 0, 0, 252, 0, 250, 0, 252,
	252, 250, 0, 252, 0, 0, 0, 0, 0, 0, 0, 252, 0, 250, 252,
	252, 250, 0, 252, 0, 0, 0, 0, 0, 0, 0, 252, 0, 250, 252,
	252, 250, 0, 252, 0, 0, 0, 0, 0, 0, 0, 252, 0, 250, 252,
	252, 0, 250, 0, 252, 0, 0, 0, 0, 0, 252, 0, 250, 0, 252,
	252, 0, 250, 0, 0, 252, 252, 252, 252, 252, 0, 0, 250, 0, 252,
	0, 252, 0, 250, 250, 0, 0, 0, 0, 0, 250, 250, 0, 252, 0,
	0, 0, 252, 0, 0, 250, 250, 250, 250, 250, 0, 0, 252, 0, 0,
	0, 0, 0, 252, 252, 252, 252, 252, 252, 252, 252, 252, 0, 0, 0};

static const byte crosshairCursor[] = {
	0, 0, 0, 0, 0, 0, 0, 250, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 250, 250, 250, 250, 250, 0, 0, 0, 0, 0,
	0, 0, 0, 250, 250, 0, 0, 250, 0, 0, 250, 250, 0, 0, 0,
	0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 0,
	0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 0,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	250, 250, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 250, 250,
	0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0,
	0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 0,
	0, 0, 250, 0, 0, 0, 0, 0, 0, 0, 0, 0, 250, 0, 0,
	0, 0, 0, 250, 250, 0, 0, 250, 0, 0, 250, 250, 0, 0, 0,
	0, 0, 0, 0, 0, 250, 250, 250, 250, 250, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 250, 0, 0, 0, 0, 0, 0, 0};

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

void WetEngine::changeCursor(const Common::String &cursor) {
	const CursorTable *entry = cursorTable;
	while (entry->name) {
		if (cursor == entry->name)
			break;
		entry++;
	}
	assert(entry->name);

	CursorMan.replaceCursor(entry->buf, entry->w, entry->h, entry->hotspotX, entry->hotspotY, 0);
	CursorMan.showMouse(true);
}

} // End of namespace Hypno

