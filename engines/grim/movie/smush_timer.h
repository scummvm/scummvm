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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GRIM_MOVIE_SMUSH_TIMER_H
#define GRIM_MOVIE_SMUSH_TIMER_H

#include "common/scummsys.h"

namespace Grim {

#define SYS_MAX_TIMERS 19

// Ticks per second of the system timer (the 8253 clock frequency)
#define SMUSH_TIMER_FREQUENCY 1193180

#define SYS_TIMER_ONE_SHOT 2
#define SYS_TIMER_EXT_SYNC 8

typedef void (*SmushTimerCallback)(int timer, void *context);

struct SmushTimerSlot {
	SmushTimerCallback callback;
	int period;
	int flags;
	void *context;
	int reentrant;
	int accumulator;
	int extSync;
};

class SmushTimer {

private:
	SmushTimerSlot _timers[SYS_MAX_TIMERS + 1];
	int _sysGlobalTimer;
	int _sysTickIncrement;
	uint32 _timerThreadLastTime;
	int _sysInitCount;

	void timerTick();

public:
	SmushTimer();
	~SmushTimer();

	int moduleStartup();
	void moduleShutdown();
	void timerThread();

	int sysAlloc(SmushTimerCallback callback, int period, int flags, void *context);
	void sysFree(int timer);
	void sysReset(int timer, int period);
	int sysGetTime();
	void sysExtSync(int value);

};

} // End of namespace Grim
#endif
