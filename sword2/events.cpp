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
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "events.h"
#include "interpreter.h"
#include "logic.h"
#include "memory.h"
#include "object.h"
#include "sync.h"

_event_unit event_list[MAX_events];

void Init_event_system(void) {	//Tony4Dec96
	for (int j = 0; j < MAX_events; j++) {
		//denotes free slot
		event_list[j].id = 0;
	}
}

uint32 CountEvents(void) {
	uint32 count = 0;

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id)
			count++;
	}

	return count;
}

int32 FN_request_speech(int32 *params) {	//Tony13Nov96
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of target to catch the event and startup speech
	//		  servicing

	uint32	j = 0;

	while(1) {
		if (event_list[j].id == (uint32) params[0])
			break;

		if (!event_list[j].id)
			break;

		j++;
	}

	if (j == MAX_events)
		Con_fatal_error("FN_set_event out of event slots (%s line %u)", __FILE__, __LINE__);

	//found that slot

	//id of person to stop
	event_list[j].id = params[0];

	//full script id to interact with - megas run their own 7th script
	event_list[j].interact_id = (params[0] * 65536) + 6;

	return IR_CONT;
}

void Set_player_action_event(uint32 id, uint32 interact_id) {	//Tony4Dec96
	uint32 j = 0;

//	if (event_list[j].id != id && event_list[j].id)
//		// zip along until we find a free slot
//		while (event_list[j].id!=id || event_list[j].id) {
//			j++;
//		}

	while (1) {
		if (event_list[j].id == id)
			break;

		if (!event_list[j].id)
			break;

		j++;
	}

	if (j == MAX_events)
		Con_fatal_error("Set_event out of event slots");

	// found that slot

	//id of person to stop
	event_list[j].id = id;

	//full script id of action script number 2
	event_list[j].interact_id = (interact_id * 65536) + 2;
}

int32 FN_set_player_action_event(int32 *params) {	//Tony10Feb97
	// we want to intercept the player character and have him interact
	// with an object - from script this code is the same as the mouse
	// engine calls when you click on an object - here, a third party
	// does the clicking IYSWIM

	// note - this routine used CUR_PLAYER_ID as the target

	// params:	0 id to interact with

	uint32	j = 0;

	// search for an existing event or a slot

	while(1) {
		if (event_list[j].id == CUR_PLAYER_ID)
			break;

		if (!event_list[j].id)
			break;

		j++;
	}

	if (j == MAX_events)
		Con_fatal_error("Set_event out of event slots");

	// found that slot

	// id of person to stop
	event_list[j].id = CUR_PLAYER_ID;

	// full script id of action script number 2
	event_list[j].interact_id = (params[0] * 65536) + 2;

	return IR_CONT;
}

int32 FN_send_event(int32 *params) {	//Tony28Feb97
	// we want to intercept the player character and have him interact
	// with an object - from script

	// params:	0 id to recieve event
	//		1 script to run

	uint32 j = 0;

	// Zdebug("*+*+* %d %d", params[0], params[1]);

	// search for an existing event or a slot

	while(1) {
		if (event_list[j].id == (uint32) params[0])
			break;

		if (!event_list[j].id)
			break;

		j++;
	}

	if (j == MAX_events)
		Con_fatal_error("fn_send_event out of event slots");

	// found that slot

	// id of person to stop
	event_list[j].id = params[0];

	//full script id
	event_list[j].interact_id = params[1];

	return IR_CONT;
}

int32 FN_check_event_waiting(int32 *params) {	//Tony4Dec96
	// returns yes/no in RESULT

	// no params

	RESULT = 0;

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID) {
			RESULT = 1;
			break;
		}
	}

	return IR_CONT;
}

// like FN_check_event_waiting, but starts the event rather than setting
// RESULT to 1

int32 FN_check_for_event(int32 *params)	{	// James (04mar97)
	// no params

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID) {
			// start the event
			// run 3rd script of target object on level 1
			LLogic.Logic_one(event_list[j].interact_id);
			// clear the event slot
			event_list[j].id = 0;
			return IR_TERMINATE;
		}
	}

	return IR_CONT;
}

// combination of FN_pause & FN_check_for_event
// - ie. does a pause, but also checks for event each cycle

int32 FN_pause_for_event(int32 *params) {	// James (04mar97)
	// returns yes/no in RESULT

	// params
	//     0 pointer to object's logic structure
	//     1 number of game-cycles to pause

	Object_logic *ob_logic = (Object_logic *)params[0];

	// first, check for an event

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID) {
			// reset the 'looping' flag
			ob_logic->looping = 0;

			// start the event
			// run 3rd script of target object on level 1
			LLogic.Logic_one(event_list[j].interact_id);

			// clear the event slot
			event_list[j].id = 0;
			return IR_TERMINATE;
		}
	}

	// no event, so do the FN_pause bit

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

uint32 Check_event_waiting(void) {	//Tony4Dec96
	// returns yes/no

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID)
			return 1;
	}

	return 0;
}

int32 FN_clear_event(int32 *params) {	//Tony11Mar97
//	no params
//	no return vaule

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID) {
			//clear the slot
			event_list[j].id = 0;
			return IR_CONT;
		}
	}

	return IR_CONT;
}

void Start_event(void) {	//Tony4Dec96
	// call this from stuff like fn_walk
	// you must follow with a return IR_TERMINATE

	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == ID) {
			// run 3rd script of target object on level 1
			LLogic.Logic_one( event_list[j].interact_id);

			//clear the slot
			event_list[j].id = 0;
			return;
		}
	}

	// oh dear - stop the system
	Con_fatal_error("Start_event can't find event for id %d", ID);
}

int32 FN_start_event(int32 *params) {	//Tony4Dec96
	for (int j = 0; j < MAX_events; j++)
		if (event_list[j].id == ID) {
			// run 3rd script of target object on level 1
			LLogic.Logic_one(event_list[j].interact_id);

			// clear the slot
			event_list[j].id = 0;
			return IR_TERMINATE;
		}

	//oh dear - stop the system
	Con_fatal_error("FN_start_event can't find event for id %d", ID);
	return 0;	//never called - but lets stop them bloody errors
}

void Kill_all_ids_events(uint32 id) {	//Tony18Dec96
	for (int j = 0; j < MAX_events; j++) {
		if (event_list[j].id == id) {
			// clear the slot
			event_list[j].id = 0;
		}
	}
}
