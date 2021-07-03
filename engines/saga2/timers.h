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

#include "saga2/idtypes.h"
#include "saga2/calender.h"
#include "saga2/objects.h"

namespace Saga2 {

class TimerList;

//  Fetch a specified actor's TimerList
TimerList *fetchTimerList(GameObject *obj);

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

class TimerList {
	GameObject *_obj;

public:
	//  Constructor -- initial construction
	TimerList(GameObject *o);

	//  Constructor -- reconstruct from archive buffer
	TimerList(void **buf);

	~TimerList();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize(void) {
		return sizeof(ObjectID);
	}

	//  Archive this object in a buffer
	void *archive(void *buf);

	GameObject *getObject(void) {
		return _obj;
	}

	Common::List<Timer *> _timers;
};

/* ===================================================================== *
   Timer class
 * ===================================================================== */

class Timer {
	GameObject *_obj;
	TimerID _id;
	int16 _interval;
	FrameAlarm _alarm;

public:
	//  Constructor -- initial construction
	Timer(GameObject *o, TimerID timerID, int16 frameInterval) : _obj(o), _id(timerID), _interval(frameInterval) {
		_alarm.set(_interval);
		debugC(1, kDebugTimers, "Creating timer %p for %p (%s)",
		       (void *)this, (void *)o, o->objName());

		g_vm->_timers.push_back(this);
	}

	//  Constructor -- reconstruct from archive buffer
	Timer(void **buf);
	~Timer();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	GameObject *getObject(void) {
		return _obj;
	}
	TimerID thisID(void) {
		return _id;
	}
	int16 getInterval(void) {
		return _interval;
	}

	bool check(void) {
		return _alarm.check();
	}
	void reset(void) {
		_alarm.set(_interval);
	}
};

} // end of namespace Saga2

#endif
