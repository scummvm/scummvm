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

bool tocapintar = false;
bool tocapintar2 = false;
bool tocapintareffect = false;

ChronoManager::ChronoManager(/* args */) : _lastTick(0), _lastDoubleTick(0) {
}

ChronoManager::~ChronoManager() {
}

void ChronoManager::updateChrono() {
	uint32 currentTime = g_system->getMillis();

	if ((currentTime - _lastTick) >= kFrameMs / _speedMultiplier) {
		tocapintar = true;
		tocapintarTick++;
		if(tocapintarTick == kDoubleFrameMultiplier){
			tocapintarTick = 0;
			tocapintar2 = true;
		}
		else {
			tocapintar2 = false;
		}
		_lastTick = currentTime;
	} else {
		tocapintar = false;
	}

	if ((currentTime - _lastTickEffect) >= kFrameEffectMs) {
		tocapintareffect = true;
		_lastTickEffect = currentTime;
	}
	else {
		tocapintareffect = false;
	}
}

bool ChronoManager::shouldPaintEffect(){
	uint32 currentTime = g_system->getMillis();
	uint32 diff = (currentTime - _lastTickEffect);
	if (diff >= kFrameEffectMs){
		_lastTickEffect = currentTime;
		return true;
	}
	return false;
}

void Tot::ChronoManager::delay(int ms) {
	if(ms > 10) {
		debug("Starting delay of %d", ms);
	}
	uint32 delayStart = g_system->getMillis();
	Common::Event e;
	ms = ms / _speedMultiplier;
	while ((g_system->getMillis() - delayStart) < ms && !g_engine->shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}
		if(ms > 10) {
			g_system->delayMillis(10);
		}
		g_engine->_screen->update();
	}
}

} // End of namespace Tot
