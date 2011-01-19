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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// This module contains all the scheduling and timing stuff

#include "common/system.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/util.h"

namespace Hugo {

Scheduler_v1d::Scheduler_v1d(HugoEngine *vm) : Scheduler(vm) {
}

Scheduler_v1d::~Scheduler_v1d() {
}

const char *Scheduler_v1d::getCypher() {
	return "Copyright (c) 1990, Gray Design Associates";
}

uint32 Scheduler_v1d::getTicks() {
	return getDosTicks(false);
}

/**
* Delete an event structure (i.e. return it to the free list)
* Note that event is assumed at head of queue (i.e. earliest).  To delete
* an event from the middle of the queue, merely overwrite its action type
* to be ANULL
*/
void Scheduler_v1d::delQueue(event_t *curEvent) {
	debugC(4, kDebugSchedule, "delQueue()");

	if (curEvent == _headEvent)                     // If p was the head ptr
		_headEvent = curEvent->nextEvent;           // then make new head_p

	if (_headEvent)
		_headEvent->prevEvent = 0;                  // Mark end of list
	else
		_tailEvent = 0;                             // Empty queue

	curEvent->nextEvent = _freeEvent;               // Return p to free list
	if (_freeEvent)                                 // Special case, if free list was empty
		_freeEvent->prevEvent = curEvent;
	_freeEvent = curEvent;
}

/**
* This is the scheduler which runs every tick.  It examines the event queue
* for any events whose time has come.  It dequeues these events and performs
* the action associated with the event, returning it to the free queue
*/
void Scheduler_v1d::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	uint32 ticker = getTicks();                     // The time now, in ticks
	event_t *curEvent = _headEvent;                 // The earliest event

	while (curEvent && (curEvent->time <= ticker))  // While mature events found
		curEvent = doAction(curEvent);              // Perform the action (returns next_p)
}

void Scheduler_v1d::delEventType(action_t actTypeDel) {
	// Note: actions are not deleted here, simply turned into NOPs!
	event_t *wrkEvent = _headEvent;                 // The earliest event
	while (wrkEvent) {                              // While events found in list
		if (wrkEvent->action->a20.actType == actTypeDel)
			wrkEvent->action->a20.actType = ANULL;
		wrkEvent = wrkEvent->nextEvent;
	}
}

void Scheduler_v1d::promptAction(act *action) {
	Utils::Box(BOX_PROMPT, "%s", _vm->_file->fetchString(action->a3.promptIndex));
	char response[256];
	strcpy(response, _vm->_file->fetchString(action->a3.responsePtr[0]));
	if (action->a3.encodedFl) {
		warning("Encrypted flag set");
		decodeString(response);
	}

	warning("STUB: doAction(act3), expecting answer %s", response);

	// TODO: The answer of the player is not handled currently! Once it'll be read in the messageBox, uncomment this block
#if 0
	if (strstr (response, action->a3.response))
		insertActionList(action->a3.actPassIndex);
	else
		insertActionList(action->a3.actFailIndex);
#endif

	// HACK: As the answer is not read, currently it's always considered correct
	insertActionList(action->a3.actPassIndex);
}
} // End of namespace Hugo
