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

namespace BladeRunner {

Time::Time(BladeRunnerEngine *vm) {
	_vm = vm;

	_start = _vm->getTotalPlayTime();
	_pauseCount = 0;
	_offset = 0;
	_pauseStart = 0;
}

int Time::current() {
	int time = _vm->getTotalPlayTime() - _offset;
	return time - _start;
}

int Time::pause() {
	if (_pauseCount == 0) {
		_pauseStart = current();
	}
	return ++_pauseCount;
}

int Time::getPauseStart() {
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

} // End of namespace BladeRunner
