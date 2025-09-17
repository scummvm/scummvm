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

#include "tot/chrono.h"
#include "tot/tot.h"

namespace Tot {

ChronoManager::ChronoManager(/* args */) : _lastTick(0) {
}

ChronoManager::~ChronoManager() {
}

void ChronoManager::updateChrono() {
	uint32 currentTime = g_system->getMillis();

	if ((currentTime - _lastTick) >= kTickMs / _speedMultiplier) {
		_gameTick = true;
		_tickCount++;
		if (_tickCount == kHalfTickMultiplier) {
			_tickCount = 0;
			_gameTickHalfSpeed = true;
		} else {
			_gameTickHalfSpeed = false;
		}
		_lastTick = currentTime;
	} else {
		_gameTick = false;
	}
}

void ChronoManager::changeSpeed() {
	if (_speedMultiplier == 1)
		_speedMultiplier = 4;
	else
		_speedMultiplier = 1;
}

void ChronoManager::delay(uint32 ms) {
	uint32 delayStart = g_system->getMillis();
	Common::Event e;
	ms = ms / _speedMultiplier;
	while ((g_system->getMillis() - delayStart) < ms && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		g_engine->_screen->update();
	}
}

} // End of namespace Tot
