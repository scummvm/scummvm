//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__EVENT_H
#define __AGS_EE_AC__EVENT_H

#include "ac/runtime_defines.h"
#include "script/runtimescriptvalue.h"

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
    int data1,data2,data3;
    int player;
};

int run_claimable_event(const char *tsname, bool includeRoom, int numParams, const RuntimeScriptValue *params, bool *eventWasClaimed);
// runs the global script on_event fnuction
void run_on_event (int evtype, RuntimeScriptValue &wparam);
void run_room_event(int id);
void run_event_block_inv(int invNum, int event);
// event list functions
void setevent(int evtyp,int ev1=0,int ev2=-1000,int ev3=0);
void force_event(int evtyp,int ev1=0,int ev2=-1000,int ev3=0);
void process_event(EventHappened*evp);
void runevent_now (int evtyp, int ev1, int ev2, int ev3);
void processallevents(int numev,EventHappened*evlist);
void update_events();
// end event list functions
void ClaimEvent();

extern int in_enters_screen,done_es_error;
extern int in_leaves_screen;

extern EventHappened event[MAXEVENTS+1];
extern int numevents;

extern const char*evblockbasename;
extern int evblocknum;

extern int eventClaimed;

extern const char*tsnames[4];

#endif // __AGS_EE_AC__EVENT_H

