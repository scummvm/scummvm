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

#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/util.h"

namespace Hugo {

Scheduler::Scheduler(HugoEngine *vm) : _vm(vm) {
}

Scheduler::~Scheduler() {
}

// Initialise the timer event queue
void Scheduler::initEventQueue() {
	debugC(1, kDebugSchedule, "initEventQueue");

	// Chain next_p from first to last
	for (int i = kMaxEvents; --i;)
		_events[i - 1].nextEvent = &_events[i];
	_events[kMaxEvents - 1].nextEvent = 0;

	// Chain prev_p from last to first
	for (int i = 1; i < kMaxEvents; i++)
		_events[i].prevEvent = &_events[i - 1];
	_events[0].prevEvent = 0;

	_headEvent = _tailEvent = 0;                    // Event list is empty
	_freeEvent = _events;                           // Free list is full
}

// Return a ptr to an event structure from the free list
event_t *Scheduler::getQueue() {
	debugC(4, kDebugSchedule, "getQueue");

	if (!_freeEvent)                                // Error: no more events available
		Utils::Error(EVNT_ERR, "%s", "getQueue");
	event_t *resEvent = _freeEvent;
	_freeEvent = _freeEvent->nextEvent;
	resEvent->nextEvent = 0;
	return resEvent;
}

// Call Insert_action for each action in the list supplied
void Scheduler::insertActionList(uint16 actIndex) {
	debugC(1, kDebugSchedule, "insertActionList(%d)", actIndex);

	if (_vm->_actListArr[actIndex]) {
		for (int i = 0; _vm->_actListArr[actIndex][i].a0.actType != ANULL; i++)
			insertAction(&_vm->_actListArr[actIndex][i]);
	}
}

// Decode a string
void Scheduler::decodeString(char *line) {
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	static const char *cypher = getCypher();

	for (uint16 i = 0; i < strlen(line); i++)
		line[i] -= cypher[i % strlen(cypher)];
	debugC(1, kDebugSchedule, "result : %s", line);
}

// Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
uint32 Scheduler::getWinTicks() {
	debugC(3, kDebugSchedule, "getTicks");

	return _vm->getGameStatus().tick;
}

// Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
// If update FALSE, simply return last known time
// Note that this is real time unless a processing cycle takes longer than
// a real tick, in which case the system tick is simply incremented
uint32 Scheduler::getDosTicks(bool updateFl) {
	debugC(5, kDebugSchedule, "getTicks");

	static  uint32 tick = 0;                        // Current system time in ticks
	static  uint32 t_old = 0;                       // The previous wall time in ticks

	uint32 t_now;                                   // Current wall time in ticks

	if (!updateFl)
		return(tick);

	if (t_old == 0) 
		t_old = (uint32) floor((double) (g_system->getMillis() * TPS / 1000));
	/* Calculate current wall time in ticks */
	t_now = g_system->getMillis() * TPS / 1000	;

	if ((t_now - t_old) > 0) {
		t_old = t_now;
		tick++;
	}
	return(tick);
}

// Add indecated bonus to score if not added already
void Scheduler::processBonus(int bonusIndex) {
	debugC(1, kDebugSchedule, "processBonus(%d)", bonusIndex);

	if (!_vm->_points[bonusIndex].scoredFl) {
		_vm->adjustScore(_vm->_points[bonusIndex].score);
		_vm->_points[bonusIndex].scoredFl = true;
	}
}

// Transition to a new screen as follows:
//	1. Clear out all non-global events from event list.
//	2. Set the new screen (in the hero object and any carried objects)
//	3. Read in the screen files for the new screen
//	4. Schedule action list for new screen
//	5. Initialise prompt line and status line
void Scheduler::newScreen(int screenIndex) {
	debugC(1, kDebugSchedule, "newScreen(%d)", screenIndex);

	// Make sure the background file exists!
	if (!_vm->isPacked()) {
		char line[32];
		if (!_vm->_file->fileExists(strcat(strncat(strcpy(line, _vm->_picDir), _vm->_screenNames[screenIndex], NAME_LEN), BKGEXT)) &&
		        !_vm->_file->fileExists(strcat(strcpy(line, _vm->_screenNames[screenIndex]), ".ART"))) {
			Utils::Box(BOX_ANY, "%s", _vm->_textSchedule[kSsNoBackground]);
			return;
		}
	}

	// 1. Clear out all local events
	event_t *curEvent = _headEvent;                 // The earliest event
	event_t *wrkEvent;                              // Event ptr
	while (curEvent) {                              // While mature events found
		wrkEvent = curEvent->nextEvent;             // Save p (becomes undefined after Del)
		if (curEvent->localActionFl)
			delQueue(curEvent);                     // Return event to free list
		curEvent = wrkEvent;
	}

	// 2. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 3. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 4. Schedule action list for this screen
	_vm->screenActions(screenIndex);

	// 5. Initialise prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

// Transition to a new screen as follows:
//	1. Set the new screen (in the hero object and any carried objects)
//	2. Read in the screen files for the new screen
//	3. Initialise prompt line and status line
void Scheduler::restoreScreen(int screenIndex) {
	debugC(1, kDebugSchedule, "restoreScreen(%d)", screenIndex);

	// 1. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 2. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 3. Initialise prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

// Wait (if necessary) for next synchronizing tick
// Slow machines won't make it by the end of tick, so will just plod on
// at their own speed, not waiting here, but free running.
// Note: DOS Versions only
void Scheduler::waitForRefresh(void) {
	debugC(1, kDebugSchedule, "waitForRefresh()");

	static uint32 timeout = 0;
	uint32 t;

	if (timeout == 0)
		timeout = getDosTicks(true);

	while ((t = getDosTicks(true)) < timeout)
		;
	timeout = ++t;
}

} // End of namespace Hugo
