/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
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

class Timer;
class TimerList;

//  Fetch a specified actor's TimerList
TimerList *fetchTimerList(GameObject *obj);

void deleteTimer(Timer *t);

//  Check all active Timers
void checkTimers();

//  Initialize the Timers
void initTimers();
void saveTimers(Common::OutSaveFile *outS);
void loadTimers(Common::InSaveFile *in);
void cleanupTimer();
//  Cleanup the active Timers
void cleanupTimers();

/* ===================================================================== *
   TimerList class
 * ===================================================================== */

class TimerList {
	GameObject *_obj;

public:
	//  Constructor -- initial construction
	TimerList(GameObject *o);

	TimerList(Common::InSaveFile *in);

	~TimerList();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize() {
		return sizeof(ObjectID);
	}

	void write(Common::MemoryWriteStreamDynamic *out);

	GameObject *getObject() {
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
	bool _active;

	//  Constructor -- initial construction
	Timer(GameObject *o, TimerID timerID, int16 frameInterval) : _obj(o), _id(timerID), _interval(frameInterval), _active(true) {
		_alarm.set(_interval);
		debugC(1, kDebugTimers, "Creating timer %p for %p (%s)",
		       (void *)this, (void *)o, o->objName());

		g_vm->_timers.push_back(this);
	}

	Timer(Common::InSaveFile *in);

	~Timer();

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	static int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	GameObject *getObject() {
		return _obj;
	}
	TimerID thisID() {
		return _id;
	}
	int16 getInterval() {
		return _interval;
	}

	bool check() {
		return _alarm.check();
	}
	void reset() {
		_alarm.set(_interval);
	}
};

} // end of namespace Saga2

#endif
