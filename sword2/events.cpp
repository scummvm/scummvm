/* Copyright (C) 1994-2005 Revolution Software Ltd
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

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/console.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

void Logic::sendEvent(uint32 id, uint32 interact_id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id || !_eventList[i].id) {
			_eventList[i].id = id;
			_eventList[i].interact_id = interact_id;
			return;
		}
	}

	error("sendEvent() ran out of event slots");
}

void Logic::setPlayerActionEvent(uint32 id, uint32 interact_id) {
	// Full script id of action script number 2
	sendEvent(id, (interact_id << 16) | 2);
}

int Logic::checkEventWaiting(void) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == _scriptVars[ID])
			return 1;
	}

	return 0;
}

void Logic::startEvent(void) {
	// call this from stuff like fnWalk
	// you must follow with a return IR_TERMINATE

	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == _scriptVars[ID]) {
			logicOne(_eventList[i].interact_id);
			_eventList[i].id = 0;
			return;
		}
	}

	error("startEvent() can't find event for id %d", _scriptVars[ID]);
}

void Logic::clearEvent(uint32 id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id) {
			_eventList[i].id = 0;
			return;
		}
	}
}

void Logic::killAllIdsEvents(uint32 id) {
	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id == id)
			_eventList[i].id = 0;
	}
}

int32 Logic::fnRequestSpeech(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of target to catch the event and startup speech
	//		  servicing

	// Full script id to interact with - megas run their own 7th script
	sendEvent(params[0], (params[0] << 16) | 6);
	return IR_CONT;
}

int32 Logic::fnSetPlayerActionEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script this code is the same as the mouse
	// engine calls when you click on an object - here, a third party
	// does the clicking IYSWIM

	// note - this routine used CUR_PLAYER_ID as the target

	// params:	0 id to interact with

	setPlayerActionEvent(CUR_PLAYER_ID, params[0]);
	return IR_CONT;
}

int32 Logic::fnSendEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script

	// params:	0 id to receive event
	//		1 script to run

	sendEvent(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnCheckEventWaiting(int32 *params) {
	// params:	none

	_scriptVars[RESULT] = checkEventWaiting();
	return IR_CONT;
}

// like fnCheckEventWaiting, but starts the event rather than setting RESULT
// to 1

int32 Logic::fnCheckForEvent(int32 *params) {
	// params:	none

	if (checkEventWaiting()) {
		startEvent();
		return IR_TERMINATE;
	}

	return IR_CONT;
}

// combination of fnPause and fnCheckForEvent
// - ie. does a pause, but also checks for event each cycle

int32 Logic::fnPauseForEvent(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	if (checkEventWaiting()) {
		ob_logic->looping = 0;
		startEvent();
		return IR_TERMINATE;
	}

	return fnPause(params);
}

int32 Logic::fnClearEvent(int32 *params) {
	// params:	none

	clearEvent(_scriptVars[ID]);
	return IR_CONT;
}

int32 Logic::fnStartEvent(int32 *params) {
	// params:	none

	startEvent();
	return IR_TERMINATE;
}

// For the debugger

uint32 Logic::countEvents(void) {
	uint32 count = 0;

	for (int i = 0; i < MAX_events; i++) {
		if (_eventList[i].id)
			count++;
	}

	return count;
}

void Logic::printEventList(void) {
	Debug_Printf("EVENT LIST:\n");

	for (uint32 i = 0; i < MAX_events; i++) {
		if (_eventList[i].id) {
			byte buf[NAME_LEN];
			uint32 target = _eventList[i].id;
			uint32 script = _eventList[i].interact_id;

			Debug_Printf("slot %2d: id = %s (%d)\n", i, _vm->fetchObjectName(target, buf), target);
			Debug_Printf("         script = %s (%d) pos %d\n", _vm->fetchObjectName(script / 65536, buf), script / 65536, script % 65536);
		}
	}
}

} // End of namespace Sword2
