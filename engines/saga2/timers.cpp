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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "common/timer.h"

#include "saga2/std.h"
#include "saga2/fta.h"
#include "saga2/timers.h"
#include "saga2/pool.h"
#include "saga2/objects.h"
#include "saga2/savefile.h"

namespace Saga2 {

volatile int32 gameTime;
bool timerPaused = false;

void timerCallback(void *refCon) {
	if (!timerPaused)
		gameTime++;
}

void initTimer(void) {
	gameTime = 0;

	g_vm->getTimerManager()->installTimerProc(&timerCallback, 1000 / 72, nullptr, "saga2");
}

void pauseTimer() {
	timerPaused = true;
}

void resumeTimer() {
	timerPaused = false;
}

void saveTimer(SaveFileConstructor &saveGame) {
	int32   time = gameTime;

	saveGame.writeChunk(
	    MakeID('T', 'I', 'M', 'E'),
	    &time,
	    sizeof(time));
}

void loadTimer(SaveFileReader &saveGame) {
	int32   time;

	saveGame.read(&time, sizeof(time));
	gameTime = time;
}

/* ====================================================================== *
   Alarms
 * ====================================================================== */

void Alarm::set(uint32 dur) {
	basetime = gameTime;
	duration = dur;
}

bool Alarm::check(void) {
	return ((uint32)(gameTime - basetime) > duration);
}

// time elapsed since alarm set

uint32 Alarm::elapsed(void) {
	return (uint32)(gameTime - basetime);
}

struct TimerListHolder : public DNode {
	uint8       timerListBuffer[sizeof(TimerList)];

	TimerList *getTimerList(void) {
		return (TimerList *)&timerListBuffer;
	}
};

/* ===================================================================== *
   TimerHolder class
 * ===================================================================== */

struct TimerHolder : public DNode {
	uint8       timerBuffer[sizeof(Timer)];

	Timer *getTimer(void) {
		return (Timer *)&timerBuffer;
	}
};

/* ===================================================================== *
   Globals
 * ===================================================================== */

//  A pool of TimerListHolders
static RPool< TimerListHolder, 64 > timerListPool;

//  The list of active TimerLists
static DList timerListList;

//  A pool of TimerHolders
static RPool< TimerHolder, 128 > timerPool;

//  The list of active Timers
static DList timerList;

/* ===================================================================== *
   TimerList management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Allocate a new TimerList

void *newTimerList(void) {
	TimerListHolder     *newTimerListHolder;

	if ((newTimerListHolder
	        = (TimerListHolder *)timerListPool.alloc())
	        ==  NULL)
		return NULL;

	timerListList.addTail(*newTimerListHolder);

	return &newTimerListHolder->timerListBuffer;
}

//----------------------------------------------------------------------
//	Deallocate an TimerList

void deleteTimerList(void *p) {
	TimerListHolder     *listHolderToDelete;

	warning("FIXME: deleteTimerList(): unsafe pointer arithmetics");
	listHolderToDelete =
	    (TimerListHolder *)((uint8 *)p
	                        -   offsetof(
	                            TimerListHolder,
	                            timerListBuffer));

	listHolderToDelete->remove();
	timerListPool.free(listHolderToDelete);
}

//----------------------------------------------------------------------
//	Fetch a specified object's TimerList

TimerList *fetchTimerList(GameObject *obj) {
	TimerListHolder     *listHolder;

	for (listHolder = (TimerListHolder *)timerListList.first();
	        listHolder != NULL;
	        listHolder = (TimerListHolder *)listHolder->next()) {
		if (listHolder->getTimerList()->getObject() == obj)
			return listHolder->getTimerList();
	}

	return NULL;
}

/* ===================================================================== *
   Timer management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Allocate an new Timer

void *newTimer(void) {
	TimerHolder     *newTimerHolder;

	if ((newTimerHolder = (TimerHolder *)timerPool.alloc()) == NULL)
		return NULL;

	timerList.addTail(*newTimerHolder);

	return &newTimerHolder->timerBuffer;
}

//----------------------------------------------------------------------
//	Deallocated an Timer

void deleteTimer(void *p) {
	TimerHolder     *timerHolderToDelete;

	warning("FIXME: deleteTimer(): unsafe pointer arithmetics");
	timerHolderToDelete =
	    (TimerHolder *)((uint8 *)p - offsetof(TimerHolder, timerBuffer));

	timerHolderToDelete->remove();
	timerPool.free(timerHolderToDelete);
}

//----------------------------------------------------------------------
//	Check all active Timers

void checkTimers(void) {
	TimerHolder     *timerHolder,
	                *nextTimerHolder;

	for (timerHolder = (TimerHolder *)timerList.first();
	        timerHolder != NULL;
	        timerHolder = nextTimerHolder) {
		nextTimerHolder = (TimerHolder *)timerHolder->next();

		Timer       *timer = timerHolder->getTimer();

		if (timer->check()) {
			timer->reset();
			timer->getObject()->timerTick(timer->thisID());
		}
	}
}

//----------------------------------------------------------------------
//	Initialize the Timers

void initTimers(void) {
	//  Nothing to do
}

//----------------------------------------------------------------------
//	Save the active Timers in a save file

void saveTimers(SaveFileConstructor &saveGame) {
	int16                   timerListCount = 0,
	                        timerCount = 0;

	TimerListHolder     *listHolder;
	TimerHolder         *timerHolder;

	void                *archiveBuffer,
	                    *bufferPtr;
	int32               archiveBufSize = 0;

	//  Add the sizes of the timer list count an timer count
	archiveBufSize += sizeof(timerListCount) + sizeof(timerCount);

	//  Tally the timer lists
	for (listHolder = (TimerListHolder *)timerListList.first();
	        listHolder != NULL;
	        listHolder = (TimerListHolder *)listHolder->next())
		timerListCount++;

	//  Add the total archive size of all of the timer lists
	archiveBufSize += timerListCount * TimerList::archiveSize();

	//  Tally the timers
	for (timerHolder = (TimerHolder *)timerList.first();
	        timerHolder != NULL;
	        timerHolder = (TimerHolder *)timerHolder->next())
		timerCount++;

	//  Add the total archive size of all of the timers
	archiveBufSize += timerCount * Timer::archiveSize();

	//  Allocate an archive buffer
	archiveBuffer = RNewPtr(archiveBufSize, NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate timer archive buffer");

	bufferPtr = archiveBuffer;

	//  Store the timer list count and timer count
	*((int16 *)bufferPtr)      = timerListCount;
	*((int16 *)bufferPtr + 1)  = timerCount;
	bufferPtr = (int16 *)bufferPtr + 2;

	//  Archive all timer lists
	for (listHolder = (TimerListHolder *)timerListList.first();
	        listHolder != NULL;
	        listHolder = (TimerListHolder *)listHolder->next())
		bufferPtr = listHolder->getTimerList()->archive(bufferPtr);

	//  Archive all timers
	for (timerHolder = (TimerHolder *)timerList.first();
	        timerHolder != NULL;
	        timerHolder = (TimerHolder *)timerHolder->next())
		bufferPtr = timerHolder->getTimer()->archive(bufferPtr);

	assert(bufferPtr == &((uint8 *)archiveBuffer)[archiveBufSize]);

	//  Write the data to the save file
	saveGame.writeChunk(
	    MakeID('T', 'I', 'M', 'R'),
	    archiveBuffer,
	    archiveBufSize);

	RDisposePtr(archiveBuffer);
}

//----------------------------------------------------------------------
//	Load the Timers from a save file

void loadTimers(SaveFileReader &saveGame) {
	int16       i,
	            timerListCount,
	            timerCount;

	void        *archiveBuffer,
	            *bufferPtr;

	//  Allocate a buffer in which to read the archive data
	archiveBuffer = RNewPtr(saveGame.getChunkSize(), NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate timer archive buffer");

	//  Read the data
	saveGame.read(archiveBuffer, saveGame.getChunkSize());

	bufferPtr = archiveBuffer;

	//  Get the timer list count and timer count
	timerListCount  = *((int16 *)bufferPtr);
	timerCount      = *((int16 *)bufferPtr + 1);
	bufferPtr = (int16 *)bufferPtr + 2;

	//  Restore all timer lists
	for (i = 0; i < timerListCount; i++)
		new TimerList(&bufferPtr);

	//  Restore all timers
	for (i = 0; i < timerCount; i++) {
		Timer       *timer;
		TimerList   *timerList;

		timer = new Timer(&bufferPtr);

		assert(timer != NULL);

		//  Get the objects's timer list
		timerList = fetchTimerList(timer->getObject());

		assert(timerList != NULL);

		//  Append this timer to the objects's timer list
		timerList->addTail(*timer);
	}

	assert(bufferPtr == &((uint8 *)archiveBuffer)[saveGame.getChunkSize()]);

	RDisposePtr(archiveBuffer);
}

//----------------------------------------------------------------------
//	Cleanup the active Timers

void cleanupTimers(void) {
	TimerListHolder     *listHolder,
	                    *nextListHolder;
	TimerHolder         *timerHolder,
	                    *nextTimerHolder;

	//  Delete all timer lists
	for (listHolder = (TimerListHolder *)timerListList.first();
	        listHolder != NULL;
	        listHolder = nextListHolder) {
		nextListHolder = (TimerListHolder *)listHolder->next();

		delete listHolder->getTimerList();
	}

	//  Delete all timers
	for (timerHolder = (TimerHolder *)timerList.first();
	        timerHolder != NULL;
	        timerHolder = nextTimerHolder) {
		nextTimerHolder = (TimerHolder *)timerHolder->next();

		delete timerHolder->getTimer();
	}
}

/* ===================================================================== *
   TimerList member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

TimerList::TimerList(void **buf) {
	ObjectID        *bufferPtr = (ObjectID *)*buf;

	assert(isObject(*bufferPtr) || isActor(*bufferPtr));

	//  Restore the object pointer
	obj = GameObject::objectAddress(*bufferPtr++);

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *TimerList::archive(void *buf) {
	//  Store the object's ID
	*((ObjectID *)buf) = obj->thisID();
	buf = (ObjectID *)buf + 1;

	return buf;
}

/* ===================================================================== *
   Timer member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from an archive buffer

Timer::Timer(void **buf) {
	void        *bufferPtr = *buf;

	assert(isObject(*((ObjectID *)bufferPtr))
	       ||  isActor(*((ObjectID *)bufferPtr)));

	//  Restore the object pointer
	obj = GameObject::objectAddress(*((ObjectID *)bufferPtr));
	bufferPtr = (ObjectID *)bufferPtr + 1;

	//  Restore the timer's ID
	id = *((TimerID *)bufferPtr);
	bufferPtr = (TimerID *)bufferPtr + 1;

	//  Restore the frame interval
	interval = *((int16 *)bufferPtr);
	bufferPtr = (int16 *)bufferPtr + 1;

	//  Restore the alarm
	memcpy(&alarm, bufferPtr, sizeof(alarm));
	bufferPtr = (FrameAlarm *)bufferPtr + 1;

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

int32 Timer::archiveSize(void) {
	return      sizeof(ObjectID)         //  obj's ID
	            +   sizeof(TimerID)
	            +   sizeof(int16)
	            +   sizeof(FrameAlarm);
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *Timer::archive(void *buf) {
	//  Store the obj's ID
	*((ObjectID *)buf) = obj->thisID();
	buf = (ObjectID *)buf + 1;

	//  Store the timer's ID
	*((TimerID *)buf) = id;
	buf = (TimerID *)buf + 1;

	//  Store the frame interval
	*((int16 *)buf) = interval;
	buf = (int16 *)buf + 1;

	//  Store the alarm
	memcpy(buf, &alarm, sizeof(alarm));
	buf = (FrameAlarm *)buf + 1;

	return buf;
}

} // end of namespace Saga2
