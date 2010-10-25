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

void Scheduler::insertActionList(uint16 actIndex) {
// Call Insert_action for each action in the list supplied
	debugC(1, kDebugSchedule, "insertActionList(%d)", actIndex);

	if (_vm->_actListArr[actIndex]) {
		for (int i = 0; _vm->_actListArr[actIndex][i].a0.actType != ANULL; i++)
			insertAction(&_vm->_actListArr[actIndex][i]);
	}
}

void Scheduler::decodeString(char *line) {
// Decode a string
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	static const char *cypher = getCypher();

	for (uint16 i = 0; i < strlen(line); i++)
		line[i] -= cypher[i % strlen(cypher)];
	debugC(1, kDebugSchedule, "result : %s", line);
}

// This is the scheduler which runs every tick.  It examines the event queue
// for any events whose time has come.  It dequeues these events and performs
// the action associated with the event, returning it to the free queue
void Scheduler::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	status_t &gameStatus = _vm->getGameStatus();
	event_t *curEvent = _headEvent;                 // The earliest event

	while (curEvent && curEvent->time <= gameStatus.tick) // While mature events found
		curEvent = doAction(curEvent);              // Perform the action (returns next_p)
	gameStatus.tick++;                              // Accessed elsewhere via getTicks()
}

uint32 Scheduler::getTicks() {
// Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
	debugC(3, kDebugSchedule, "getTicks");

	return _vm->getGameStatus().tick;
}

void Scheduler::processBonus(int bonusIndex) {
// Add indecated bonus to score if not added already
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

// Write the event queue to the file with handle f
// Note that we convert all the event structure ptrs to indexes
// using -1 for NULL.  We can't convert the action ptrs to indexes
// so we save address of first dummy action ptr to compare on restore.
void Scheduler::saveEvents(Common::WriteStream *f) {
	debugC(1, kDebugSchedule, "saveEvents()");

	uint32 curTime = getTicks();
	event_t  saveEventArr[kMaxEvents];              // Convert event ptrs to indexes

	// Convert event ptrs to indexes
	for (int16 i = 0; i < kMaxEvents; i++) {
		event_t *wrkEvent = &_events[i];
		saveEventArr[i] = *wrkEvent;
		saveEventArr[i].prevEvent = (wrkEvent->prevEvent == 0) ? (event_t *) - 1 : (event_t *)(wrkEvent->prevEvent - _events);
		saveEventArr[i].nextEvent = (wrkEvent->nextEvent == 0) ? (event_t *) - 1 : (event_t *)(wrkEvent->nextEvent - _events);
	}

	int16 freeIndex = (_freeEvent == 0) ? -1 : _freeEvent - _events;
	int16 headIndex = (_headEvent == 0) ? -1 : _headEvent - _events;
	int16 tailIndex = (_tailEvent == 0) ? -1 : _tailEvent - _events;

	f->write(&curTime,   sizeof(curTime));
	f->write(&freeIndex, sizeof(freeIndex));
	f->write(&headIndex, sizeof(headIndex));
	f->write(&tailIndex, sizeof(tailIndex));
	f->write(saveEventArr, sizeof(saveEventArr));
}

// Restore the event list from file with handle f
void Scheduler::restoreEvents(Common::SeekableReadStream *f) {
	debugC(1, kDebugSchedule, "restoreEvents");

	uint32   saveTime;
	int16    freeIndex;                             // Free list index
	int16    headIndex;                             // Head of list index
	int16    tailIndex;                             // Tail of list index
	event_t  savedEvents[kMaxEvents];               // Convert event ptrs to indexes

	f->read(&saveTime,  sizeof(saveTime));          // time of save
	f->read(&freeIndex, sizeof(freeIndex));
	f->read(&headIndex, sizeof(headIndex));
	f->read(&tailIndex, sizeof(tailIndex));
	f->read(savedEvents, sizeof(savedEvents));

	event_t *wrkEvent;
	// Restore events indexes to pointers
	for (int i = 0; i < kMaxEvents; i++) {
		wrkEvent = &savedEvents[i];
		_events[i] = *wrkEvent;
		_events[i].prevEvent = (wrkEvent->prevEvent == (event_t *) - 1) ? (event_t *)0 : &_events[(size_t)wrkEvent->prevEvent ];
		_events[i].nextEvent = (wrkEvent->nextEvent == (event_t *) - 1) ? (event_t *)0 : &_events[(size_t)wrkEvent->nextEvent ];
	}
	_freeEvent = (freeIndex == -1) ? 0 : &_events[freeIndex];
	_headEvent = (headIndex == -1) ? 0 : &_events[headIndex];
	_tailEvent = (tailIndex == -1) ? 0 : &_events[tailIndex];

	// Adjust times to fit our time
	uint32 curTime = getTicks();
	wrkEvent = _headEvent;                              // The earliest event
	while (wrkEvent) {                              // While mature events found
		wrkEvent->time = wrkEvent->time - saveTime + curTime;
		wrkEvent = wrkEvent->nextEvent;
	}
}

void Scheduler::restoreScreen(int screenIndex) {
// Transition to a new screen as follows:
//	1. Set the new screen (in the hero object and any carried objects)
//	2. Read in the screen files for the new screen
//	3. Initialise prompt line and status line

	debugC(1, kDebugSchedule, "restoreScreen(%d)", screenIndex);

	// 1. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 2. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 3. Initialise prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

} // End of namespace Hugo
