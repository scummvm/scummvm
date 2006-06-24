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

#ifndef BACKENDS_TIMER_DEFAULT_H
#define BACKENDS_TIMER_DEFAULT_H

#include "common/timer.h"
#include "common/mutex.h"

class OSystem;

class DefaultTimerManager : public Common::TimerManager {
private:
	enum {
		MAX_TIMERS = 8
	};
	OSystem *_system;
	Common::Mutex _mutex;
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
	DefaultTimerManager(OSystem *system);
	~DefaultTimerManager();
	bool installTimerProc(TimerProc proc, int32 interval, void *refCon);
	void removeTimerProc(TimerProc proc);

protected:
	static int timer_handler(int t);
	int handler(int t);
};

#endif
