/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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

#ifndef LIBRETRO_TIMER_H
#define LIBRETRO_TIMER_H

// Thread switch caller
#define THREAD_SWITCH_POLL              (1 << 0)
#define THREAD_SWITCH_DELAY             (1 << 1)
#define THREAD_SWITCH_UPDATE            (1 << 2)

#include "backends/timer/default/default-timer.h"
#include "backends/platform/libretro/include/libretro-defs.h"

class LibretroTimerManager : public DefaultTimerManager {
	uint32 _interval;
	uint32 _nextSwitchTime;
	uint32 _spentOnMainThread;
	uint8 _threadSwitchCaller;
public:
	LibretroTimerManager(uint32 refresh_rate);
	~LibretroTimerManager(void) {};
	void switchThread(uint8 caller = 0);
	bool checkThread(uint8 caller = 0);
	uint32 timeToNextSwitch(void);
	uint32 spentOnMainThread(void);
	uint8 getThreadSwitchCaller(void);
};

#endif // LIBRETRO_TIMER_H
