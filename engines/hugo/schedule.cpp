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

Scheduler::Scheduler(HugoEngine *vm) : _vm(vm), _actListArr(0) {
}

Scheduler::~Scheduler() {
}

/**
* Initialise the timer event queue
*/
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

/**
* Return a ptr to an event structure from the free list
*/
event_t *Scheduler::getQueue() {
	debugC(4, kDebugSchedule, "getQueue");

	if (!_freeEvent)                                // Error: no more events available
		error("An error has occurred: %s", "getQueue");
	event_t *resEvent = _freeEvent;
	_freeEvent = _freeEvent->nextEvent;
	resEvent->nextEvent = 0;
	return resEvent;
}

/**
* Call Insert_action for each action in the list supplied
*/
void Scheduler::insertActionList(uint16 actIndex) {
	debugC(1, kDebugSchedule, "insertActionList(%d)", actIndex);

	if (_actListArr[actIndex]) {
		for (int i = 0; _actListArr[actIndex][i].a0.actType != ANULL; i++)
			insertAction(&_actListArr[actIndex][i]);
	}
}

/**
* Decode a string
*/
void Scheduler::decodeString(char *line) {
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	static const char *cypher = getCypher();

	for (uint16 i = 0; i < strlen(line); i++)
		line[i] -= cypher[i % strlen(cypher)];
	debugC(1, kDebugSchedule, "result : %s", line);
}

/**
* Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
*/
uint32 Scheduler::getWinTicks() {
	debugC(3, kDebugSchedule, "getWinTicks");

	return _vm->getGameStatus().tick;
}

/**
* Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
* If update FALSE, simply return last known time
* Note that this is real time unless a processing cycle takes longer than
* a real tick, in which case the system tick is simply incremented
*/
uint32 Scheduler::getDosTicks(bool updateFl) {
	debugC(5, kDebugSchedule, "getDosTicks(%s)", (updateFl) ? "TRUE" : "FALSE");

	static  uint32 tick = 0;                        // Current system time in ticks
	static  uint32 t_old = 0;                       // The previous wall time in ticks

	uint32 t_now;                                   // Current wall time in ticks

	if (!updateFl)
		return(tick);

	if (t_old == 0) 
		t_old = (uint32) floor((double) (g_system->getMillis() * _vm->getTPS() / 1000));
	/* Calculate current wall time in ticks */
	t_now = g_system->getMillis() * _vm->getTPS() / 1000	;

	if ((t_now - t_old) > 0) {
		t_old = t_now;
		tick++;
	}
	return(tick);
}

/**
* Add indecated bonus to score if not added already
*/
void Scheduler::processBonus(int bonusIndex) {
	debugC(1, kDebugSchedule, "processBonus(%d)", bonusIndex);

	if (!_vm->_points[bonusIndex].scoredFl) {
		_vm->adjustScore(_vm->_points[bonusIndex].score);
		_vm->_points[bonusIndex].scoredFl = true;
	}
}

/**
* Transition to a new screen as follows:
* 1. Clear out all non-global events from event list.
* 2. Set the new screen (in the hero object and any carried objects)
* 3. Read in the screen files for the new screen
* 4. Schedule action list for new screen
* 5. Initialise prompt line and status line
*/
void Scheduler::newScreen(int screenIndex) {
	debugC(1, kDebugSchedule, "newScreen(%d)", screenIndex);

	// Make sure the background file exists!
	if (!_vm->isPacked()) {
		char line[32];
		if (!_vm->_file->fileExists(strcat(strncat(strcpy(line, _vm->_picDir), _vm->_screenNames[screenIndex], NAME_LEN), BKGEXT)) &&
		    !_vm->_file->fileExists(strcat(strcpy(line, _vm->_screenNames[screenIndex]), ".ART"))) {
				error("Unable to find background file for %s", _vm->_screenNames[screenIndex]);
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

/**
* Transition to a new screen as follows:
* 1. Set the new screen (in the hero object and any carried objects)
* 2. Read in the screen files for the new screen
* 3. Initialise prompt line and status line
*/
void Scheduler::restoreScreen(int screenIndex) {
	debugC(1, kDebugSchedule, "restoreScreen(%d)", screenIndex);

	// 1. Set the new screen in the hero object and any being carried
	_vm->setNewScreen(screenIndex);

	// 2. Read in new screen files
	_vm->readScreenFiles(screenIndex);

	// 3. Initialise prompt line and status line
	_vm->_screen->initNewScreenDisplay();
}

/**
* Wait (if necessary) for next synchronizing tick
* Slow machines won't make it by the end of tick, so will just plod on
* at their own speed, not waiting here, but free running.
* Note: DOS Versions only
*/
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

/**
* Read kALnewscr used by maze (Hugo 2)
*/
void Scheduler::loadAlNewscrIndex(Common::File &in) {
	debugC(6, kDebugSchedule, "loadAlNewscrIndex(&in)");

	int numElem;
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_alNewscrIndex = numElem;
	}
}

/**
* Load actListArr from Hugo.dat
*/
void Scheduler::loadActListArr(Common::File &in) {
	debugC(6, kDebugSchedule, "loadActListArr(&in)");

	int numElem, numSubElem, numSubAct;
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_actListArrSize = numElem;
			_actListArr = (act **)malloc(sizeof(act *) * _actListArrSize);
			for (int i = 0; i < _actListArrSize; i++) {
				numSubElem = in.readUint16BE();
				_actListArr[i] = (act *) malloc(sizeof(act) * (numSubElem + 1));
				for (int j = 0; j < numSubElem; j++) {
					_actListArr[i][j].a0.actType = (action_t) in.readByte();
					switch (_actListArr[i][j].a0.actType) {
					case ANULL:              // -1
						break;
					case ASCHEDULE:          // 0
						_actListArr[i][j].a0.timer = in.readSint16BE();
						_actListArr[i][j].a0.actIndex = in.readUint16BE();
						break;
					case START_OBJ:          // 1
						_actListArr[i][j].a1.timer = in.readSint16BE();
						_actListArr[i][j].a1.objNumb = in.readSint16BE();
						_actListArr[i][j].a1.cycleNumb = in.readSint16BE();
						_actListArr[i][j].a1.cycle = (cycle_t) in.readByte();
						break;
					case INIT_OBJXY:         // 2
						_actListArr[i][j].a2.timer = in.readSint16BE();
						_actListArr[i][j].a2.objNumb = in.readSint16BE();
						_actListArr[i][j].a2.x = in.readSint16BE();
						_actListArr[i][j].a2.y = in.readSint16BE();
						break;
					case PROMPT:             // 3
						_actListArr[i][j].a3.timer = in.readSint16BE();
						_actListArr[i][j].a3.promptIndex = in.readSint16BE();
						numSubAct = in.readUint16BE();
						_actListArr[i][j].a3.responsePtr = (int *) malloc(sizeof(int) * numSubAct);
						for (int k = 0; k < numSubAct; k++)
							_actListArr[i][j].a3.responsePtr[k] = in.readSint16BE();
						_actListArr[i][j].a3.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a3.actFailIndex = in.readUint16BE();
						_actListArr[i][j].a3.encodedFl = (in.readByte() == 1) ? true : false;
						break;
					case BKGD_COLOR:         // 4
						_actListArr[i][j].a4.timer = in.readSint16BE();
						_actListArr[i][j].a4.newBackgroundColor = in.readUint32BE();
						break;
					case INIT_OBJVXY:        // 5
						_actListArr[i][j].a5.timer = in.readSint16BE();
						_actListArr[i][j].a5.objNumb = in.readSint16BE();
						_actListArr[i][j].a5.vx = in.readSint16BE();
						_actListArr[i][j].a5.vy = in.readSint16BE();
						break;
					case INIT_CARRY:         // 6
						_actListArr[i][j].a6.timer = in.readSint16BE();
						_actListArr[i][j].a6.objNumb = in.readSint16BE();
						_actListArr[i][j].a6.carriedFl = (in.readByte() == 1) ? true : false;
						break;
					case INIT_HF_COORD:      // 7
						_actListArr[i][j].a7.timer = in.readSint16BE();
						_actListArr[i][j].a7.objNumb = in.readSint16BE();
						break;
					case NEW_SCREEN:         // 8
						_actListArr[i][j].a8.timer = in.readSint16BE();
						_actListArr[i][j].a8.screenIndex = in.readSint16BE();
						break;
					case INIT_OBJSTATE:      // 9
						_actListArr[i][j].a9.timer = in.readSint16BE();
						_actListArr[i][j].a9.objNumb = in.readSint16BE();
						_actListArr[i][j].a9.newState = in.readByte();
						break;
					case INIT_PATH:          // 10
						_actListArr[i][j].a10.timer = in.readSint16BE();
						_actListArr[i][j].a10.objNumb = in.readSint16BE();
						_actListArr[i][j].a10.newPathType = in.readSint16BE();
						_actListArr[i][j].a10.vxPath = in.readByte();
						_actListArr[i][j].a10.vyPath = in.readByte();
						break;
					case COND_R:             // 11
						_actListArr[i][j].a11.timer = in.readSint16BE();
						_actListArr[i][j].a11.objNumb = in.readSint16BE();
						_actListArr[i][j].a11.stateReq = in.readByte();
						_actListArr[i][j].a11.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a11.actFailIndex = in.readUint16BE();
						break;
					case TEXT:               // 12
						_actListArr[i][j].a12.timer = in.readSint16BE();
						_actListArr[i][j].a12.stringIndex = in.readSint16BE();
						break;
					case SWAP_IMAGES:        // 13
						_actListArr[i][j].a13.timer = in.readSint16BE();
						_actListArr[i][j].a13.obj1 = in.readSint16BE();
						_actListArr[i][j].a13.obj2 = in.readSint16BE();
						break;
					case COND_SCR:           // 14
						_actListArr[i][j].a14.timer = in.readSint16BE();
						_actListArr[i][j].a14.objNumb = in.readSint16BE();
						_actListArr[i][j].a14.screenReq = in.readSint16BE();
						_actListArr[i][j].a14.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a14.actFailIndex = in.readUint16BE();
						break;
					case AUTOPILOT:          // 15
						_actListArr[i][j].a15.timer = in.readSint16BE();
						_actListArr[i][j].a15.obj1 = in.readSint16BE();
						_actListArr[i][j].a15.obj2 = in.readSint16BE();
						_actListArr[i][j].a15.dx = in.readByte();
						_actListArr[i][j].a15.dy = in.readByte();
						break;
					case INIT_OBJ_SEQ:       // 16
						_actListArr[i][j].a16.timer = in.readSint16BE();
						_actListArr[i][j].a16.objNumb = in.readSint16BE();
						_actListArr[i][j].a16.seqIndex = in.readSint16BE();
						break;
					case SET_STATE_BITS:     // 17
						_actListArr[i][j].a17.timer = in.readSint16BE();
						_actListArr[i][j].a17.objNumb = in.readSint16BE();
						_actListArr[i][j].a17.stateMask = in.readSint16BE();
						break;
					case CLEAR_STATE_BITS:   // 18
						_actListArr[i][j].a18.timer = in.readSint16BE();
						_actListArr[i][j].a18.objNumb = in.readSint16BE();
						_actListArr[i][j].a18.stateMask = in.readSint16BE();
						break;
					case TEST_STATE_BITS:    // 19
						_actListArr[i][j].a19.timer = in.readSint16BE();
						_actListArr[i][j].a19.objNumb = in.readSint16BE();
						_actListArr[i][j].a19.stateMask = in.readSint16BE();
						_actListArr[i][j].a19.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a19.actFailIndex = in.readUint16BE();
						break;
					case DEL_EVENTS:         // 20
						_actListArr[i][j].a20.timer = in.readSint16BE();
						_actListArr[i][j].a20.actTypeDel = (action_t) in.readByte();
						break;
					case GAMEOVER:           // 21
						_actListArr[i][j].a21.timer = in.readSint16BE();
						break;
					case INIT_HH_COORD:      // 22
						_actListArr[i][j].a22.timer = in.readSint16BE();
						_actListArr[i][j].a22.objNumb = in.readSint16BE();
						break;
					case EXIT:               // 23
						_actListArr[i][j].a23.timer = in.readSint16BE();
						break;
					case BONUS:              // 24
						_actListArr[i][j].a24.timer = in.readSint16BE();
						_actListArr[i][j].a24.pointIndex = in.readSint16BE();
						break;
					case COND_BOX:           // 25
						_actListArr[i][j].a25.timer = in.readSint16BE();
						_actListArr[i][j].a25.objNumb = in.readSint16BE();
						_actListArr[i][j].a25.x1 = in.readSint16BE();
						_actListArr[i][j].a25.y1 = in.readSint16BE();
						_actListArr[i][j].a25.x2 = in.readSint16BE();
						_actListArr[i][j].a25.y2 = in.readSint16BE();
						_actListArr[i][j].a25.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a25.actFailIndex = in.readUint16BE();
						break;
					case SOUND:              // 26
						_actListArr[i][j].a26.timer = in.readSint16BE();
						_actListArr[i][j].a26.soundIndex = in.readSint16BE();
						break;
					case ADD_SCORE:          // 27
						_actListArr[i][j].a27.timer = in.readSint16BE();
						_actListArr[i][j].a27.objNumb = in.readSint16BE();
						break;
					case SUB_SCORE:          // 28
						_actListArr[i][j].a28.timer = in.readSint16BE();
						_actListArr[i][j].a28.objNumb = in.readSint16BE();
						break;
					case COND_CARRY:         // 29
						_actListArr[i][j].a29.timer = in.readSint16BE();
						_actListArr[i][j].a29.objNumb = in.readSint16BE();
						_actListArr[i][j].a29.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a29.actFailIndex = in.readUint16BE();
						break;
					case INIT_MAZE:          // 30
						_actListArr[i][j].a30.timer = in.readSint16BE();
						_actListArr[i][j].a30.mazeSize = in.readByte();
						_actListArr[i][j].a30.x1 = in.readSint16BE();
						_actListArr[i][j].a30.y1 = in.readSint16BE();
						_actListArr[i][j].a30.x2 = in.readSint16BE();
						_actListArr[i][j].a30.y2 = in.readSint16BE();
						_actListArr[i][j].a30.x3 = in.readSint16BE();
						_actListArr[i][j].a30.x4 = in.readSint16BE();
						_actListArr[i][j].a30.firstScreenIndex = in.readByte();
						break;
					case EXIT_MAZE:          // 31
						_actListArr[i][j].a31.timer = in.readSint16BE();
						break;
					case INIT_PRIORITY:      // 32
						_actListArr[i][j].a32.timer = in.readSint16BE();
						_actListArr[i][j].a32.objNumb = in.readSint16BE();
						_actListArr[i][j].a32.priority = in.readByte();
						break;
					case INIT_SCREEN:        // 33
						_actListArr[i][j].a33.timer = in.readSint16BE();
						_actListArr[i][j].a33.objNumb = in.readSint16BE();
						_actListArr[i][j].a33.screenIndex = in.readSint16BE();
						break;
					case AGSCHEDULE:         // 34
						_actListArr[i][j].a34.timer = in.readSint16BE();
						_actListArr[i][j].a34.actIndex = in.readUint16BE();
						break;
					case REMAPPAL:           // 35
						_actListArr[i][j].a35.timer = in.readSint16BE();
						_actListArr[i][j].a35.oldColorIndex = in.readSint16BE();
						_actListArr[i][j].a35.newColorIndex = in.readSint16BE();
						break;
					case COND_NOUN:          // 36
						_actListArr[i][j].a36.timer = in.readSint16BE();
						_actListArr[i][j].a36.nounIndex = in.readUint16BE();
						_actListArr[i][j].a36.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a36.actFailIndex = in.readUint16BE();
						break;
					case SCREEN_STATE:       // 37
						_actListArr[i][j].a37.timer = in.readSint16BE();
						_actListArr[i][j].a37.screenIndex = in.readSint16BE();
						_actListArr[i][j].a37.newState = in.readByte();
						break;
					case INIT_LIPS:          // 38
						_actListArr[i][j].a38.timer = in.readSint16BE();
						_actListArr[i][j].a38.lipsObjNumb = in.readSint16BE();
						_actListArr[i][j].a38.objNumb = in.readSint16BE();
						_actListArr[i][j].a38.dxLips = in.readByte();
						_actListArr[i][j].a38.dyLips = in.readByte();
						break;
					case INIT_STORY_MODE:    // 39
						_actListArr[i][j].a39.timer = in.readSint16BE();
						_actListArr[i][j].a39.storyModeFl = (in.readByte() == 1);
						break;
					case WARN:               // 40
						_actListArr[i][j].a40.timer = in.readSint16BE();
						_actListArr[i][j].a40.stringIndex = in.readSint16BE();
						break;
					case COND_BONUS:         // 41
						_actListArr[i][j].a41.timer = in.readSint16BE();
						_actListArr[i][j].a41.BonusIndex = in.readSint16BE();
						_actListArr[i][j].a41.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a41.actFailIndex = in.readUint16BE();
						break;
					case TEXT_TAKE:          // 42
						_actListArr[i][j].a42.timer = in.readSint16BE();
						_actListArr[i][j].a42.objNumb = in.readSint16BE();
						break;
					case YESNO:              // 43
						_actListArr[i][j].a43.timer = in.readSint16BE();
						_actListArr[i][j].a43.promptIndex = in.readSint16BE();
						_actListArr[i][j].a43.actYesIndex = in.readUint16BE();
						_actListArr[i][j].a43.actNoIndex = in.readUint16BE();
						break;
					case STOP_ROUTE:         // 44
						_actListArr[i][j].a44.timer = in.readSint16BE();
						break;
					case COND_ROUTE:         // 45
						_actListArr[i][j].a45.timer = in.readSint16BE();
						_actListArr[i][j].a45.routeIndex = in.readSint16BE();
						_actListArr[i][j].a45.actPassIndex = in.readUint16BE();
						_actListArr[i][j].a45.actFailIndex = in.readUint16BE();
						break;
					case INIT_JUMPEXIT:      // 46
						_actListArr[i][j].a46.timer = in.readSint16BE();
						_actListArr[i][j].a46.jumpExitFl = (in.readByte() == 1);
						break;
					case INIT_VIEW:          // 47
						_actListArr[i][j].a47.timer = in.readSint16BE();
						_actListArr[i][j].a47.objNumb = in.readSint16BE();
						_actListArr[i][j].a47.viewx = in.readSint16BE();
						_actListArr[i][j].a47.viewy = in.readSint16BE();
						_actListArr[i][j].a47.direction = in.readSint16BE();
						break;
					case INIT_OBJ_FRAME:     // 48
						_actListArr[i][j].a48.timer = in.readSint16BE();
						_actListArr[i][j].a48.objNumb = in.readSint16BE();
						_actListArr[i][j].a48.seqIndex = in.readSint16BE();
						_actListArr[i][j].a48.frameIndex = in.readSint16BE();
						break;
					case OLD_SONG:           //49
						_actListArr[i][j].a49.timer = in.readSint16BE();
						_actListArr[i][j].a49.songIndex = in.readUint16BE();
						break;
					default:
						error("Engine - Unknown action type encountered: %d", _actListArr[i][j].a0.actType);
					}
				}
				_actListArr[i][numSubElem].a0.actType = ANULL;
			}
		} else {
			for (int i = 0; i < numElem; i++) {
				numSubElem = in.readUint16BE();
				for (int j = 0; j < numSubElem; j++) {
					numSubAct = in.readByte();
					switch (numSubAct) {
					case ANULL:              // -1
						break;
					case ASCHEDULE:          // 0
						in.readSint16BE();
						in.readUint16BE();
						break;
					case START_OBJ:          // 1
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_OBJXY:         // 2
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case PROMPT:             // 3
						in.readSint16BE();
						in.readSint16BE();
						numSubAct = in.readUint16BE();
						for (int k = 0; k < numSubAct; k++)
							in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						in.readByte();
						break;
					case BKGD_COLOR:         // 4
						in.readSint16BE();
						in.readUint32BE();
						break;
					case INIT_OBJVXY:        // 5
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_CARRY:         // 6
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_HF_COORD:      // 7
						in.readSint16BE();
						in.readSint16BE();
						break;
					case NEW_SCREEN:         // 8
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_OBJSTATE:      // 9
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_PATH:          // 10
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case COND_R:             // 11
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case TEXT:               // 12
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SWAP_IMAGES:        // 13
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_SCR:           // 14
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case AUTOPILOT:          // 15
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case INIT_OBJ_SEQ:       // 16
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SET_STATE_BITS:     // 17
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case CLEAR_STATE_BITS:   // 18
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case TEST_STATE_BITS:    // 19
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case DEL_EVENTS:         // 20
						in.readSint16BE();
						in.readByte();
						break;
					case GAMEOVER:           // 21
						in.readSint16BE();
						break;
					case INIT_HH_COORD:      // 22
						in.readSint16BE();
						in.readSint16BE();
						break;
					case EXIT:               // 23
						in.readSint16BE();
						break;
					case BONUS:              // 24
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_BOX:           // 25
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case SOUND:              // 26
						in.readSint16BE();
						in.readSint16BE();
						break;
					case ADD_SCORE:          // 27
						in.readSint16BE();
						in.readSint16BE();
						break;
					case SUB_SCORE:          // 28
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_CARRY:         // 29
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case INIT_MAZE:          // 30
						in.readSint16BE();
						in.readByte();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case EXIT_MAZE:          // 31
						in.readSint16BE();
						break;
					case INIT_PRIORITY:      // 32
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_SCREEN:        // 33
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case AGSCHEDULE:         // 34
						in.readSint16BE();
						in.readUint16BE();
						break;
					case REMAPPAL:           // 35
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_NOUN:          // 36
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case SCREEN_STATE:       // 37
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_LIPS:          // 38
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readByte();
						in.readByte();
						break;
					case INIT_STORY_MODE:    // 39
						in.readSint16BE();
						in.readByte();
						break;
					case WARN:               // 40
						in.readSint16BE();
						in.readSint16BE();
						break;
					case COND_BONUS:         // 41
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case TEXT_TAKE:          // 42
						in.readSint16BE();
						in.readSint16BE();
						break;
					case YESNO:              // 43
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case STOP_ROUTE:         // 44
						in.readSint16BE();
						break;
					case COND_ROUTE:         // 45
						in.readSint16BE();
						in.readSint16BE();
						in.readUint16BE();
						in.readUint16BE();
						break;
					case INIT_JUMPEXIT:      // 46
						in.readSint16BE();
						in.readByte();
						break;
					case INIT_VIEW:          // 47
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case INIT_OBJ_FRAME:     // 48
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						in.readSint16BE();
						break;
					case OLD_SONG:           //49
						in.readSint16BE();
						in.readUint16BE();
						break;
					default:
						error("Engine - Unknown action type encountered %d - variante %d pos %d.%d", numSubAct, varnt, i, j);
					}
				}
			}
		}
	}
}

void Scheduler::freeActListArr() {
	debugC(6, kDebugSchedule, "freeActListArr()");

	if (_actListArr) {
		for (int i = 0; i < _actListArrSize; i++) {
			for (int j = 0; _actListArr[i][j].a0.actType != ANULL; j++) {
				if (_actListArr[i][j].a0.actType == PROMPT)
					free(_actListArr[i][j].a3.responsePtr);
			}
			free(_actListArr[i]);
		}
		free(_actListArr);
	}
}

/**
* Maze mode is enabled.  Check to see whether hero has crossed the maze
* bounding box, if so, go to the next room
*/
void Scheduler::processMaze(int x1, int x2, int y1, int y2) {
	debugC(1, kDebugSchedule, "processMaze");

	status_t &gameStatus = _vm->getGameStatus();

	if (x1 < _maze.x1) {
		// Exit west
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_vm->_screen_p - 1;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x2 - SHIFT - (x2 - x1);
		_actListArr[_alNewscrIndex][0].a2.y = _vm->_hero->y;
		gameStatus.routeIndex = -1;
		insertActionList(_alNewscrIndex);
	} else if (x2 > _maze.x2) {
		// Exit east
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_vm->_screen_p + 1;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x1 + SHIFT;
		_actListArr[_alNewscrIndex][0].a2.y = _vm->_hero->y;
		gameStatus.routeIndex = -1;
		insertActionList(_alNewscrIndex);
	} else if (y1 < _maze.y1 - SHIFT) {
		// Exit north
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_vm->_screen_p - _maze.size;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x3;
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y2 - SHIFT - (y2 - y1);
		gameStatus.routeIndex = -1;
		insertActionList(_alNewscrIndex);
	} else if (y2 > _maze.y2 - SHIFT / 2) {
		// Exit south
		_actListArr[_alNewscrIndex][3].a8.screenIndex = *_vm->_screen_p + _maze.size;
		_actListArr[_alNewscrIndex][0].a2.x = _maze.x4;
		_actListArr[_alNewscrIndex][0].a2.y = _maze.y1 + SHIFT;
		gameStatus.routeIndex = -1;
		insertActionList(_alNewscrIndex);
	}
}

/**
* Write the event queue to the file with handle f
* Note that we convert all the event structure ptrs to indexes
* using -1 for NULL.  We can't convert the action ptrs to indexes
* so we save address of first dummy action ptr to compare on restore.
*/
void Scheduler::saveEvents(Common::WriteStream *f) {
	debugC(1, kDebugSchedule, "saveEvents()");

	f->writeUint32BE(getTicks());

	int16 freeIndex = (_freeEvent == 0) ? -1 : _freeEvent - _events;
	int16 headIndex = (_headEvent == 0) ? -1 : _headEvent - _events;
	int16 tailIndex = (_tailEvent == 0) ? -1 : _tailEvent - _events;

	f->writeSint16BE(freeIndex);
	f->writeSint16BE(headIndex);
	f->writeSint16BE(tailIndex);

	// Convert event ptrs to indexes
	event_t  saveEventArr[kMaxEvents];              // Convert event ptrs to indexes
	for (int16 i = 0; i < kMaxEvents; i++) {
		event_t *wrkEvent = &_events[i];
		saveEventArr[i] = *wrkEvent;
		saveEventArr[i].prevEvent = (wrkEvent->prevEvent == 0) ? (event_t *) - 1 : (event_t *)(wrkEvent->prevEvent - _events);
		saveEventArr[i].nextEvent = (wrkEvent->nextEvent == 0) ? (event_t *) - 1 : (event_t *)(wrkEvent->nextEvent - _events);
	}

	f->write(saveEventArr, sizeof(saveEventArr));
	warning("TODO: serialize saveEventArr");
}

/**
* Restore the event list from file with handle f
*/
void Scheduler::restoreEvents(Common::SeekableReadStream *f) {
	debugC(1, kDebugSchedule, "restoreEvents");

	event_t  savedEvents[kMaxEvents];               // Convert event ptrs to indexes

	uint32 saveTime = f->readUint32BE();            // time of save
	int16 freeIndex = f->readSint16BE();            // Free list index
	int16 headIndex = f->readSint16BE();            // Head of list index
	int16 tailIndex = f->readSint16BE();            // Tail of list index
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

/**
* Insert the action pointed to by p into the timer event queue
* The queue goes from head (earliest) to tail (latest) timewise
*/
void Scheduler::insertAction(act *action) {
	debugC(1, kDebugSchedule, "insertAction() - Action type A%d", action->a0.actType);

	// First, get and initialise the event structure
	event_t *curEvent = getQueue();
	curEvent->action = action;
	switch (action->a0.actType) {                   // Assign whether local or global
	case AGSCHEDULE:
		curEvent->localActionFl = false;            // Lasts over a new screen
		break;
	default:
		curEvent->localActionFl = true;             // Rest are for current screen only
		break;
	}

	curEvent->time = action->a0.timer + getTicks(); // Convert rel to abs time

	// Now find the place to insert the event
	if (!_tailEvent) {                              // Empty queue
		_tailEvent = _headEvent = curEvent;
		curEvent->nextEvent = curEvent->prevEvent = 0;
	} else {
		event_t *wrkEvent = _tailEvent;             // Search from latest time back
		bool found = false;

		while (wrkEvent && !found) {
			if (wrkEvent->time <= curEvent->time) { // Found if new event later
				found = true;
				if (wrkEvent == _tailEvent)         // New latest in list
					_tailEvent = curEvent;
				else
					wrkEvent->nextEvent->prevEvent = curEvent;
				curEvent->nextEvent = wrkEvent->nextEvent;
				wrkEvent->nextEvent = curEvent;
				curEvent->prevEvent = wrkEvent;
			}
			wrkEvent = wrkEvent->prevEvent;
		}

		if (!found) {                               // Must be earliest in list
			_headEvent->prevEvent = curEvent;       // So insert as new head
			curEvent->nextEvent = _headEvent;
			curEvent->prevEvent = 0;
			_headEvent = curEvent;
		}
	}
}

} // End of namespace Hugo
