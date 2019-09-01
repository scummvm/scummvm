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

#include "bladerunner/framelimiter.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/time.h"
#include "common/system.h"

namespace BladeRunner {

Framelimiter::Framelimiter(BladeRunnerEngine *vm, FramelimiterFpsRate framerateMode, bool useDelayMs) {
	_vm = vm;

//	// FUTURE: The frame limiter is disabled when vsync is enabled.
//	_enabled = !_system->getFeatureState(OSystem::kFeatureVSync);
	_enabled         = true;
	_useDelayMs      = useDelayMs;

	_speedLimitMs    = 0u;
	uint32 framerate = 1u; // dummy init
	switch (framerateMode) {
	case kFramelimiter15fps:
		framerate =  15u;
		break;
	case kFramelimiter25fps:
		framerate =  25u;
		break;
	case kFramelimiter30fps:
		framerate =  30u;
		break;
	case kFramelimiter60fps:
		framerate =  60u;
		break;
	case kFramelimiter120fps:
		framerate = 120u;
		break;
	case kFramelimiterDisabled:
		// fall through
	default:
		_enabled = false;
		break;
	}

	if (_enabled) {
		_speedLimitMs = 1000 / CLIP<uint32>(framerate, 1, 120);
	}

	reset();
}

Framelimiter::~Framelimiter() { }

void Framelimiter::init(bool forceFirstPass) {
	reset();
	_timeOfLastPass = _vm->_time->currentSystem();
	_forceFirstPass = forceFirstPass;
}

uint32 Framelimiter::getLastFrameDuration() const {
	return _lastFrameDurationMs;
}

uint32 Framelimiter::getTimeOfCurrentPass() const {
	return _timeOfCurrentPass;
}

uint32 Framelimiter::getTimeOfLastPass() const {
	return _timeOfLastPass;
}

bool Framelimiter::shouldExecuteScreenUpdate() {
	bool shouldUpdateScreen = true;
	_timeOfCurrentPass = _vm->_time->currentSystem();
	if (_enabled) {
		shouldUpdateScreen = ((_timeOfCurrentPass - _timeOfLastPass) >= _speedLimitMs) || _forceFirstPass;

		if (shouldUpdateScreen) {
			if (_forceFirstPass) {
				_forceFirstPass = false;
			}
			_startFrameTime = _timeOfCurrentPass;
		}
	}
	return shouldUpdateScreen;
}

void  Framelimiter::postScreenUpdate() {
	_timeOfLastPass = _timeOfCurrentPass;
	if (_enabled) {

		if (_useDelayMs) {
			uint32 endFrameTime = _vm->_time->currentSystem();
			uint32 frameDuration = endFrameTime - _startFrameTime;

			if (frameDuration < _speedLimitMs) {
				_vm->_system->delayMillis(_speedLimitMs - frameDuration);
			}
		}
	}
}

void Framelimiter::reset() {
	_forceFirstPass      = false;
	_timeOfLastPass      = 0u;
	_timeOfCurrentPass   = 0u;
	_startFrameTime      = 0u;
	_lastFrameDurationMs = _speedLimitMs;
}

} // End of namespace BladeRunner
