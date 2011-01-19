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
#include "hugo/global.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/util.h"

namespace Hugo {

Scheduler_v2d::Scheduler_v2d(HugoEngine *vm) : Scheduler_v1d(vm) {
}

Scheduler_v2d::~Scheduler_v2d() {
}

const char *Scheduler_v2d::getCypher() {
	return "Copyright 1991, Gray Design Associates";
}

/**
* Delete an event structure (i.e. return it to the free list)
* Historical note:  Originally event p was assumed to be at head of queue
* (i.e. earliest) since all events were deleted in order when proceeding to
* a new screen.  To delete an event from the middle of the queue, the action
* was overwritten to be ANULL.  With the advent of GLOBAL events, delQueue
* was modified to allow deletes anywhere in the list, and the DEL_EVENT
* action was modified to perform the actual delete.
*/
void Scheduler_v2d::delQueue(event_t *curEvent) {
	debugC(4, kDebugSchedule, "delQueue()");

	if (curEvent == _headEvent) {                   // If p was the head ptr
		_headEvent = curEvent->nextEvent;           // then make new head_p
	} else {                                        // Unlink p
		curEvent->prevEvent->nextEvent = curEvent->nextEvent;
		if (curEvent->nextEvent)
			curEvent->nextEvent->prevEvent = curEvent->prevEvent;
		else
			_tailEvent = curEvent->prevEvent;
	}

	if (_headEvent)
		_headEvent->prevEvent = 0;                  // Mark end of list
	else
		_tailEvent = 0;                             // Empty queue

	curEvent->nextEvent = _freeEvent;               // Return p to free list
	if (_freeEvent)                                 // Special case, if free list was empty
		_freeEvent->prevEvent = curEvent;
	_freeEvent = curEvent;
}

void Scheduler_v2d::delEventType(action_t actTypeDel) {
	// Note: actions are not deleted here, simply turned into NOPs!
	event_t *wrkEvent = _headEvent;                 // The earliest event
	event_t *saveEvent;

	while (wrkEvent) {                              // While events found in list
		saveEvent = wrkEvent->nextEvent;
		if (wrkEvent->action->a20.actType == actTypeDel)
			delQueue(wrkEvent);
		wrkEvent = saveEvent;
	}
}

void Scheduler_v2d::promptAction(act *action) {
	Utils::Box(BOX_PROMPT, "%s", _vm->_file->fetchString(action->a3.promptIndex));
	warning("STUB: doAction(act3), expecting answer %s", _vm->_file->fetchString(action->a3.responsePtr[0]));

	// TODO: The answer of the player is not handled currently! Once it'll be read in the messageBox, uncomment this block
#if 0
	char *response = Utils::Box(BOX_PROMPT, "%s", _vm->_file->fetchString(action->a3.promptIndex));

	bool  found = false;
	char *tmpStr;                                   // General purpose string ptr

	for (dx = 0; !found && (action->a3.responsePtr[dx] != -1); dx++) {
		tmpStr = _vm->_file->fetchString(action->a3.responsePtr[dx]);
		if (strstr(Utils::strlwr(response) , tmpStr))
			found = true;
	}

	if (found)
		insertActionList(action->a3.actPassIndex);
	else
		insertActionList(action->a3.actFailIndex);
#endif

	// HACK: As the answer is not read, currently it's always considered correct
	insertActionList(action->a3.actPassIndex);
}

/**
* Decode a string
*/
void Scheduler_v2d::decodeString(char *line) {
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	static const char *cypher = getCypher();

	for (uint16 i = 0; i < strlen(line); i++)
		line[i] -= cypher[i % strlen(cypher)];
	debugC(1, kDebugSchedule, "result : %s", line);
}
} // End of namespace Hugo
