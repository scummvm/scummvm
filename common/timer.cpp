/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef __MORPHOS__

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/timer.h"
#include "common/util.h"

Timer *g_timer = NULL;

Timer::Timer(OSystem *system) :
	_system(system),
	_mutex(0),
	_timerHandler(0),
	_lastTime(0) {

	_mutex = _system->create_mutex();

	g_timer = this;

	for (int i = 0; i < MAX_TIMERS; i++) {
		_timerSlots[i].procedure = NULL;
		_timerSlots[i].interval = 0;
		_timerSlots[i].counter = 0;
	}

	_thisTime = _system->get_msecs();

	// Set the timer last, after everything has been initialised
	_system->set_timer(&timer_handler, 10);

}

Timer::~Timer() {
	_system->set_timer(0, 0);

	{
		Common::StackLock lock(_mutex);
		for (int i = 0; i < MAX_TIMERS; i++) {
			_timerSlots[i].procedure = NULL;
			_timerSlots[i].interval = 0;
			_timerSlots[i].counter = 0;
		}
	}


	// FIXME: There is still a potential race condition here, depending on how
	// the system backend implements set_timer: If timers are done using
	// threads, and if set_timer does *not* gurantee that after it terminates
	// that timer thread is not run anymore, we are fine. However, if the timer
	// is still running in parallel to this destructor, then it might be that
	// it is still waiting for the _mutex. So, again depending on the backend,
	// we might end up unlocking the mutex then immediately deleting it, while
	// the timer thread is about to lock it.
	_system->delete_mutex(_mutex);
}

int Timer::timer_handler(int t) {
	if (g_timer)
		return g_timer->handler(t);
	return 0;
}

int Timer::handler(int t) {
	Common::StackLock lock(_mutex);
	uint32 interval, l;

	_lastTime = _thisTime;
	_thisTime = _system->get_msecs();
	interval = 1000 * (_thisTime - _lastTime);

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
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (!_timerSlots[l].procedure) {
			_timerSlots[l].procedure = procedure;
			_timerSlots[l].interval = interval;
			_timerSlots[l].counter = interval;
			_timerSlots[l].refCon = refCon;
			return true;
		}
	}

	warning("Couldn't find free timer slot!");
	return false;
}

void Timer::removeTimerProc(TimerProc procedure) {
	Common::StackLock lock(_mutex);

	for (int l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure == procedure) {
			_timerSlots[l].procedure = 0;
			_timerSlots[l].interval = 0;
			_timerSlots[l].counter = 0;
			_timerSlots[l].refCon = 0;
		}
	}
}

#endif
