/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

static Timer *g_timer = NULL;

Timer::Timer(Engine * engine) :
	_engine(engine),
	_mutex(0),
	_timerHandler(0),
	_lastTime(0) {

	_mutex = _engine->_system->create_mutex();

	g_timer = this;
	
	for (int i = 0; i < MAX_TIMERS; i++) {
		_timerSlots[i].procedure = NULL;
		_timerSlots[i].interval = 0;
		_timerSlots[i].counter = 0;
	}

	_thisTime = _engine->_system->get_msecs();

	// Set the timer last, after everything has been initialised
	_engine->_system->set_timer(10, &timer_handler);

}

Timer::~Timer() {
	_engine->_system->set_timer(0, NULL);

	_engine->_system->lock_mutex(_mutex);
	for (int i = 0; i < MAX_TIMERS; i++) {
		_timerSlots[i].procedure = NULL;
		_timerSlots[i].interval = 0;
		_timerSlots[i].counter = 0;
	}
	_engine->_system->unlock_mutex(_mutex);

	// FIXME: There is still a potential race condition here, depending on how
	// the system backend implements set_timer: If timers are done using
	// threads, and if set_timer does *not* gurantee that after it terminates
	// that timer thread is not run anymore, we are fine. However, if the timer
	// is still running in parallel to this destructor, then it might be that
	// it is still waiting for the _mutex. So, again depending on the backend,
	// we might end up unlocking the mutex then immediately deleting it, while
	// the timer thread is about to lock it.
	_engine->_system->delete_mutex(_mutex);
}

int Timer::timer_handler(int t) {
	if (g_timer)
		return g_timer->handler(t);
	return 0;
}

int Timer::handler(int t) {
	uint32 interval, l;

	_engine->_system->lock_mutex(_mutex);

	_lastTime = _thisTime;
	_thisTime = _engine->_system->get_msecs();
	interval = 1000 * (_thisTime - _lastTime);

	for (l = 0; l < MAX_TIMERS; l++) {
		if ((_timerSlots[l].procedure) && (_timerSlots[l].interval > 0)) {
			_timerSlots[l].counter -= interval;
			if (_timerSlots[l].counter <= 0) {
				_timerSlots[l].counter += _timerSlots[l].interval;
				_timerSlots[l].procedure(_engine);
			}
		}
	}

	_engine->_system->unlock_mutex(_mutex);

	return t;
}

bool Timer::installProcedure (TimerProc procedure, int32 interval) {
	int32 l;
	bool found = false;

	_engine->_system->lock_mutex(_mutex);
	for (l = 0; l < MAX_TIMERS; l++) {
		if (!_timerSlots[l].procedure) {
			_timerSlots[l].procedure = procedure;
			_timerSlots[l].interval = interval;
			_timerSlots[l].counter = interval;
			found = true;
			break;
		}
	}
	_engine->_system->unlock_mutex(_mutex);

	if (!found)
		warning("Couldn't find free timer slot!");

	return found;
}

void Timer::releaseProcedure (TimerProc procedure) {
	int32 l;

	_engine->_system->lock_mutex(_mutex);
	for (l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure == procedure) {
			_timerSlots[l].procedure = 0;
			_timerSlots[l].interval = 0;
			_timerSlots[l].counter = 0;
		}
	}
	_engine->_system->unlock_mutex(_mutex);
}

#endif
