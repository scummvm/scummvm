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

#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H

#include "common/scummsys.h"
#include "common/system.h"

#define MAX_TIMERS 5

typedef void (*TimerProc)(void *refCon);

#ifdef __MORPHOS__
#include "morphos_timer.h"
#else

class Timer {

private:
	OSystem *_system;
	OSystem::MutexRef _mutex;
	void *_timerHandler;
	int32 _thisTime;
	int32 _lastTime;

	struct TimerSlots {
		TimerProc procedure;
		int32 interval;
		int32 counter;
		void *refCon;
	} _timerSlots[MAX_TIMERS];

public:
	Timer(OSystem *system);
	~Timer();

	/**
	 * Install a new timer callback. It will from now be called every interval microseconds.
	 * The timer may be invoked from a separate thread. Hence any timer code should be
	 * written following the same safety guidelines as any other threaded code.
	 *
	 * @note Although the interval is specified in microseconds, the actual timer resolution
	 *       may be lower. In particular, with the SDL backend the timer resolution is 10ms.
	 * @param proc		the callback
	 * @param interval	the interval in which the timer shall be invoked (in microseconds)
	 * @param refCon	an arbitrary void pointer; will be passed to the timer callback
	 * @return	true if the timer was installed successfully, false otherwise
	 */
	bool installTimerProc(TimerProc proc, int32 interval, void *refCon);

	/**
	 * Remove the given timer callback. It will not be invoked anymore.
	 */
	void removeTimerProc(TimerProc proc);

protected:
	static int timer_handler(int t);
	int handler(int t);
};

extern Timer *g_timer;

#endif

#endif
