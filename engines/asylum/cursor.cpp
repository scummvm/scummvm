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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/cursorman.h"
#include "asylum/cursor.h"

namespace Asylum {

Cursor::Cursor(ResourcePack *res) {
	_resPack = res;
	_mouseX  = 0;
	_mouseY  = 0;
	cursorLoaded = false;
}

Cursor::~Cursor() {
	if (cursorLoaded)
		delete _cursorResource;
}

void Cursor::load(uint32 index) {
	if (cursorLoaded)
		delete _cursorResource;

	_cursorResource = new GraphicResource(_resPack, index);
	_cursorStep		= 1;
	_curFrame	    = 0;
	cursorLoaded    = true;
}

void Cursor::hide() {
	CursorMan.showMouse(false);
}

void Cursor::show() {
	CursorMan.showMouse(true);
}

void Cursor::set(byte *data, byte width, byte height) {
	CursorMan.replaceCursor(data, width, height, 0, 0, 0);
}

void Cursor::set(int frame) {
	if (frame >= 0) {
		GraphicFrame *mouseCursor = _cursorResource->getFrame(frame);
		set((byte *)mouseCursor->surface.pixels, mouseCursor->surface.w, mouseCursor->surface.h);
		_curFrame = frame;
	} else {
		_cursorStep = 1;
	}
}

void Cursor::setCoords(uint32 mouseX, uint32 mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;
}

void Cursor::animate() {
	_curFrame += _cursorStep;
	if (_curFrame == 0)
		_cursorStep = 1;
	if (_curFrame == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	set(_curFrame);
}

void Cursor::update(CommonResources *cr, int currentAction) {
	uint32 newCursor = 0;

	// Change cursor
	switch (currentAction) {
	case kWalkN:
		newCursor = cr->curScrollUp;
		break;
	case kWalkNE:
		newCursor = cr->curScrollUpRight;
		break;
	case kWalkNW:
		newCursor = cr->curScrollUpLeft;
		break;
	case kWalkS:
		newCursor = cr->curScrollDown;
		break;
	case kWalkSE:
		newCursor = cr->curScrollDownRight;
		break;
	case kWalkSW:
		newCursor = cr->curScrollDownLeft;
		break;
	case kWalkW:
		newCursor = cr->curScrollLeft;
		break;
	case kWalkE:
		newCursor = cr->curScrollRight;
		break;
	}

	// FIXME Added the newCursor > 0 check because cursors
	// aren't working properly with the new actor code
	if (_cursorResource->getEntryNum() != newCursor && newCursor > 0)
		load(newCursor);
}

} // end of namespace Asylum
