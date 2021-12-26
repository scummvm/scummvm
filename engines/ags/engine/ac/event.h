/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_AC_EVENT_H
#define AGS_ENGINE_AC_EVENT_H

#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/script/runtime_script_value.h"

namespace AGS3 {

// parameters to run_on_event
#define GE_LEAVE_ROOM 1
#define GE_ENTER_ROOM 2
#define GE_MAN_DIES   3
#define GE_GOT_SCORE  4
#define GE_GUI_MOUSEDOWN 5
#define GE_GUI_MOUSEUP   6
#define GE_ADD_INV       7
#define GE_LOSE_INV      8
#define GE_RESTORE_GAME  9

#define MAXEVENTS 15

#define EV_TEXTSCRIPT 1
#define EV_RUNEVBLOCK 2
#define EV_FADEIN     3
#define EV_IFACECLICK 4
#define EV_NEWROOM    5
#define TS_REPEAT   1
#define TS_KEYPRESS 2
#define TS_MCLICK   3
#define EVB_HOTSPOT 1
#define EVB_ROOM    2

struct EventHappened {
	int type;
	int data1, data2, data3;
	int player;
};

int run_claimable_event(const char *tsname, bool includeRoom, int numParams, const RuntimeScriptValue *params, bool *eventWasClaimed);
// runs the global script on_event fnuction
void run_on_event(int evtype, RuntimeScriptValue &wparam);
void run_room_event(int id);
void run_event_block_inv(int invNum, int event);
// event list functions
void setevent(int evtyp, int ev1 = 0, int ev2 = -1000, int ev3 = 0);
void force_event(int evtyp, int ev1 = 0, int ev2 = -1000, int ev3 = 0);
void process_event(EventHappened *evp);
void runevent_now(int evtyp, int ev1, int ev2, int ev3);
void processallevents(int numev, EventHappened *evlist);
void update_events();
// end event list functions
void ClaimEvent();

} // namespace AGS3

#endif
