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

#ifndef SAGA2_TIMERS_H
#define SAGA2_TIMERS_H

#include "saga2/objects.h"
#include "saga2/calender.h"

namespace Saga2 {

//  Allocate an new TimerList
void *newTimerList(void);
//  Deallocate an TimerList
void deleteTimerList(void *p);

//  Fetch a specified actor's TimerList
TimerList *fetchTimerList(GameObject *obj);

//  Allocate a new Timer
void *newTimer(void);
//  Deallocate an Timer
void deleteTimer(void *p);

//  Check all active Timers
void checkTimers(void);

//  Initialize the Timers
void initTimers(void);
//  Save the active Timers in a save file
void saveTimers(SaveFileConstructor &saveGame);
//  Load Timers from a save file
void loadTimers(SaveFileReader &saveGame);
//  Cleanup the active Timers
void cleanupTimers(void);

/* ===================================================================== *
   TimerList class
 * ===================================================================== */

class TimerList : public DList {
	GameObject          *obj;
public:
	//  Constructor -- initial construction
	TimerList(GameObject *o) : obj(o) {}

	//  Constructor -- reconstruct from archive buffer
	TimerList(void **buf);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize(void) {
		return sizeof(ObjectID);
	}

	//  Archive this object in a buffer
	void *archive(void *buf);

	void *operator new (size_t) {
		return newTimerList();
	}
	void operator delete (void *p) {
		deleteTimerList(p);
	}

	GameObject *getObject(void) {
		return obj;
	}
};

/* ===================================================================== *
   Timer class
 * ===================================================================== */

class Timer : public DNode {
	GameObject      *obj;
	TimerID         id;
	int16           interval;
	FrameAlarm      alarm;

public:
	//  Constructor -- initial construction
	Timer(GameObject *o, TimerID timerID, int16 frameInterval) :
		obj(o),
		id(timerID),
		interval(frameInterval) {
		alarm.set(interval);
	}

	//  Constructor -- reconstruct from archive buffer
	Timer(void **buf);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	void *operator new (size_t) {
		return newTimer();
	}
	void operator delete (void *p) {
		deleteTimer(p);
	}

	GameObject *getObject(void) {
		return obj;
	}
	TimerID thisID(void) {
		return id;
	}
	int16 getInterval(void) {
		return interval;
	}

	bool check(void) {
		return alarm.check();
	}
	void reset(void) {
		alarm.set(interval);
	}
};

} // end of namespace Saga2

#endif
