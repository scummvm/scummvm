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
 * User events processing and utility functions
 */

#ifndef TINSEL_EVENTS_H
#define TINSEL_EVENTS_H

#include "tinsel/dw.h"
#include "tinsel/coroutine.h"

namespace Tinsel {

enum BUTEVENT {
	BE_NONE, BE_SLEFT, BE_DLEFT, BE_SRIGHT, BE_DRIGHT,
	BE_LDSTART, BE_LDEND, BE_RDSTART, BE_RDEND,
	BE_UNKNOWN
};


enum KEYEVENT {
	ESC_KEY, QUIT_KEY, SAVE_KEY, LOAD_KEY, OPTION_KEY,
	PGUP_KEY, PGDN_KEY, HOME_KEY, END_KEY,
	WALKTO_KEY, ACTION_KEY, LOOK_KEY,
	NOEVENT_KEY
};


/**
 * Reasons for running Glitter code.
 * Do not re-order these as equivalent CONSTs are defined in the master
 * scene Glitter source file for testing against the event() library function.
 */
enum USER_EVENT {
	POINTED, WALKTO, ACTION, LOOK,
	ENTER, LEAVE, STARTUP, CONVERSE,
	UNPOINT, PUTDOWN,
	NOEVENT
};


void AllowDclick(CORO_PARAM, BUTEVENT be);
bool GetControl(int param);

void RunPolyTinselCode(HPOLYGON hPoly, USER_EVENT event, BUTEVENT be, bool tc);
void effRunPolyTinselCode(HPOLYGON hPoly, USER_EVENT event, int actor);

void ProcessButEvent(BUTEVENT be);
void ProcessKeyEvent(KEYEVENT ke);


int GetEscEvents(void);
int GetLeftEvents(void);
int getUserEvents(void);

uint32 getUserEventTime(void);
void resetUserEventTime(void);

void ResetEcount(void);

} // end of namespace Tinsel

#endif /* TINSEL_EVENTS_H */
