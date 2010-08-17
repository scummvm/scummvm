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
#include "common/stream.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/global.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/util.h"
#include "hugo/sound.h"

namespace Hugo {

#define SIGN(X)       (X < 0 ? -1 : 1)

Scheduler::Scheduler(HugoEngine &vm) : _vm(vm) {
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
	event_t *resEvent;

	if (!_freeEvent)                                // Error: no more events available
		Utils::Error(EVNT_ERR, "getQueue");
	resEvent = _freeEvent;
	_freeEvent = _freeEvent->nextEvent;
	resEvent->nextEvent = 0;
	return resEvent;
}

// Delete an event structure (i.e. return it to the free list)
// Historical note:  Originally event p was assumed to be at head of queue
// (i.e. earliest) since all events were deleted in order when proceeding to
// a new screen.  To delete an event from the middle of the queue, the action
// was overwritten to be ANULL.  With the advent of GLOBAL events, Del_queue
// was modified to allow deletes anywhere in the list, and the DEL_EVENT
// action was modified to perform the actual delete.
void Scheduler::delQueue(event_t *curEvent) {
	debugC(4, kDebugSchedule, "delQueue");
	if (curEvent == _headEvent)                     // If p was the head ptr
		_headEvent = curEvent->nextEvent;           // then make new head_p
	else {                                          // Unlink p
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

// Insert the action pointed to by p into the timer event queue
// The queue goes from head (earliest) to tail (latest) timewise
void Scheduler::insertAction(act *action) {
	debugC(1, kDebugSchedule, "insertAction - Action type A%d", action->a0.actType);

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
		curEvent->nextEvent = curEvent->prevEvent = NULL;
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
			curEvent->prevEvent = NULL;
			_headEvent = curEvent;
		}
	}
}

void Scheduler::insertActionList(uint16 actIndex) {
// Call Insert_action for each action in the list supplied
	debugC(1, kDebugSchedule, "insertActionList(%d)", actIndex);

	if (_vm._actListArr[actIndex])
		for (int i = 0; _vm._actListArr[actIndex][i].a0.actType != ANULL; i++)
			insertAction(&_vm._actListArr[actIndex][i]);
}

void Scheduler::decodeString(char *line) {
// Decode a string
	debugC(1, kDebugSchedule, "decodeString(%s)", line);

	static char cypher[] = "Copyright 1992, Gray Design Associates";

	for (uint16 i = 0; i < strlen(line); i++)
		line[i] -= cypher[i % strlen(cypher)];
	debugC(1, kDebugSchedule, "result : %s", line);
}

event_t *Scheduler::doAction(event_t *curEvent) {
// This function performs the action in the event structure pointed to by p
// It dequeues the event and returns it to the free list.  It returns a ptr
// to the next action in the list, except special case of NEW_SCREEN
	event_t  *wrkEvent;                             // Save ev_p->next_p for return
	event_t  *saveEvent;                            // Used in DEL_EVENTS
	char     *response;                             // User's response string
	object_t *obj1;
	object_t *obj2;
	int       dx, dy;
	act      *action;                               // Ptr to action structure

	status_t &gameStatus = _vm.getGameStatus();

	action = curEvent->action;
	debugC(1, kDebugSchedule, "doAction - Event action type : %d", action->a0.actType);

	switch (action->a0.actType) {
	case ANULL:                                     // Big NOP from DEL_EVENTS
		break;
	case ASCHEDULE:                                 // act0: Schedule an action list
		insertActionList(action->a0.actIndex);
		break;
	case START_OBJ:                                 // act1: Start an object cycling
		_vm._objects[action->a1.objNumb].cycleNumb = action->a1.cycleNumb;
		_vm._objects[action->a1.objNumb].cycling = action->a1.cycle;
		break;
	case INIT_OBJXY:                                // act2: Initialise an object
		_vm._objects[action->a2.objNumb].x = action->a2.x;          // Coordinates
		_vm._objects[action->a2.objNumb].y = action->a2.y;
		break;
	case PROMPT:                                    // act3: Prompt user for key phrase
// TODO : Add specific code for Hugo 1 DOS, which is handled differently,
		response = Utils::Box(BOX_PROMPT, _vm.file().fetchString(action->a3.promptIndex));

		warning("STUB: doAction(act3), expecting answer %s", response);

// TODO : The answer of the player is not handled currently! Once it'll be read in the messageBox, uncomment this block
#if 0
		bool      found;
		char     *tmpStr;                           // General purpose string ptr

		for (found = false, dx = 0; !found && (action->a3.responsePtr[dx] != -1); dx++) {
			tmpStr = _vm.file().Fetch_string(action->a3.responsePtr[dx]);
			if (strstr(_vm.parser().strlwr(response) , tmpStr))
				found = true;
		}

		if (found)
			insertActionList(action->a3.actPassIndex);
		else
			insertActionList(action->a3.actFailIndex);
#endif

//HACK: As the answer is not read, currently it's always considered correct
		insertActionList(action->a3.actPassIndex);
		break;
	case BKGD_COLOR:                                // act4: Set new background color
		HugoEngine::get().screen().setBackgroundColor(action->a4.newBackgroundColor);
		break;
	case INIT_OBJVXY:                               // act5: Initialise an object
		_vm._objects[action->a5.objNumb].vx = action->a5.vx;        // velocities
		_vm._objects[action->a5.objNumb].vy = action->a5.vy;
		break;
	case INIT_CARRY:                                // act6: Initialise an object
		_vm._objects[action->a6.objNumb].carriedFl = action->a6.carriedFl;  // carried status
		break;
	case INIT_HF_COORD:                             // act7: Initialise an object to hero's "feet" coords
		_vm._objects[action->a7.objNumb].x = _vm._hero->x - 1;
		_vm._objects[action->a7.objNumb].y = _vm._hero->y + _vm._hero->currImagePtr->y2 - 1;
		_vm._objects[action->a7.objNumb].screenIndex = *_vm._screen_p;  // Don't forget screen!
		break;
	case NEW_SCREEN:                                // act8: Start new screen
		newScreen(action->a8.screenIndex);
		break;
	case INIT_OBJSTATE:                             // act9: Initialise an object state
		_vm._objects[action->a9.objNumb].state = action->a9.newState;
		break;
	case INIT_PATH:                                 // act10: Initialise an object path and velocity
		_vm._objects[action->a10.objNumb].pathType = (path_t) action->a10.newPathType;
		_vm._objects[action->a10.objNumb].vxPath = action->a10.vxPath;
		_vm._objects[action->a10.objNumb].vyPath = action->a10.vyPath;
		break;
	case COND_R:                                    // act11: action lists conditional on object state
		if (_vm._objects[action->a11.objNumb].state == action->a11.stateReq)
			insertActionList(action->a11.actPassIndex);
		else
			insertActionList(action->a11.actFailIndex);
		break;
	case TEXT:                                      // act12: Text box (CF WARN)
		Utils::Box(BOX_ANY, _vm.file().fetchString(action->a12.stringIndex));   // Fetch string from file
		break;
	case SWAP_IMAGES:                               // act13: Swap 2 object images
		swapImages(action->a13.obj1, action->a13.obj2);
		break;
	case COND_SCR:                                  // act14: Conditional on current screen
		if (_vm._objects[action->a14.objNumb].screenIndex == action->a14.screenReq)
			insertActionList(action->a14.actPassIndex);
		else
			insertActionList(action->a14.actFailIndex);
		break;
	case AUTOPILOT:                                 // act15: Home in on a (stationary) object
		// object p1 will home in on object p2
		obj1 = &_vm._objects[action->a15.obj1];
		obj2 = &_vm._objects[action->a15.obj2];
		obj1->pathType = AUTO;
		dx = obj1->x + obj1->currImagePtr->x1 - obj2->x - obj2->currImagePtr->x1;
		dy = obj1->y + obj1->currImagePtr->y1 - obj2->y - obj2->currImagePtr->y1;

		if (dx == 0)                                // Don't EVER divide by zero!
			dx = 1;
		if (dy == 0)
			dy = 1;

		if (abs(dx) > abs(dy)) {
			obj1->vx = action->a15.dx * -SIGN(dx);
			obj1->vy = abs((action->a15.dy * dy) / dx) * -SIGN(dy);
		} else {
			obj1->vy = action->a15.dy * -SIGN(dy);
			obj1->vx = abs((action->a15.dx * dx) / dy) * -SIGN(dx);
		}
		break;
	case INIT_OBJ_SEQ:                              // act16: Set sequence number to use
		// Note: Don't set a sequence at time 0 of a new screen, it causes
		// problems clearing the boundary bits of the object!  t>0 is safe
		_vm._objects[action->a16.objNumb].currImagePtr = _vm._objects[action->a16.objNumb].seqList[action->a16.seqIndex].seqPtr;
		break;
	case SET_STATE_BITS:                            // act17: OR mask with curr obj state
		_vm._objects[action->a17.objNumb].state |= action->a17.stateMask;
		break;
	case CLEAR_STATE_BITS:                          // act18: AND ~mask with curr obj state
		_vm._objects[action->a18.objNumb].state &= ~action->a18.stateMask;
		break;
	case TEST_STATE_BITS:                           // act19: If all bits set, do apass else afail
		if ((_vm._objects[action->a19.objNumb].state & action->a19.stateMask) == action->a19.stateMask)
			insertActionList(action->a19.actPassIndex);
		else
			insertActionList(action->a19.actFailIndex);
		break;
	case DEL_EVENTS:                                // act20: Remove all events of this action type
		// Note: actions are not deleted here, simply turned into NOPs!
		wrkEvent = _headEvent;                      // The earliest event
		while (wrkEvent) {                          // While events found in list
			saveEvent = wrkEvent->nextEvent;
			if (wrkEvent->action->a20.actType == action->a20.actTypeDel)
				delQueue(wrkEvent);
			wrkEvent = saveEvent;
		}
		break;
	case GAMEOVER:                                  // act21: Game over!
		// NOTE: Must wait at least 1 tick before issuing this action if
		// any objects are to be made invisible!
		gameStatus.gameOverFl = true;
		break;
	case INIT_HH_COORD:                             // act22: Initialise an object to hero's actual coords
		_vm._objects[action->a22.objNumb].x = _vm._hero->x;
		_vm._objects[action->a22.objNumb].y = _vm._hero->y;
		_vm._objects[action->a22.objNumb].screenIndex = *_vm._screen_p;// Don't forget screen!
		break;
	case EXIT:                                      // act23: Exit game back to DOS
		_vm.endGame();
		break;
	case BONUS:                                     // act24: Get bonus score for action
		processBonus(action->a24.pointIndex);
		break;
	case COND_BOX:                                  // act25: Conditional on bounding box
		obj1 = &_vm._objects[action->a25.objNumb];
		dx = obj1->x + obj1->currImagePtr->x1;
		dy = obj1->y + obj1->currImagePtr->y2;
		if ((dx >= action->a25.x1) && (dx <= action->a25.x2) &&
		        (dy >= action->a25.y1) && (dy <= action->a25.y2))
			insertActionList(action->a25.actPassIndex);
		else
			insertActionList(action->a25.actFailIndex);
		break;
	case SOUND:                                     // act26: Play a sound (or tune)
		if (action->a26.soundIndex < _vm._tunesNbr)
			_vm.sound().playMusic(action->a26.soundIndex);
		else
			_vm.sound().playSound(action->a26.soundIndex, BOTH_CHANNELS, MED_PRI);
		break;
	case ADD_SCORE:                                 // act27: Add object's value to score
		_vm.adjustScore(_vm._objects[action->a27.objNumb].objValue);
		break;
	case SUB_SCORE:                                 // act28: Subtract object's value from score
		_vm.adjustScore(-_vm._objects[action->a28.objNumb].objValue);
		break;
	case COND_CARRY:                                // act29: Conditional on object being carried
		if (_vm._objects[action->a29.objNumb].carriedFl)
			insertActionList(action->a29.actPassIndex);
		else
			insertActionList(action->a29.actFailIndex);
		break;
	case INIT_MAZE:                                 // act30: Enable and init maze structure
		_maze.enabledFl = true;
		_maze.size = action->a30.mazeSize;
		_maze.x1 = action->a30.x1;
		_maze.y1 = action->a30.y1;
		_maze.x2 = action->a30.x2;
		_maze.y2 = action->a30.y2;
		_maze.x3 = action->a30.x3;
		_maze.x4 = action->a30.x4;
		_maze.firstScreenIndex = action->a30.firstScreenIndex;
		break;
	case EXIT_MAZE:                                 // act31: Disable maze mode
		_maze.enabledFl = false;
		break;
	case INIT_PRIORITY:
		_vm._objects[action->a32.objNumb].priority = action->a32.priority;
		break;
	case INIT_SCREEN:
		_vm._objects[action->a33.objNumb].screenIndex = action->a33.screenIndex;
		break;
	case AGSCHEDULE:                                // act34: Schedule a (global) action list
		insertActionList(action->a34.actIndex);
		break;
	case REMAPPAL:                                  // act35: Remap a palette color
		HugoEngine::get().screen().remapPal(action->a35.oldColorIndex, action->a35.newColorIndex);
		break;
	case COND_NOUN:                                 // act36: Conditional on noun mentioned
		if (_vm.parser().isWordPresent(_vm._arrayNouns[action->a36.nounIndex]))
			insertActionList(action->a36.actPassIndex);
		else
			insertActionList(action->a36.actFailIndex);
		break;
	case SCREEN_STATE:                              // act37: Set new screen state
		_vm._screenStates[action->a37.screenIndex] = action->a37.newState;
		break;
	case INIT_LIPS:                                 // act38: Position lips on object
		_vm._objects[action->a38.lipsObjNumb].x = _vm._objects[action->a38.objNumb].x + action->a38.dxLips;
		_vm._objects[action->a38.lipsObjNumb].y = _vm._objects[action->a38.objNumb].y + action->a38.dyLips;
		_vm._objects[action->a38.lipsObjNumb].screenIndex = *_vm._screen_p; // Don't forget screen!
		_vm._objects[action->a38.lipsObjNumb].cycling = CYCLE_FORWARD;
		break;
	case INIT_STORY_MODE:                           // act39: Init story_mode flag
		// This is similar to the QUIET path mode, except that it is
		// independant of it and it additionally disables the ">" prompt
		gameStatus.storyModeFl = action->a39.storyModeFl;

		// End the game after story if this is special vendor demo mode
		if (gameStatus.demoFl && action->a39.storyModeFl == false)
			_vm.endGame();
		break;
	case WARN:                                      // act40: Text box (CF TEXT)
		Utils::Box(BOX_OK, _vm.file().fetchString(action->a40.stringIndex));
		break;
	case COND_BONUS:                                // act41: Perform action if got bonus
		if (_vm._points[action->a41.BonusIndex].scoredFl)
			insertActionList(action->a41.actPassIndex);
		else
			insertActionList(action->a41.actFailIndex);
		break;
	case TEXT_TAKE:                                 // act42: Text box with "take" message
		Utils::Box(BOX_ANY, TAKE_TEXT, _vm._arrayNouns[_vm._objects[action->a42.objNumb].nounIndex][TAKE_NAME]);
		break;
	case YESNO:                                     // act43: Prompt user for Yes or No
		warning("doAction(act43) - Yes/No Box");
		if (Utils::Box(BOX_YESNO, _vm.file().fetchString(action->a43.promptIndex)) != NULL)
			insertActionList(action->a43.actYesIndex);
		else
			insertActionList(action->a43.actNoIndex);
		break;
	case STOP_ROUTE:                                // act44: Stop any route in progress
		gameStatus.routeIndex = -1;
		break;
	case COND_ROUTE:                                // act45: Conditional on route in progress
		if (gameStatus.routeIndex >= action->a45.routeIndex)
			insertActionList(action->a45.actPassIndex);
		else
			insertActionList(action->a45.actFailIndex);
		break;
	case INIT_JUMPEXIT:                             // act46: Init status.jumpexit flag
		// This is to allow left click on exit to get there immediately
		// For example the plane crash in Hugo2 where hero is invisible
		// Couldn't use INVISIBLE flag since conflicts with boat in Hugo1
		gameStatus.jumpExitFl = action->a46.jumpExitFl;
		break;
	case INIT_VIEW:                                 // act47: Init object.viewx, viewy, dir
		_vm._objects[action->a47.objNumb].viewx = action->a47.viewx;
		_vm._objects[action->a47.objNumb].viewy = action->a47.viewy;
		_vm._objects[action->a47.objNumb].direction = action->a47.direction;
		break;
	case INIT_OBJ_FRAME:                            // act48: Set seq,frame number to use
		// Note: Don't set a sequence at time 0 of a new screen, it causes
		// problems clearing the boundary bits of the object!  t>0 is safe
		_vm._objects[action->a48.objNumb].currImagePtr = _vm._objects[action->a48.objNumb].seqList[action->a48.seqIndex].seqPtr;
		for (dx = 0; dx < action->a48.frameIndex; dx++)
			_vm._objects[action->a48.objNumb].currImagePtr = _vm._objects[action->a48.objNumb].currImagePtr->nextSeqPtr;
		break;
	case OLD_SONG:
		//TODO For Hugo 1 and Hugo2 DOS: The songs were not stored in a DAT file, but directly as
		//strings. the current play_music should be modified to use a strings instead of reading
		//the file, in those cases. This replaces, for those DOS versions, act26.
		warning("STUB: doAction(act49)");
		break;
	default:
		Utils::Error(EVNT_ERR, "doAction");
		break;
	}

	if (action->a0.actType == NEW_SCREEN)           // New_screen() deletes entire list
		return (NULL);                              // next_p = NULL since list now empty
	else {
		wrkEvent = curEvent->nextEvent;
		delQueue(curEvent);                         // Return event to free list
		return(wrkEvent);                           // Return next event ptr
	}
}

// This is the scheduler which runs every tick.  It examines the event queue
// for any events whose time has come.  It dequeues these events and performs
// the action associated with the event, returning it to the free queue
void Scheduler::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	status_t &gameStatus = _vm.getGameStatus();

	event_t *curEvent = _headEvent;                 // The earliest event
	while (curEvent && curEvent->time <= gameStatus.tick) // While mature events found
		curEvent = doAction(curEvent);              // Perform the action (returns next_p)
	gameStatus.tick++;                              // Accessed elsewhere via getTicks()
}

uint32 Scheduler::getTicks() {
// Return system time in ticks.  A tick is 1/TICKS_PER_SEC mS
	debugC(3, kDebugSchedule, "getTicks");

	return _vm.getGameStatus().tick;
}

void Scheduler::processBonus(int bonusIndex) {
// Add indecated bonus to score if not added already
	debugC(1, kDebugSchedule, "processBonus(%d)", bonusIndex);

	if (!_vm._points[bonusIndex].scoredFl) {
		_vm.adjustScore(_vm._points[bonusIndex].score);
		_vm._points[bonusIndex].scoredFl = true;
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
	if (!_vm.isPacked()) {
		char line[32];
		if (!_vm.file().fileExists(strcat(strncat(strcpy(line, _vm._picDir), _vm._screenNames[screenIndex], NAME_LEN), BKGEXT)) &&
		        !_vm.file().fileExists(strcat(strcpy(line, _vm._screenNames[screenIndex]), ".ART"))) {
			Utils::Box(BOX_ANY, _vm._textSchedule[kSsNoBackground]);
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
	_vm.setNewScreen(screenIndex);

	// 3. Read in new screen files
	_vm.readScreenFiles(screenIndex);

	// 4. Schedule action list for this screen
	_vm.screenActions(screenIndex);

	// 5. Initialise prompt line and status line
	_vm.initNewScreenDisplay();
}

// Write the event queue to the file with handle f
// Note that we convert all the event structure ptrs to indexes
// using -1 for NULL.  We can't convert the action ptrs to indexes
// so we save address of first dummy action ptr to compare on restore.
void Scheduler::saveEvents(Common::WriteStream *f) {
	uint32   curTime;
	event_t  saveEvents_[kMaxEvents];                // Convert event ptrs to indexes
	event_t *wrkEvent;                              // Event ptr
	int16    freeIndex;                             // Free list index
	int16    headIndex;                             // Head of list index
	int16    tailIndex;                             // Tail of list index

	debugC(1, kDebugSchedule, "saveEvents");

	curTime = getTicks();

	// Convert event ptrs to indexes
	for (int16 i = 0; i < kMaxEvents; i++) {
		wrkEvent = &_events[i];
		saveEvents_[i] = *wrkEvent;
		saveEvents_[i].prevEvent = (wrkEvent->prevEvent == NULL) ? (event_t *) - 1 : (event_t *)(wrkEvent->prevEvent - _events);
		saveEvents_[i].nextEvent = (wrkEvent->nextEvent == NULL) ? (event_t *) - 1 : (event_t *)(wrkEvent->nextEvent - _events);
	}
	freeIndex = (_freeEvent == 0) ? -1 : _freeEvent - _events;
	headIndex = (_headEvent == 0) ? -1 : _headEvent - _events;
	tailIndex = (_tailEvent == 0) ? -1 : _tailEvent - _events;

	f->write(&curTime,   sizeof(curTime));
	f->write(&freeIndex, sizeof(freeIndex));
	f->write(&headIndex, sizeof(headIndex));
	f->write(&tailIndex, sizeof(tailIndex));
	f->write(saveEvents_, sizeof(saveEvents_));
}

// Restore the event list from file with handle f
void Scheduler::restoreEvents(Common::SeekableReadStream *f) {
	uint32   curTime, saveTime;
	event_t *wrkEvent;                              // Event ptr
	event_t  savedEvents[kMaxEvents];               // Convert event ptrs to indexes
	int16    freeIndex;                             // Free list index
	int16    headIndex;                             // Head of list index
	int16    tailIndex;                             // Tail of list index

	debugC(1, kDebugSchedule, "restoreEvents");

	f->read(&saveTime,  sizeof(saveTime));          // time of save
	f->read(&freeIndex, sizeof(freeIndex));
	f->read(&headIndex, sizeof(headIndex));
	f->read(&tailIndex, sizeof(tailIndex));
	f->read(savedEvents, sizeof(savedEvents));

	// Restore events indexes to pointers
	for (int i = 0; i < kMaxEvents; i++) {
		wrkEvent = &savedEvents[i];
		_events[i] = *wrkEvent;
		_events[i].prevEvent = (wrkEvent->prevEvent == (event_t *) - 1) ? (event_t *)0 : &_events[(size_t)wrkEvent->prevEvent ];
		_events[i].nextEvent = (wrkEvent->nextEvent == (event_t *) - 1) ? (event_t *)0 : &_events[(size_t)wrkEvent->nextEvent ];
	}
	_freeEvent = (freeIndex == -1) ? NULL : &_events[freeIndex];
	_headEvent = (headIndex == -1) ? NULL : &_events[headIndex];
	_tailEvent = (tailIndex == -1) ? NULL : &_events[tailIndex];

	// Adjust times to fit our time
	curTime = getTicks();
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
	_vm.setNewScreen(screenIndex);

	// 2. Read in new screen files
	_vm.readScreenFiles(screenIndex);

	// 3. Initialise prompt line and status line
	_vm.initNewScreenDisplay();
}

void Scheduler::swapImages(int objNumb1, int objNumb2) {
// Swap all the images of one object with another.  Set hero_image (we make
// the assumption for now that the first obj is always the HERO) to the object
// number of the swapped image
	seqList_t tmpSeqList[MAX_SEQUENCES];
	int seqListSize = sizeof(seqList_t) * MAX_SEQUENCES;

	debugC(1, kDebugSchedule, "swapImages(%d, %d)", objNumb1, objNumb2);

	_vm.file().saveSeq(&_vm._objects[objNumb1]);
	memcpy(tmpSeqList, _vm._objects[objNumb1].seqList, seqListSize);
	memcpy(_vm._objects[objNumb1].seqList, _vm._objects[objNumb2].seqList, seqListSize);
	memcpy(_vm._objects[objNumb2].seqList, tmpSeqList, seqListSize);
	_vm.file().restoreSeq(&_vm._objects[objNumb1]);
	_vm._objects[objNumb2].currImagePtr = _vm._objects[objNumb2].seqList[0].seqPtr;
	_vm._heroImage = (_vm._heroImage == HERO) ? objNumb2 : HERO;

	// Make sure baseline stays constant
	_vm._objects[objNumb1].y += _vm._objects[objNumb2].currImagePtr->y2 - _vm._objects[objNumb1].currImagePtr->y2;
}

} // end of namespace Hugo
