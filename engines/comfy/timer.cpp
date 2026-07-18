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

#include "comfy/comfy.h"

namespace Comfy {

void ComfyEngine::timerInit() {
	if (_timerInitialized)
		return;

	_timerPitPhase = 0;
	_timerLastMillis = g_system->getMillis();
	_timerInitialized = true;
}

void ComfyEngine::timerShutdown() {
	if (!_timerInitialized)
		return;

	_timerInitialized = false;
}

uint16 ComfyEngine::timerTick() {
	uint32 now = g_system->getMillis();
	uint32 elapsedMillis = now - _timerLastMillis;

	_timerLastMillis = now;
	_timerPitPhase += (uint64)elapsedMillis * 1000 * COMFY_PIT_INPUT_FREQUENCY;

	uint64 pitPeriod = (uint64)COMFY_PIT_TIMER_DIVISOR * 1000000;
	uint32 callbacks = (uint32)(_timerPitPhase / pitPeriod);
	_timerPitPhase %= pitPeriod;

	while (callbacks--) {
		if (shouldQuit())
			return 0;
	}

	_timer2 = _timer1;
	_timer1 = _timer0;
	_timer0 = _timerCurrent;
	_timerCurrent = (int16)midiTick();

	return (uint16)_timerCurrent;
}

} // End of namespace Comfy
