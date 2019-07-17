/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

#include "bladerunner/time.h"

#include "bladerunner/bladerunner.h"

#include "common/timer.h"
#include "common/system.h"

namespace BladeRunner {

Time::Time(BladeRunnerEngine *vm) {
	_vm = vm;

	_start = currentSystem();
	_pauseCount = 0;
	_offset = 0u;
	_pauseStart = 0u;
}

uint32 Time::currentSystem() {
	return _vm->getTotalPlayTime();
}

uint32 Time::current() {
	uint32 time = currentSystem() - _offset;
	return time - _start;
}

int Time::pause() {
	if (_pauseCount == 0) {
		_pauseStart = current();
	}
	return ++_pauseCount;
}

uint32 Time::getPauseStart() {
	return _pauseStart;
}

int Time::resume() {
	assert(_pauseCount > 0);
	if (--_pauseCount == 0) {
		_offset += current() - _pauseStart;
	}
	return _pauseCount;
}

bool Time::isLocked() {
	return _pauseCount > 0;
}

// To be called before loading a new game, since
// the offset should be reset to zero and _pauseStart should be current() (ie currentSystem() - _start)
// TODO Explore if it would make sense to only use the Engine methods for time accounting (pauseEngine, get/setTotalPlatTime)
//      or do we need separated/independent time accounting and pausing?
void Time::resetPauseStart() {
	_offset = 0u;
	_pauseStart = current();
}

} // End of namespace BladeRunner
