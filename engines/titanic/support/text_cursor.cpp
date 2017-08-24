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

#include "titanic/support/text_cursor.h"
#include "titanic/events.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"
#include "common/textconsole.h"

namespace Titanic {

CTextCursor::CTextCursor(CScreenManager *screenManager) :
		_screenManager(screenManager), _active(false), _blinkVisible(false),
		_backRenderSurface(nullptr), _frontRenderSurface(nullptr),
		_blinkDelay(300), _size(2, 10), _priorBlinkTime(0),
		_cursorR(0), _cursorG(0), _cursorB(0), _mode(-1) {
	_surface = screenManager->createSurface(10, 10, 16);
}

CTextCursor::~CTextCursor() {
	delete _surface;
}

void CTextCursor::setColor(byte r, byte g, byte b) {
	_cursorR = r;
	_cursorG = g;
	_cursorB = b;
}

void CTextCursor::show() {
	_backRenderSurface = _screenManager->getSurface(SURFACE_BACKBUFFER);
	_frontRenderSurface = _screenManager->getFrontRenderSurface();
	_active = true;
	_priorBlinkTime = g_vm->_events->getTicksCount();
}

void CTextCursor::hide() {
	_active = false;
}

void CTextCursor::draw() {
	if (!_active)
		return;

	// Handle updating whether the blinking cursor is visible or not
	uint newTicks = g_vm->_events->getTicksCount();
	while (newTicks > (_priorBlinkTime + _blinkDelay)) {
		_priorBlinkTime += _blinkDelay;
		_blinkVisible = !_blinkVisible;
	}

	if (_blinkVisible) {
		Rect cursorRect = getCursorBounds();
		_surface->blitFrom(Common::Point(0, 0), _backRenderSurface, &cursorRect);

		if (!_screenBounds.isEmpty())
			// Limit the cursor rect to only within designated screen area
			cursorRect.constrain(_screenBounds);

		if (!cursorRect.isEmpty()) {
			// Draw cursor onto the screen
			_backRenderSurface->_ddSurface->fillRect(&cursorRect,
				_cursorR, _cursorG, _cursorB);
		}

		//_screenManager->blitFrom(SURFACE_BACKBUFFER, _surface, &_pos);
	}
}

} // End of namespace Titanic
