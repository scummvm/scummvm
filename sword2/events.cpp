/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "bs2/console.h"
#include "bs2/debug.h"
#include "bs2/defs.h"
#include "bs2/events.h"
#include "bs2/interpreter.h"
#include "bs2/logic.h"
#include "bs2/memory.h"
#include "bs2/object.h"
#include "bs2/sync.h"

namespace Sword2 {

_event_unit event_list[MAX_events];

void Init_event_system(void) {
	for (int i = 0; i < MAX_events; i++) {
		//denotes free slot
		event_list[i].id = 0;
	}
}

#ifdef _SWORD2_DEBUG
uint32 CountEvents(void) {
	uint32 count = 0;

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id)
			count++;
	}

	return count;
}
#endif

int32 Logic::fnRequestSpeech(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of target to catch the event and startup speech
	//		  servicing

	int i;

	for (i = 0; i < MAX_events; i++) {
		if (event_list[i].id == (uint32) params[0])
			break;

		if (!event_list[i].id)
			break;
	}

	if (i == MAX_events)
		Con_fatal_error("fnSetEvent out of event slots");

	// found that slot

	// id of person to stop
	event_list[i].id = params[0];

	// full script id to interact with - megas run their own 7th script
	event_list[i].interact_id = (params[0] * 65536) + 6;

	return IR_CONT;
}

void Set_player_action_event(uint32 id, uint32 interact_id) {
	int i;

	for (i = 0; i < MAX_events; i++) {
		if (event_list[i].id == id)
			break;

		if (!event_list[i].id)
			break;
	}

	if (i == MAX_events)
		Con_fatal_error("Set_event out of event slots");

	// found that slot

	// id of person to stop
	event_list[i].id = id;

	// full script id of action script number 2
	event_list[i].interact_id = (interact_id * 65536) + 2;
}

int32 Logic::fnSetPlayerActionEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script this code is the same as the mouse
	// engine calls when you click on an object - here, a third party
	// does the clicking IYSWIM

	// note - this routine used CUR_PLAYER_ID as the target

	// params:	0 id to interact with

	// search for an existing event or a slot

	int i;

	for (i = 0; i < MAX_events; i++) {
		if (event_list[i].id == CUR_PLAYER_ID)
			break;

		if (!event_list[i].id)
			break;
	}

	if (i == MAX_events)
		Con_fatal_error("Set_event out of event slots");

	// found that slot

	// id of person to stop
	event_list[i].id = CUR_PLAYER_ID;

	// full script id of action script number 2
	event_list[i].interact_id = (params[0] * 65536) + 2;

	return IR_CONT;
}

int32 Logic::fnSendEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script

	// params:	0 id to recieve event
	//		1 script to run

	// search for an existing event or a slot

	int i;

	for (i = 0; i < MAX_events; i++) {
		if (event_list[i].id == (uint32) params[0])
			break;

		if (!event_list[i].id)
			break;
	}

	if (i == MAX_events)
		Con_fatal_error("fnSendEvent out of event slots");

	// found that slot

	// id of person to stop
	event_list[i].id = params[0];

	//full script id
	event_list[i].interact_id = params[1];

	return IR_CONT;
}

int32 Logic::fnCheckEventWaiting(int32 *params) {
	// returns yes/no in RESULT

	// params:	none

	RESULT = 0;

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID) {
			RESULT = 1;
			break;
		}
	}

	return IR_CONT;
}

// like fnCheckEventWaiting, but starts the event rather than setting RESULT
// to 1

int32 Logic::fnCheckForEvent(int32 *params) {
	// params:	none

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID) {
			// start the event
			// run 3rd script of target object on level 1
			logicOne(event_list[i].interact_id);
			// clear the event slot
			event_list[i].id = 0;
			return IR_TERMINATE;
		}
	}

	return IR_CONT;
}

// combination of fnPause and fnCheckForEvent
// - ie. does a pause, but also checks for event each cycle

int32 Logic::fnPauseForEvent(int32 *params) {
	// returns yes/no in RESULT

	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	Object_logic *ob_logic = (Object_logic *)params[0];

	// first, check for an event

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID) {
			// reset the 'looping' flag
			ob_logic->looping = 0;

			// start the event
			// run 3rd script of target object on level 1
			logicOne(event_list[i].interact_id);

			// clear the event slot
			event_list[i].id = 0;
			return IR_TERMINATE;
		}
	}

	// no event, so do the fnPause bit

	// start the pause
	if (ob_logic->looping == 0) {
		ob_logic->looping = 1;
		// no. of game cycles
		ob_logic->pause = params[1];
	}

	// if non-zero
	if (ob_logic->pause) {
		// decrement the pause count
		ob_logic->pause--;

		// drop out of script, but call this again next cycle
		return IR_REPEAT;
	} else {
		// pause count is zerp
		ob_logic->looping = 0;

		// continue script
		return IR_CONT;
	}
}

bool Check_event_waiting(void) {
	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID)
			return true;
	}

	return false;
}

int32 Logic::fnClearEvent(int32 *params) {
	// params:	none

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID) {
			//clear the slot
			event_list[i].id = 0;
			return IR_CONT;
		}
	}

	return IR_CONT;
}

void Start_event(void) {
	// call this from stuff like fnWalk
	// you must follow with a return IR_TERMINATE

	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == ID) {
			// run 3rd script of target object on level 1
			g_logic.logicOne(event_list[i].interact_id);

			//clear the slot
			event_list[i].id = 0;
			return;
		}
	}

	// oh dear - stop the system
	Con_fatal_error("Start_event can't find event for id %d", ID);
}

int32 Logic::fnStartEvent(int32 *params) {
	// params:	none

	for (int i = 0; i < MAX_events; i++)
		if (event_list[i].id == ID) {
			// run 3rd script of target object on level 1
			logicOne(event_list[i].interact_id);

			// clear the slot
			event_list[i].id = 0;
			return IR_TERMINATE;
		}

	// oh dear - stop the system
	Con_fatal_error("fnStartEvent can't find event for id %d", ID);
	return 0;	// never called - but lets stop them bloody errors
}

void Kill_all_ids_events(uint32 id) {
	for (int i = 0; i < MAX_events; i++) {
		if (event_list[i].id == id) {
			// clear the slot
			event_list[i].id = 0;
		}
	}
}

} // End of namespace Sword2
