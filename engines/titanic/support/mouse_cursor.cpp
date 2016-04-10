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

#include "common/memstream.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "titanic/support/mouse_cursor.h"
#include "titanic/support/movie.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"
#include "titanic/support/video_surface.h"
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
		_screenManager(screenManager), _cursorId(CURSOR_HOURGLASS), _setCursorCount(0) {
	loadCursorImages();
	setCursor(CURSOR_ARROW);
}

CMouseCursor::~CMouseCursor() {
	for (int idx = 0; idx < NUM_CURSORS; ++idx)
		delete _cursors[idx]._videoSurface;
}

void CMouseCursor::loadCursorImages() {
	const CString name("ycursors.avi");
	g_vm->_filesManager.fn4(name);

	// WORKAROUND: We need to manipulate ycursors.avi file so it can be read
	// by the ScummVM AVIDecoder, by removing the redundant second video track
	Common::File f;
	if (!f.open(name))
		error("Could not open cursors file");

	// Read in the entire file
	byte *movieData = (byte *)malloc(f.size());
	f.read(movieData, f.size());

	if (READ_BE_UINT32(movieData + 254) == MKTAG('s', 't', 'r', 'h')) {
		// Change the second video chunk to junk data so it gets ignored
		WRITE_BE_UINT32(movieData + 254, MKTAG('J', 'U', 'N', 'K'));
		WRITE_LE_UINT32(movieData + 258, 1128);
	}

	// Iterate through each cursor
	for (int idx = 0; idx < NUM_CURSORS; ++idx) {
		assert(CURSOR_DATA[idx][0] == (idx + 1));
		_cursors[idx]._centroid = Common::Point(CURSOR_DATA[idx][2],
			CURSOR_DATA[idx][3]);

		// Create the surface
		CVideoSurface *surface = _screenManager->createSurface(64, 64);
		_cursors[idx]._videoSurface = surface;

		Common::SeekableReadStream *stream = new Common::MemoryReadStream(
			movieData, f.size(), DisposeAfterUse::NO);
		OSMovie movie(stream, surface);
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
	++_setCursorCount;

	if (cursorId != _cursorId) {
		CursorEntry &ce = _cursors[cursorId - 1];
		CVideoSurface &surface = *ce._videoSurface;
		surface.lock();

		CursorMan.replaceCursor(surface.getPixels(), surface.getWidth(), surface.getHeight(),
			ce._centroid.x, ce._centroid.y, 0, false, &g_vm->_screen->format);
		surface.unlock();

		_cursorId = cursorId;
	}
}

void CMouseCursor::update() {
	// No implementation needed
}

} // End of namespace Titanic
