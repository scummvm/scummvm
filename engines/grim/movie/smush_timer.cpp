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

#include "common/system.h"

#include "engines/grim/movie/smush_timer.h"

namespace Grim {

SmushTimer::SmushTimer() {
	memset(_timers, 0, sizeof(_timers));
	_sysGlobalTimer = 0;
	_sysTickIncrement = 0;
	_timerThreadLastTime = 0;
	_sysInitCount = 0;
}

SmushTimer::~SmushTimer() {
}

int SmushTimer::moduleStartup() {
	if (_sysInitCount++ == 0) {
		for (int i = 1; i <= SYS_MAX_TIMERS; i++)
			_timers[i].callback = nullptr;

		_sysTickIncrement = 19886;
		_timerThreadLastTime = g_system->getMillis();
	}

	return 0;
}

void SmushTimer::moduleShutdown() {
	_sysInitCount--;
}

void SmushTimer::timerThread() {
	if (!_sysInitCount)
		return;

	// The host timer exists only after the first movie, don't replay the idle time as ticks
	uint32 now = g_system->getMillis();
	if (now - _timerThreadLastTime > 1000)
		_timerThreadLastTime = now;

	int elapsed = now - _timerThreadLastTime;
	if (elapsed > 16) {
		_timerThreadLastTime += 16;
		timerTick();
	}
}

void SmushTimer::timerTick() {
	_sysGlobalTimer += _sysTickIncrement;

	for (int i = 1; i <= SYS_MAX_TIMERS; i++) {
		SmushTimerSlot *slot = &_timers[i];
		if (!slot->callback)
			continue;

		slot->accumulator += _sysTickIncrement;
		slot->extSync += _sysTickIncrement;
	}

	for (int i = 1; i <= SYS_MAX_TIMERS; i++) {
		SmushTimerSlot *slot = &_timers[i];
		if (!slot->callback || !slot->period || slot->reentrant)
			continue;

		slot->reentrant = 1;
		if (slot->accumulator >= slot->period) {
			while (true) {
				slot->accumulator -= slot->period;
				if (slot->flags & SYS_TIMER_ONE_SHOT) {
					slot->period = 0;
					slot->callback(i, slot->context);
					break;
				}

				slot->callback(i, slot->context);
				if (slot->accumulator < slot->period)
					break;
			}
		}
		slot->reentrant = 0;
	}
}

int SmushTimer::sysAlloc(SmushTimerCallback callback, int period, int flags, void *context) {
	int i = 1;
	while (_timers[i].callback) {
		i++;
		if (i > SYS_MAX_TIMERS)
			return 0;
	}

	SmushTimerSlot *slot = &_timers[i];
	slot->reentrant = 0;
	slot->accumulator = 0;
	slot->context = context;
	slot->flags = flags;
	slot->period = period;
	slot->callback = callback;
	slot->extSync = 0;

	return i;
}

void SmushTimer::sysFree(int timer) {
	if (timer > 0 && timer <= SYS_MAX_TIMERS)
		_timers[timer].callback = nullptr;
}

void SmushTimer::sysReset(int timer, int period) {
	if (timer > 0 && timer <= SYS_MAX_TIMERS) {
		_timers[timer].period = period;
		_timers[timer].accumulator = 0;
	}
}

int SmushTimer::sysGetTime() {
	return _sysGlobalTimer;
}

void SmushTimer::sysExtSync(int value) {
	for (int i = 1; i <= SYS_MAX_TIMERS; i++) {
		SmushTimerSlot *slot = &_timers[i];
		if (slot->callback && (slot->flags & SYS_TIMER_EXT_SYNC)) {
			if (value > 0)
				slot->accumulator += value - slot->extSync;

			slot->extSync = 0;
		}
	}
}

} // End of namespace Grim
