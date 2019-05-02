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

#include "titanic/support/mouse_cursor.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/transparency_surface.h"
#include "titanic/support/video_surface.h"
#include "titanic/events.h"
#include "titanic/input_handler.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/titanic.h"
#include "graphics/cursorman.h"
#include "graphics/screen.h"

namespace Titanic {

#define CURSOR_SIZE 64

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

CMouseCursor::CursorEntry::~CursorEntry() {
	delete _surface;
}

CMouseCursor::CMouseCursor(CScreenManager *screenManager) :
		_screenManager(screenManager), _cursorId(CURSOR_HOURGLASS), _hideCounter(0),
		_busyCount(0), _cursorSuppressed(false), _setCursorCount(0), _inputEnabled(true), _fieldE8(0) {
	loadCursorImages();
	setCursor(CURSOR_ARROW);
	CursorMan.showMouse(true);
}

CMouseCursor::~CMouseCursor() {
}

void CMouseCursor::loadCursorImages() {
	const CResourceKey key("ycursors.avi");

	// Iterate through getting each cursor
	for (int idx = 0; idx < NUM_CURSORS; ++idx) {
		assert(CURSOR_DATA[idx][0] == (idx + 1));
		_cursors[idx]._centroid = Common::Point(CURSOR_DATA[idx][2],
			CURSOR_DATA[idx][3]);

		// Create the surface
		CVideoSurface *surface = _screenManager->createSurface(CURSOR_SIZE, CURSOR_SIZE);

		// Open the cursors video and move to the given frame
		OSMovie *movie = new OSMovie(key, surface);
		movie->setFrame(idx);
		Graphics::ManagedSurface *transSurface = movie->duplicateTransparency();

		// Create a managed surface to hold the RGBA version of the cursor
		Graphics::PixelFormat rgbaFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
		_cursors[idx]._surface = new Graphics::ManagedSurface(CURSOR_SIZE, CURSOR_SIZE, rgbaFormat);

		// Copy the cursor from the movie's video surface
		surface->lock();
		_cursors[idx]._surface->blitFrom(*surface->getRawSurface());
		surface->unlock();

		// We need to separately merge in the transparency surface
		for (int y = 0; y < CURSOR_SIZE; ++y) {
			const byte *srcP = (const byte *)transSurface->getBasePtr(0, y);
			uint32 *destP = (uint32 *)_cursors[idx]._surface->getBasePtr(0, y);

			for (int x = 0; x < CURSOR_SIZE; ++x, ++srcP, ++destP)
				*destP = (*destP & ~0xff) | *srcP;
		}

		delete movie;
		delete transSurface;
		delete surface;
	}
}

void CMouseCursor::incBusyCount() {
	if (_busyCount == 0)
		setCursor(CURSOR_HOURGLASS);
	++_busyCount;
}

void CMouseCursor::decBusyCount() {
	assert(_busyCount > 0);
	if (--_busyCount == 0)
		setCursor(CURSOR_ARROW);
}

void CMouseCursor::incHideCounter() {
	if (_hideCounter++ == 0)
		CursorMan.showMouse(false);
}

void CMouseCursor::decHideCounter() {
	--_hideCounter;
	assert(_hideCounter >= 0);
	if (_hideCounter == 0)
		CursorMan.showMouse(true);
}

void CMouseCursor::suppressCursor() {
	_cursorSuppressed = true;
	CursorMan.showMouse(false);
}

void CMouseCursor::unsuppressCursor() {
	_cursorSuppressed = false;
	if (_hideCounter == 0)
		CursorMan.showMouse(true);
}

void CMouseCursor::setCursor(CursorId cursorId) {
	++_setCursorCount;

	if (cursorId != _cursorId && _busyCount == 0) {
		const CursorEntry &ce = _cursors[cursorId - 1];
		_cursorId = cursorId;

		// Set the cursor
		CursorMan.replaceCursor(ce._surface->getPixels(), CURSOR_SIZE, CURSOR_SIZE,
			ce._centroid.x, ce._centroid.y, 0, false, &ce._surface->format);
	}
}

void CMouseCursor::update() {
	if (!_inputEnabled && _moveStartTime) {
		uint32 time = CLIP(g_system->getMillis(), _moveStartTime, _moveEndTime);
		Common::Point pt(
			_moveStartPos.x + (_moveDestPos.x - _moveStartPos.x) *
				(int)(time - _moveStartTime) / (int)(_moveEndTime - _moveStartTime),
			_moveStartPos.y + (_moveDestPos.y - _moveStartPos.y) *
			(int)(time - _moveStartTime) / (int)(_moveEndTime - _moveStartTime)
		);

		if (pt != g_vm->_events->getMousePos()) {
			g_vm->_events->setMousePos(pt);

			CInputHandler &inputHandler = *CScreenManager::_screenManagerPtr->_inputHandler;
			CMouseMoveMsg msg(pt, 0);
			inputHandler.handleMessage(msg, false);
		}

		if (time == _moveEndTime)
			_moveStartTime = _moveEndTime = 0;
	}
}

void CMouseCursor::disableControl() {
	_inputEnabled = false;
	CScreenManager::_screenManagerPtr->_inputHandler->incLockCount();
}

void CMouseCursor::enableControl() {
	_inputEnabled = true;
	_fieldE8 = 0;
	CScreenManager::_screenManagerPtr->_inputHandler->decLockCount();
}

void CMouseCursor::setPosition(const Point &pt, double duration) {
	_moveStartPos = g_vm->_events->getMousePos();
	_moveDestPos = pt;
	_moveStartTime = g_system->getMillis();
	_moveEndTime = _moveStartTime + (int)duration;
	update();
}

} // End of namespace Titanic
