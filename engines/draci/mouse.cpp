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

#include "draci/draci.h"
#include "draci/mouse.h"
#include "draci/barchive.h"

namespace Draci {

Mouse::Mouse(DraciEngine *vm) {
	_x = 0;
	_y = 0;
	_lButton = false;
	_rButton = false;
	_cursorType = kNormalCursor;
	_vm = vm;
}

void Mouse::handleEvent(Common::Event event) {
	_x = (uint16) event.mouse.x;
	_y = (uint16) event.mouse.y;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		debugC(6, kDraciGeneralDebugLevel, "Left button down (x: %u y: %u)", _x, _y);
		_lButton = true;
		break;
	case Common::EVENT_LBUTTONUP:
		debugC(6, kDraciGeneralDebugLevel, "Left button up (x: %u y: %u)", _x, _y);
		_lButton = false;
		break;
	case Common::EVENT_RBUTTONDOWN:
		debugC(6, kDraciGeneralDebugLevel, "Right button down (x: %u y: %u)", _x, _y);
		_rButton = true;
		break;
	case Common::EVENT_RBUTTONUP:
		debugC(6, kDraciGeneralDebugLevel, "Right button up (x: %u y: %u)", _x, _y);
		_rButton = false;
		break;
	case Common::EVENT_MOUSEMOVE:
		setPosition(_x, _y);
		break;	
	default:
		break;
	}
}

void Mouse::cursorOn() {
	CursorMan.showMouse(true);
}

void Mouse::cursorOff() {
	CursorMan.showMouse(false);
}

void Mouse::setPosition(uint16 x, uint16 y) {
	_vm->_system->warpMouse(x, y);
}

// FIXME: Handle hotspots properly
// TODO: Implement a resource manager
void Mouse::setCursorType(CursorType cur) {
	_cursorType = cur;
	
	Common::String path("HRA.DFW");
	BAFile *f;
	BArchive ar;
	ar.openArchive(path);
	
	if(ar.isOpen()) {
		f = ar.getFile(_cursorType);	
	} else {
		debugC(2, kDraciGeneralDebugLevel, "ERROR - Archive not opened - %s", path.c_str());
		return;
	}	

	Sprite sp(f->_data, f->_length, 0, 0, 0);
	CursorMan.replaceCursorPalette(_vm->_screen->getPalette(), 0, kNumColours);
	CursorMan.replaceCursor(sp.getBuffer(), sp.getWidth(), sp.getHeight(), 
			sp.getWidth() / 2, sp.getHeight() / 2);
}

}
