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

#include "asylum/system/cursor.h"

#include "asylum/system/graphics.h"

#include "graphics/cursorman.h"

namespace Asylum {

const uint32 CURSOR_UPDATE_TICKS = 100;

Cursor::Cursor(AsylumEngine *engine) : _vm(engine) {
	_pos.x = 0;
	_pos.y = 0;
	_cursorRes = 0;
}

Cursor::~Cursor() {
	if (_cursorRes)
		delete _cursorRes;
}

/*
void Cursor::load(int32 index) {
	if (cursorLoaded)
		delete _cursorResource;

	_cursorResource = new GraphicResource(_vm, index);
	_cursorStep		= 1;
	_curFrame	    = 0;
	cursorLoaded    = true;
}
*/

void Cursor::hide() {
	CursorMan.showMouse(false);
}

void Cursor::show() {
	CursorMan.showMouse(true);
}

void Cursor::set(ResourceId resourceId, int32 cntr, int32 flgs, int32 frames) {
	if (_cursorRes)
		delete _cursorRes;

	_cursorRes = new GraphicResource(_vm, resourceId);

	if (frames >= 0)
		frameCount = frames;
	else
		frameCount = _cursorRes->getFrameCount();
	this->flags   = flgs;
	this->counter = cntr;
	currentFrame = 0;
	_cursor_byte_45756C = 1;

	update();
}

void Cursor::update() {
	GraphicFrame *fra = _cursorRes->getFrame(currentFrame);
	CursorMan.replaceCursor((byte *)fra->surface.pixels,
			fra->surface.w,
			fra->surface.h,
			0, 0, 0);
}

/*
void Cursor::set(byte *data, byte width, byte height) {
	CursorMan.replaceCursor(data, width, height, 0, 0, 0);
}

void Cursor::set(int32 frame) {
	if (frame >= 0) {
		GraphicFrame *mouseCursor = _cursorResource->getFrame(frame);
		set((byte *)mouseCursor->surface.pixels, mouseCursor->surface.w, mouseCursor->surface.h);
		_curFrame = frame;
	} else {
		_cursorStep = 1;
	}
}
*/

void Cursor::move(int16 x, int16 y) {
	_pos.x = x;
	_pos.y = y;
}


void Cursor::animate() {
	/*
	_curFrame += _cursorStep;
	if (_curFrame == 0)
		_cursorStep = 1;
	if (_curFrame == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	set(_curFrame);
	*/
}

/*
void Cursor::update(WorldStats *ws, int32 currentAction) {
	uint32 newCursor = 0;

	// Change cursor
	switch (currentAction) {
	case kWalkN:
		newCursor = ws->curScrollUp;
		break;
	case kWalkNE:
		newCursor = ws->curScrollUpRight;
		break;
	case kWalkNW:
		newCursor = ws->curScrollUpLeft;
		break;
	case kWalkS:
		newCursor = ws->curScrollDown;
		break;
	case kWalkSE:
		newCursor = ws->curScrollDownRight;
		break;
	case kWalkSW:
		newCursor = ws->curScrollDownLeft;
		break;
	case kWalkW:
		newCursor = ws->curScrollLeft;
		break;
	case kWalkE:
		newCursor = ws->curScrollRight;
		break;
	}

	// FIXME Added the newCursor > 0 check because cursors
	// aren't working properly with the new actor code
	if (_cursorResource->getEntryNum() != newCursor && newCursor > 0)
		load(newCursor);
}
*/

} // end of namespace Asylum
