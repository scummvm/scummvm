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
#include "common/textconsole.h"
#include "titanic/mouse_cursor.h"
#include "titanic/movie.h"
#include "titanic/screen_manager.h"
#include "titanic/titanic.h"
#include "titanic/video_surface.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

static const int CURSOR_DATA[NUM_CURSORS][4] = {
	{ 1, 136, 19, 18 },
	{ 2, 139, 1, 1 },
	{ 3, 140, 32, 1 },
	{ 4, 137, 13, 0 },
	{ 5, 145, 13, 0 },
	{ 6, 144, 13, 22 },
	{ 7, 137, 14, 0 },
	{ 8, 148, 22, 40 },
	{ 9, 136, 19, 18 },
	{ 10, 143, 11, 11 },
	{ 11, 146, 11, 11 },
	{ 12, 136, 19, 18 },
	{ 13, 136, 19, 25 },
	{ 14, 136, 13, 22 },
	{ 15, 138, 20, 28 }
};

CMouseCursor::CMouseCursor(CScreenManager *screenManager) : 
		_screenManager(screenManager), _cursorId(CURSOR_1) {
	loadCursorImages();
}

CMouseCursor::~CMouseCursor() {
	for (int idx = 0; idx < NUM_CURSORS; ++idx)
		delete _cursors[idx]._videoSurface;
}

void CMouseCursor::loadCursorImages() {
	const CString name("ycursors.avi");
	const CResourceKey key(name);
	g_vm->_filesManager.fn4(name);

	// Iterate through each cursor
	for (int idx = 0; idx < NUM_CURSORS; ++idx) {
		assert(CURSOR_DATA[idx][0] == (idx + 1));
		_cursors[idx]._centroid = Common::Point(CURSOR_DATA[idx][2],
			CURSOR_DATA[idx][3]);

		CVideoSurface *surface = _screenManager->createSurface(64, 64);
		_cursors[idx]._videoSurface = surface;

		OSMovie movie(key, surface);
		movie.setFrame(idx);
		_cursors[idx]._ptrUnknown = movie.proc21();
		surface->set40(_cursors[idx]._ptrUnknown);
	}
}

void CMouseCursor::show() {
	CursorMan.showMouse(true);
}

void CMouseCursor::hide() {
	CursorMan.showMouse(false);
}

void CMouseCursor::setCursor(CursorId cursorId) {
	warning("CMouseCursor::setCursor");
}

void CMouseCursor::update() {
	// No implementation needed
}

} // End of namespace Titanic
