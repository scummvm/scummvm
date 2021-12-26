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

#include "common/timer.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/timers.h"
#include "saga2/objects.h"
#include "saga2/saveload.h"

namespace Saga2 {

volatile int32 gameTime;
bool timerPaused = false;

void timerCallback(void *refCon) {
	if (!timerPaused)
		gameTime++;
}

void initTimer() {
	gameTime = 0;

	g_vm->getTimerManager()->installTimerProc(&timerCallback, 1000000 / 72, nullptr, "saga2");
}

void pauseTimer() {
	timerPaused = true;
}

void resumeTimer() {
	timerPaused = false;
}

void saveTimer(Common::OutSaveFile *out) {
	debugC(2, kDebugSaveload, "Saving time");

	int32 time = gameTime;

	out->write("TIME", 4);
	out->writeUint32LE(sizeof(time));
	out->writeUint32LE(time);

	debugC(3, kDebugSaveload, "... time = %d", time);
}

void loadTimer(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading time");

	gameTime = in->readUint32LE();

	debugC(3, kDebugSaveload, "... time = %d", gameTime);
}

void cleanupTimer() {
	g_vm->getTimerManager()->removeTimerProc(&timerCallback);
}

/* ====================================================================== *
   Alarms
 * ====================================================================== */

void Alarm::write(Common::MemoryWriteStreamDynamic *out) {
	out->writeUint32LE(basetime);
	out->writeUint32LE(duration);
}

void Alarm::read(Common::InSaveFile *in) {
	basetime = in->readUint32LE();
	duration = in->readUint32LE();
}

void Alarm::set(uint32 dur) {
	basetime = gameTime;
	duration = dur;
}

bool Alarm::check() {
	return ((uint32)(gameTime - basetime) > duration);
}

// time elapsed since alarm set

uint32 Alarm::elapsed() {
	return (uint32)(gameTime - basetime);
}

/* ===================================================================== *
   TimerList management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Fetch a specified object's TimerList

TimerList *fetchTimerList(GameObject *obj) {
	for (Common::List<TimerList *>::iterator it = g_vm->_timerLists.begin(); it != g_vm->_timerLists.end(); ++it)
		if ((*it)->getObject() == obj)
			return *it;

	return nullptr;
}

void deleteTimer(Timer *t) {
	g_vm->_timers.remove(t);
}

//----------------------------------------------------------------------
//	Check all active Timers
void checkTimers() {
	for (Common::List<Timer *>::iterator it = g_vm->_timers.begin(); it != g_vm->_timers.end(); it++) {
		if ((*it)->_active == false)
			continue;

		if ((*it)->check()) {
			debugC(2, kDebugTimers, "Timer tick for %p (%s): %p (duration %d)", (void *)(*it)->getObject(), (*it)->getObject()->objName(), (void *)(*it), (*it)->getInterval());
			(*it)->reset();
			(*it)->getObject()->timerTick((*it)->thisID());
		}
	}

	for (Common::List<Timer *>::iterator it = g_vm->_timers.begin(); it != g_vm->_timers.end(); it++) {
		if ((*it)->_active == false) {
			delete *it;
			it = g_vm->_timers.erase(it);
		}
	}
}

//----------------------------------------------------------------------
//	Initialize the Timers

void initTimers() {
	//  Nothing to do
}

static int getTimerListID(TimerList *t) {
	int i = 0;
	for (Common::List<TimerList *>::iterator it = g_vm->_timerLists.begin(); it != g_vm->_timerLists.end(); it++, i++) {
		if ((*it) == t)
			return i;
	}
	return -1;
}

static int getTimerID(Timer *t) {
	int i = 0;
	for (Common::List<Timer *>::iterator it = g_vm->_timers.begin(); it != g_vm->_timers.end(); it++, i++) {
		if ((*it) == t)
			return i;
	}
	return -1;
}

void saveTimers(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Timers");

	int16 timerListCount = 0,
	      timerCount = 0;

	//  Tally the timer lists
	timerListCount = g_vm->_timerLists.size();

	//  Tally the timers
	timerCount = g_vm->_timers.size();

	debugC(3, kDebugSaveload, "... timerListCount = %d", timerListCount);
	debugC(3, kDebugSaveload, "... timerCount = %d", timerCount);

	outS->write("TIMR", 4);
	CHUNK_BEGIN;
	//  Store the timer list count and timer count
	out->writeSint16LE(timerListCount);
	out->writeSint16LE(timerCount);

	//  Archive all timer lists
	for (Common::List<TimerList *>::iterator it = g_vm->_timerLists.begin(); it != g_vm->_timerLists.end(); it++) {
		debugC(3, kDebugSaveload, "Saving TimerList %d", getTimerListID(*it));
		(*it)->write(out);
	}

	for (Common::List<Timer *>::iterator it = g_vm->_timers.begin(); it != g_vm->_timers.end(); it++) {
		if ((*it)->_active == false)
			continue;
		debugC(3, kDebugSaveload, "Saving Timer %d", getTimerID(*it));

		(*it)->write(out);
	}
	CHUNK_END;
}

void loadTimers(Common::InSaveFile *in) {
	debugC(2, kDebugSaveload, "Loading Timers");

	int16 timerListCount,
	      timerCount;

	//  Get the timer list count and timer count
	timerListCount = in->readSint16LE();
	timerCount = in->readSint16LE();

	debugC(3, kDebugSaveload, "... timerListCount = %d", timerListCount);
	debugC(3, kDebugSaveload, "... timerCount = %d", timerCount);

	//  Restore all timer lists
	for (int i = 0; i < timerListCount; i++) {
		debugC(3, kDebugSaveload, "Loading TimerList %d", i);
		new TimerList(in);
	}

	//  Restore all timers
	for (int i = 0; i < timerCount; i++) {
		Timer       *timer;
		TimerList   *timerList;

		debugC(3, kDebugSaveload, "Loading Timer %d", i);

		timer = new Timer(in);

		assert(timer != nullptr);

		//  Get the objects's timer list
		timerList = fetchTimerList(timer->getObject());

		assert(timerList != nullptr);

		//  Append this timer to the objects's timer list
		timerList->_timers.push_back(timer);
	}
}

//----------------------------------------------------------------------
//	Cleanup the active Timers

void cleanupTimers() {
	while (g_vm->_timerLists.size() > 0)
		delete g_vm->_timerLists.front();

	while (g_vm->_timers.size() > 0) {
		Timer *t = g_vm->_timers.front();
		deleteTimer(t);
		delete t;
	}
}

/* ===================================================================== *
   TimerList member functions
 * ===================================================================== */

TimerList::TimerList(GameObject *o) : _obj(o) {
	debugC(1, kDebugTimers, "Creating timer list %p for %p (%s)",
		   (void *)this, (void *)o, o->objName());
	g_vm->_timerLists.push_back(this);
}

TimerList::TimerList(Common::InSaveFile *in) {
	ObjectID id = in->readUint16LE();

	assert(isObject(id) || isActor(id));

	//  Restore the object pointer
	_obj = GameObject::objectAddress(id);

	g_vm->_timerLists.push_back(this);
}

TimerList::~TimerList() {
	debugC(1, kDebugTimers, "Deleting timer list %p for %p (%s))",
		   (void *)this, (void *)_obj, _obj->objName());
	g_vm->_timerLists.remove(this);
}

void TimerList::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store the object's ID
	out->writeUint16LE(_obj->thisID());
}

/* ===================================================================== *
   Timer member functions
 * ===================================================================== */

Timer::Timer(Common::InSaveFile *in) {
	ObjectID id = in->readUint16LE();

	assert(isObject(id) || isActor(id));

	//  Restore the object pointer
	_obj = GameObject::objectAddress(id);

	//  Restore the timer's ID
	_id = in->readSint16LE();

	//  Restore the frame interval
	_interval = in->readSint16LE();

	//  Restore the alarm
	_alarm.read(in);

	_active = true;

	g_vm->_timers.push_back(this);
}

Timer::~Timer() {
	debugC(1, kDebugTimers, "Deleting timer %p (obj %p)",
		   (void *)this, (void *)_obj);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a buffer

int32 Timer::archiveSize() {
	return      sizeof(ObjectID)         //  obj's ID
	            +   sizeof(TimerID)
	            +   sizeof(int16)
	            +   sizeof(FrameAlarm);
}

void Timer::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store the obj's ID
	out->writeUint16LE(_obj->thisID());

	//  Store the timer's ID
	out->writeSint16LE(_id);

	//  Store the frame interval
	out->writeSint16LE(_interval);

	//  Store the alarm
	_alarm.write(out);
}

} // end of namespace Saga2
