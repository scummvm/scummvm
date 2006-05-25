/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "lure/events.h"
#include "lure/system.h"
#include "lure/res.h"

namespace Lure {

static Mouse *int_mouse = NULL;

Mouse &Mouse::getReference() { 
	return *int_mouse; 
}

Mouse::Mouse() {
	int_mouse = this;

	_lButton = false; 
	_rButton = false;
	_cursorNum = 0;
	_x = 0;
	_y = 0;
	setCursorNum(0);
}

Mouse::~Mouse() {
}

void Mouse::handleEvent(OSystem::Event event) {
	_x = (int16) event.mouse.x;
	_y = (int16) event.mouse.y;

	switch (event.type) {
	case OSystem::EVENT_LBUTTONDOWN:
		_lButton = true;
		break;
	case OSystem::EVENT_LBUTTONUP:
		_lButton = false;
		break;
	case OSystem::EVENT_RBUTTONDOWN:
		_rButton = true;
		break;
	case OSystem::EVENT_RBUTTONUP:
		_rButton = false;
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

void Mouse::setCursorNum(uint8 cursorNum) {
	int hotspotX = 7, hotspotY = 7;
	if ((cursorNum == CURSOR_ARROW) || (cursorNum == CURSOR_MENUBAR)) {
		hotspotX = 0; 
		hotspotY = 0;
	}

	setCursorNum(cursorNum, hotspotX, hotspotY);
}

void Mouse::setCursorNum(uint8 cursorNum, int hotspotX, int hotspotY) {
	Resources &res = Resources::getReference();

	_cursorNum = cursorNum;
	byte *cursorAddr = res.getCursor(cursorNum);
	CursorMan.replaceCursor(cursorAddr, CURSOR_WIDTH, CURSOR_HEIGHT, hotspotX, hotspotY, 0);
}

void Mouse::setPosition(int newX, int newY) {
	System::getReference().warpMouse(newX, newY);
}

void Mouse::waitForRelease() {
	OSystem &system = System::getReference();
	Events &e = Events::getReference();

	do {
		e.pollEvent();
		system.delayMillis(20);
	} while (!e.quitFlag && (lButton() || rButton()));
}

/*--------------------------------------------------------------------------*/

static Events *int_events = NULL;

Events::Events() {
	int_events = this;
	quitFlag = false;
}

Events &Events::getReference() {
	return *int_events;
}


bool Events::pollEvent() {
	if (!System::getReference().pollEvent(_event)) return false;

	// Handle keypress
	switch (_event.type) {
	case OSystem::EVENT_QUIT:
		quitFlag = true;
		break;

	case OSystem::EVENT_LBUTTONDOWN:
	case OSystem::EVENT_LBUTTONUP:
	case OSystem::EVENT_RBUTTONDOWN:
	case OSystem::EVENT_RBUTTONUP:
	case OSystem::EVENT_MOUSEMOVE:
	case OSystem::EVENT_WHEELUP:
	case OSystem::EVENT_WHEELDOWN:
		Mouse::getReference().handleEvent(_event);
		break;

	default:
 		break;
	}

	return true;
}

void Events::waitForPress() {
	OSystem &system = System::getReference();
	bool keyButton = false;
	while (!keyButton) {
		if (pollEvent()) {
			if (_event.type == OSystem::EVENT_QUIT) return;
			else if (_event.type == OSystem::EVENT_KEYDOWN) keyButton = true;
			else if ((_event.type == OSystem::EVENT_LBUTTONDOWN) ||
				(_event.type == OSystem::EVENT_RBUTTONDOWN)) {
				keyButton = true;
				Mouse::getReference().waitForRelease();				
			}
		}
		system.delayMillis(20);
	}
}

} // end of namespace Lure
