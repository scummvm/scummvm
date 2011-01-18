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
#include "hugo/parser.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"

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
* This function performs the action in the event structure pointed to by p
* It dequeues the event and returns it to the free list.  It returns a ptr
* to the next action in the list, except special case of NEW_SCREEN
*/
event_t *Scheduler_v1d::doAction(event_t *curEvent) {
	debugC(1, kDebugSchedule, "doAction - Event action type : %d", curEvent->action->a0.actType);

	status_t &gameStatus = _vm->getGameStatus();
	act *action = curEvent->action;
	char     *response;                             // User's response string
	object_t *obj1;
	object_t *obj2;
	int       dx, dy;
	event_t  *wrkEvent;                             // Save ev_p->next_p for return

	switch (action->a0.actType) {
	case ANULL:                                     // Big NOP from DEL_EVENTS
		break;
	case ASCHEDULE:                                 // act0: Schedule an action list
		insertActionList(action->a0.actIndex);
		break;
	case START_OBJ:                                 // act1: Start an object cycling
		_vm->_object->_objects[action->a1.objIndex].cycleNumb = action->a1.cycleNumb;
		_vm->_object->_objects[action->a1.objIndex].cycling = action->a1.cycle;
		break;
	case INIT_OBJXY:                                // act2: Initialise an object
		_vm->_object->_objects[action->a2.objIndex].x = action->a2.x;          // Coordinates
		_vm->_object->_objects[action->a2.objIndex].y = action->a2.y;
		break;
	case PROMPT: {                                  // act3: Prompt user for key phrase
		response = Utils::Box(BOX_PROMPT, "%s", _vm->_file->fetchString(action->a3.promptIndex));
		strcpy(response, _vm->_file->fetchString(action->a3.promptIndex));
		if (action->a3.encodedFl)
			decodeString(response);

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
		break;
		}
	case BKGD_COLOR:                                // act4: Set new background color
		_vm->_screen->setBackgroundColor(action->a4.newBackgroundColor);
		break;
	case INIT_OBJVXY:                               // act5: Initialise an object velocity
		_vm->_object->setVelocity(action->a5.objIndex, action->a5.vx, action->a5.vy);
		break;
	case INIT_CARRY:                                // act6: Initialise an object
		_vm->_object->setCarry(action->a6.objIndex, action->a6.carriedFl);  // carried status
		break;
	case INIT_HF_COORD:                             // act7: Initialise an object to hero's "feet" coords
		_vm->_object->_objects[action->a7.objIndex].x = _vm->_hero->x - 1;
		_vm->_object->_objects[action->a7.objIndex].y = _vm->_hero->y + _vm->_hero->currImagePtr->y2 - 1;
		_vm->_object->_objects[action->a7.objIndex].screenIndex = *_vm->_screen_p;  // Don't forget screen!
		break;
	case NEW_SCREEN:                                // act8: Start new screen
		newScreen(action->a8.screenIndex);
		break;
	case INIT_OBJSTATE:                             // act9: Initialise an object state
		_vm->_object->_objects[action->a9.objIndex].state = action->a9.newState;
		break;
	case INIT_PATH:                                 // act10: Initialise an object path and velocity
		_vm->_object->setPath(action->a10.objIndex, (path_t) action->a10.newPathType, action->a10.vxPath, action->a10.vyPath);
		break;
	case COND_R:                                    // act11: action lists conditional on object state
		if (_vm->_object->_objects[action->a11.objIndex].state == action->a11.stateReq)
			insertActionList(action->a11.actPassIndex);
		else
			insertActionList(action->a11.actFailIndex);
		break;
	case TEXT:                                      // act12: Text box (CF WARN)
		Utils::Box(BOX_ANY, "%s", _vm->_file->fetchString(action->a12.stringIndex));   // Fetch string from file
		break;
	case SWAP_IMAGES:                               // act13: Swap 2 object images
		_vm->_object->swapImages(action->a13.objIndex1, action->a13.objIndex2);
		break;
	case COND_SCR:                                  // act14: Conditional on current screen
		if (_vm->_object->_objects[action->a14.objIndex].screenIndex == action->a14.screenReq)
			insertActionList(action->a14.actPassIndex);
		else
			insertActionList(action->a14.actFailIndex);
		break;
	case AUTOPILOT:                                 // act15: Home in on a (stationary) object
		// object p1 will home in on object p2
		obj1 = &_vm->_object->_objects[action->a15.objIndex1];
		obj2 = &_vm->_object->_objects[action->a15.objIndex2];
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
			obj1->vy = action->a15.dy * SIGN(dy);
			obj1->vx = abs((action->a15.dx * dx) / dy) * SIGN(dx);
		}
		break;
	case INIT_OBJ_SEQ:                              // act16: Set sequence number to use
		// Note: Don't set a sequence at time 0 of a new screen, it causes
		// problems clearing the boundary bits of the object!  t>0 is safe
		_vm->_object->_objects[action->a16.objIndex].currImagePtr = _vm->_object->_objects[action->a16.objIndex].seqList[action->a16.seqIndex].seqPtr;
		break;
	case SET_STATE_BITS:                            // act17: OR mask with curr obj state
		_vm->_object->_objects[action->a17.objIndex].state |= action->a17.stateMask;
		break;
	case CLEAR_STATE_BITS:                          // act18: AND ~mask with curr obj state
		_vm->_object->_objects[action->a18.objIndex].state &= ~action->a18.stateMask;
		break;
	case TEST_STATE_BITS:                           // act19: If all bits set, do apass else afail
		if ((_vm->_object->_objects[action->a19.objIndex].state & action->a19.stateMask) == action->a19.stateMask)
			insertActionList(action->a19.actPassIndex);
		else
			insertActionList(action->a19.actFailIndex);
		break;
	case DEL_EVENTS:                                // act20: Remove all events of this action type
		// Note: actions are not deleted here, simply turned into NOPs!
		wrkEvent = _headEvent;                      // The earliest event
		while (wrkEvent) {                          // While events found in list
			if (wrkEvent->action->a20.actType == action->a20.actTypeDel)
				wrkEvent->action->a20.actType = ANULL;
			wrkEvent = wrkEvent->nextEvent;
		}
		break;
	case GAMEOVER:                                  // act21: Game over!
		// NOTE: Must wait at least 1 tick before issuing this action if
		// any objects are to be made invisible!
		gameStatus.gameOverFl = true;
		break;
	case INIT_HH_COORD:                             // act22: Initialise an object to hero's actual coords
		_vm->_object->_objects[action->a22.objIndex].x = _vm->_hero->x;
		_vm->_object->_objects[action->a22.objIndex].y = _vm->_hero->y;
		_vm->_object->_objects[action->a22.objIndex].screenIndex = *_vm->_screen_p;// Don't forget screen!
		break;
	case EXIT:                                      // act23: Exit game back to DOS
		_vm->endGame();
		break;
	case BONUS:                                     // act24: Get bonus score for action
		processBonus(action->a24.pointIndex);
		break;
	case COND_BOX:                                  // act25: Conditional on bounding box
		obj1 = &_vm->_object->_objects[action->a25.objIndex];
		dx = obj1->x + obj1->currImagePtr->x1;
		dy = obj1->y + obj1->currImagePtr->y2;
		if ((dx >= action->a25.x1) && (dx <= action->a25.x2) &&
		        (dy >= action->a25.y1) && (dy <= action->a25.y2))
			insertActionList(action->a25.actPassIndex);
		else
			insertActionList(action->a25.actFailIndex);
		break;
	case ADD_SCORE:                                 // act27: Add object's value to score
		_vm->adjustScore(_vm->_object->_objects[action->a27.objIndex].objValue);
		break;
	case SUB_SCORE:                                 // act28: Subtract object's value from score
		_vm->adjustScore(-_vm->_object->_objects[action->a28.objIndex].objValue);
		break;
	case COND_CARRY:                                // act29: Conditional on object being carried
		if (_vm->_object->isCarried(action->a29.objIndex))
			insertActionList(action->a29.actPassIndex);
		else
			insertActionList(action->a29.actFailIndex);
		break;
	case OLD_SONG:
		// Replaces ACT26 for DOS games.
		_vm->_sound->DOSSongPtr = _vm->_textData[action->a49.songIndex];
		break;
	default:
		error("An error has occurred: %s", "doAction");
		break;
	}

	if (action->a0.actType == NEW_SCREEN) {         // New_screen() deletes entire list
		return 0;                                   // next_p = 0 since list now empty
	} else {
		wrkEvent = curEvent->nextEvent;
		delQueue(curEvent);                         // Return event to free list
		return wrkEvent;                            // Return next event ptr
	}
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

} // End of namespace Hugo
