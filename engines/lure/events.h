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

#ifndef __lure_events_h__
#define __lure_events_h__

#include "common/stdafx.h"
#include "common/str.h"
#include "lure/luredefs.h"
#include "lure/disk.h"

namespace Lure {

class Mouse {
private:
	uint8 _cursorNum;
	int16 _x, _y;
	bool _lButton, _rButton;
public:
	Mouse();
	~Mouse();
	static Mouse &getReference(); 
	void handleEvent(OSystem::Event event);

	void cursorOn();
	void cursorOff();
	void setCursorNum(uint8 cursorNum);
	void setCursorNum(uint8 cursorNum, int hotspotX, int hotspotY);
	uint8 getCursorNum() { return _cursorNum; }
	void setPosition(int x, int y);
	int16 x() { return _x; }
	int16 y() { return _y; }
	bool lButton() { return _lButton; }
	bool rButton() { return _rButton; }
	void waitForRelease();
};

class Events {
private:
	OSystem::Event _event;
public:
	bool quitFlag;

	Events();
	static Events &getReference();

	bool pollEvent();
	void waitForPress();
	OSystem::Event event() { return _event; }
	OSystem::EventType type() { return _event.type; }
};

} // End of namespace Lure

#endif
