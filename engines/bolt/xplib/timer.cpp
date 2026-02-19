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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

bool XpLib::initTimer() {
	for (int i = 0; i < ARRAYSIZE(g_timers); i++)
		g_timers[i].active = false;

	g_nextTimerId = 1;
	g_timerInitialized = true;

	return true;
}

void XpLib::shutdownTimer() {
	if (!g_timerInitialized) {
		g_timerInitialized = false;

		for (int i = 0; i < ARRAYSIZE(g_timers); i++) {
			g_timers[i].active = false;
		}
	}
}

uint32 XpLib::startTimer(int16 delay) {
	uint32 now = _bolt->_system->getMillis();

	for (int i = 0; i < ARRAYSIZE(g_timers); i++) {
		if (!g_timers[i].active) {
			g_timers[i].id = g_nextTimerId++;

			if (g_nextTimerId == 0)
				g_nextTimerId = 1;

			g_timers[i].deadline = now + delay;
			g_timers[i].active = true;
			return g_timers[i].id;
		}
	}

	return 0;
}

void XpLib::updateTimers() {
	uint32 now = _bolt->_system->getMillis();

	for (int i = 0; i < ARRAYSIZE(g_timers); i++) {
		if (g_timers[i].active && now >= g_timers[i].deadline) {
			g_timers[i].active = false;
			handleTimer(g_timers[i].id);
		}
	}
}

bool XpLib::killTimer(uint32 timerId) {
	for (int i = 0; i < ARRAYSIZE(g_timers); i++) {
		if (g_timers[i].active && g_timers[i].id == timerId) {
			g_timers[i].active = false;
			return true;
		}
	}

	return false;
}

} // End of namespace Bolt
