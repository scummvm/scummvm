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

#if defined(__LIBRETRO__)
#include "common/scummsys.h"
#include "common/timer.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-threads.h"
#include "backends/platform/libretro/include/libretro-timer.h"
#include "backends/platform/libretro/include/libretro-defs.h"

LibretroTimerManager::LibretroTimerManager(uint32 refresh_rate) {
	_interval = 1000 / refresh_rate / 2;
	_nextSwitchTime = _interval + g_system->getMillis();
}

void LibretroTimerManager::switchThread(uint8 caller) {
	_spentOnMainThread = g_system->getMillis();
	_threadSwitchCaller = caller;
	LIBRETRO_G_SYSTEM->refreshScreen();
	retro_switch_to_main_thread();
	_spentOnMainThread = g_system->getMillis() - _spentOnMainThread;
	_nextSwitchTime =  g_system->getMillis() + _interval;
	handler();
}

bool LibretroTimerManager::checkThread(uint8 caller) {
	if (g_system->getMillis() >= _nextSwitchTime) {
		switchThread(caller);
		return true;
	} else
		return false;
}

uint32 LibretroTimerManager::timeToNextSwitch(void) {
	uint32 now = g_system->getMillis();
	return _nextSwitchTime > now ? _nextSwitchTime - now : 0;
}

uint32 LibretroTimerManager::spentOnMainThread(void) {
	return _spentOnMainThread;
}

uint8 LibretroTimerManager::getThreadSwitchCaller(void) {
	return _threadSwitchCaller;
}
#endif
