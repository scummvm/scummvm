/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef TIMER_H
#define TIMER_H

#include "scummsys.h"

#define MAX_TIMERS 5

class Scumm;

typedef void (*TimerProc)(Scumm *);

#ifdef __MORPHOS__
#include "morphos_timer.h"
#else

class OSystem;

class Timer {

private:
	OSystem *_osystem;
	Scumm *_scumm;
	bool _initialized;
	bool _timerRunning;
	void *_timerHandler;
	int32 _thisTime;
	int32 _lastTime;

	struct TimerSlots {
		TimerProc procedure;
		int32 interval;
		int32 counter;
	} _timerSlots[MAX_TIMERS];

public:
	  Timer(Scumm *system);
	 ~Timer();

	int handler(int *t);
	bool init();
	void release();
	bool installProcedure(TimerProc procedure, int32 interval);
	void releaseProcedure(TimerProc procedure);
};

#endif

#endif
