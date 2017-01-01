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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/gui.h"

namespace MacVenture {

static void cursorTimerHandler(void *refCon);

static const ClickState _transitionTable[kCursorStateCount][kCursorInputCount] = {
	/*				Button down,		Button Up,		Tick		*/
	/* Idle */		{kCursorSCStart,	kCursorIdle,	kCursorIdle	},
	/* SC Start */	{kCursorSCStart,	kCursorDCStart,	kCursorSCDrag},
	/* SC Do */		{kCursorSCDrag, 	kCursorIdle,	kCursorSCDrag},
	/* DC Start */	{kCursorDCDo,		kCursorDCStart,	kCursorSCSink},
	/* DC Do */		{kCursorDCDo,		kCursorIdle,	kCursorDCDo	},
	/* SC Sink */	{kCursorIdle,		kCursorIdle,	kCursorIdle	}
};

Cursor::Cursor(Gui *gui) {
	_gui = gui;
	_state = kCursorIdle;
}
Cursor::~Cursor() {}

void Cursor::tick() {
	changeState(kTickCol);
}

bool Cursor::processEvent(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE) {
		_pos = event.mouse;
		return true;
	}
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		changeState(kButtonDownCol);
		return true;
	}
	if (event.type == Common::EVENT_LBUTTONUP) {
		changeState(kButtonUpCol);
		return true;
	}

	return false;
}

Common::Point Cursor::getPos() {
	return _pos;
}

bool Cursor::canSelectDraggable() {
	return _state == kCursorSCDrag;
}

void Cursor::changeState(CursorInput input) {
	debugC(3, kMVDebugGUI, "Change cursor state: [%d] -> [%d]", _state, _transitionTable[_state][input]);
	if (_state != _transitionTable[_state][input]) {
		executeStateOut();
		_state = _transitionTable[_state][input];
		executeStateIn();
	}
}

void Cursor::executeStateIn() {
	switch (_state) {
	case kCursorSCStart:
		g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 300000, this, "macVentureCursor");
		_gui->selectForDrag(_pos);
		break;
	case kCursorDCStart:
		g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 300000, this, "macVentureCursor");
		break;
	case kCursorSCSink:
		_gui->handleSingleClick();
		changeState(kTickCol);
		break;
	default:
		break;
	}
}

void Cursor::executeStateOut() {
	switch (_state) {
	case kCursorIdle:
		break;
	case kCursorSCStart:
		g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
		break;
	case kCursorSCDrag:
		_gui->handleSingleClick();
		break;
	case kCursorDCStart:
		g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
		break;
	case kCursorDCDo:
		_gui->handleDoubleClick();
		break;
	default:
		break;
	}
}

static void cursorTimerHandler(void *refCon) {
	Cursor *cursor = (Cursor *)refCon;
	cursor->tick();
}


} // End of namespace MacVenture
