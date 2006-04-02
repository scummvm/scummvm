/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "timer.h"
#include "driver.h"

Timer *g_timer = NULL;

Timer::Timer() :
	_mutex(0),
	_timerHandler(0),
	_lastTime(0) {

	_mutex = g_driver->createMutex();

	g_timer = this;

	for (int i = 0; i < MAX_TIMERS; i++) {
		_timerSlots[i].procedure = NULL;
		_timerSlots[i].interval = 0;
		_timerSlots[i].counter = 0;
	}

	_thisTime = g_driver->getMillis();

	// Set the timer last, after everything has been initialised
	g_driver->setTimerCallback(timer_handler, 10);
}

Timer::~Timer() {
	g_driver->setTimerCallback(NULL, 0);

	{
		StackLock lock(_mutex);
		for (int i = 0; i < MAX_TIMERS; i++) {
			_timerSlots[i].procedure = NULL;
			_timerSlots[i].interval = 0;
			_timerSlots[i].counter = 0;
		}
	}

	g_driver->deleteMutex(_mutex);
}

int Timer::timer_handler(int t) {
	if (g_timer)
		return g_timer->handler(t);
	return 0;
}

int Timer::handler(int t) {
	StackLock lock(_mutex);
	uint32 interval, l;

	_lastTime = _thisTime;
	_thisTime = g_driver->getMillis();
	interval = 1000 * (_thisTime - _lastTime);

	// If the timer has been frozen for a long time, don't
	// call the procedures a silly number of times, just resynchronize
	if(interval > 1000000) {
		interval = 0;
		warning("Timer skipped forward");
	}

	for (l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure && _timerSlots[l].interval > 0) {
			_timerSlots[l].counter -= interval;
			while (_timerSlots[l].counter <= 0) {
				// A small paranoia check which catches the case where
				// a timer removes itself (which it never should do).
				assert(_timerSlots[l].procedure && _timerSlots[l].interval > 0);
				_timerSlots[l].counter += _timerSlots[l].interval;
				_timerSlots[l].procedure(_timerSlots[l].refCon);
			}
		}
	}

	return t;
}

bool Timer::installTimerProc(TimerProc procedure, int32 interval, void *refCon) {
	assert(interval > 0);
	StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (!_timerSlots[l].procedure) {
			_timerSlots[l].procedure = procedure;
			_timerSlots[l].interval = interval;
			_timerSlots[l].counter = interval;
			_timerSlots[l].refCon = refCon;
			return true;
		}
	}

	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("Couldn't find free timer slot!");
	return false;
}

void Timer::removeTimerProc(TimerProc procedure) {
	StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure == procedure) {
			_timerSlots[l].procedure = 0;
			_timerSlots[l].interval = 0;
			_timerSlots[l].counter = 0;
			_timerSlots[l].refCon = 0;
		}
	}
}
