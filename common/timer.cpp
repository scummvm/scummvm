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
#include "scummsys.h"
#include "timer.h"

static Timer *g_timer = NULL;

Timer::Timer(Engine * engine) {
	memset(this,0,sizeof(Timer)); //palmos
	
	_initialized = false;
	_timerRunning = false;
	_engine = engine;
	g_timer = this;
}

Timer::~Timer() {
	release();
}

int Timer::timer_handler(int t) {
	if (g_timer)
		return g_timer->handler(t);
	return 0;
}

int Timer::handler(int t) {
	uint32 interval, l;

	if (_timerRunning) {
		_lastTime = _thisTime;
		_thisTime = _engine->_system->get_msecs();
		interval = 1000 * (_thisTime - _lastTime);

		for (l = 0; l < MAX_TIMERS; l++) {
			if ((_timerSlots[l].procedure) && (_timerSlots[l].interval > 0)) {
				_timerSlots[l].counter -= interval;
				if (_timerSlots[l].counter <= 0) {
					_timerSlots[l].counter += _timerSlots[l].interval;
					_timerSlots[l].procedure (_engine);
				}
			}
		}
	}

	return t;
}

bool Timer::init() {
	int32 l;

	if (_engine->_system == NULL) {
		warning("Timer: OSystem not initialized!");
		return false;
	}

	if (_initialized) 
		return true;

	for (l = 0; l < MAX_TIMERS; l++) {
		_timerSlots[l].procedure = NULL;
		_timerSlots[l].interval = 0;
		_timerSlots[l].counter = 0;
	}

	_thisTime = _engine->_system->get_msecs();
	_engine->_system->set_timer(10, &timer_handler);

	_timerRunning = true;
	_initialized = true;
	return true;
}

void Timer::release() {
	int32 l;

	if (_initialized == false) 
		return;

	_timerRunning = false;
	_engine->_system->set_timer(0, NULL);
	_initialized = false;

	for (l = 0; l < MAX_TIMERS; l++) {
		_timerSlots[l].procedure = NULL;
		_timerSlots[l].interval = 0;
		_timerSlots[l].counter = 0;
	}
}

bool Timer::installProcedure (TimerProc procedure, int32 interval) {
	int32 l;
	bool found = false;

	if (_initialized == false) {
		warning("Timer: is not initialized!");
		return false;
	}

	_timerRunning = false;
	for (l = 0; l < MAX_TIMERS; l++) {
		if (!_timerSlots[l].procedure) {
			_timerSlots[l].procedure = procedure;
			_timerSlots[l].interval = interval;
			_timerSlots[l].counter = interval;
			found = true;
			break;
		}
	}

	_timerRunning = true;
	if (!found)	{
		warning("Can't find free slot!");
		return false;
	}

	return true;
}

void Timer::releaseProcedure (TimerProc procedure) {
	int32 l;

	if (_initialized == false) {
		warning("Timer: is not initialized!");
		return;
	}

	_timerRunning = false;
	for (l = 0; l < MAX_TIMERS; l++) {
		if (_timerSlots[l].procedure == procedure) {
			_timerSlots[l].procedure = 0;
			_timerSlots[l].interval = 0;
			_timerSlots[l].counter = 0;
		}
	}
	_timerRunning = true;
}

#endif
