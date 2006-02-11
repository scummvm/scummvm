/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H

#include "common/scummsys.h"
#include "common/mutex.h"

#define MAX_TIMERS 8


#ifdef __MORPHOS__
#include "morphos_timer.h"
#else

class OSystem;

namespace Common {

class Timer {
public:
	typedef void (*TimerProc)(void *refCon);

private:
	OSystem *_system;
	Mutex _mutex;
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

} // End of namespace Common

#endif

#endif
