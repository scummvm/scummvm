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

void Framelimiter::init(bool forceScreenUpdate) {
	reset();
	_timeOfLastPass = _vm->_time->currentSystem();
	_forceScreenUpdate = forceScreenUpdate;
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
		if (_useDelayMs) {
			// _timeOfCurrentPass is used to calculate the duration that the current frame is on screen so far
			uint32 frameDuration = _timeOfCurrentPass - _startFrameTime;
			if (frameDuration < _speedLimitMs) {
				_vm->_system->delayMillis(_speedLimitMs - frameDuration);
				// cheaper than calling _vm->_time->currentSystem() again
				_timeOfCurrentPass += (_speedLimitMs - frameDuration);
			}
		}

		shouldUpdateScreen = ((_timeOfCurrentPass - _timeOfLastPass) >= _speedLimitMs) || _forceScreenUpdate || _useDelayMs;

		if (shouldUpdateScreen) {
			if (_forceScreenUpdate) {
				_forceScreenUpdate = false;
			}
			_lastFrameDurationMs = _timeOfCurrentPass - _startFrameTime;
			_startFrameTime = _timeOfCurrentPass;
		}
	}
	return shouldUpdateScreen;
}

void  Framelimiter::postScreenUpdate() {
	_timeOfLastPass = _timeOfCurrentPass;
//	if (_enabled) {
//		// for debug purposes, this calculates the time between deciding to draw the frame, and the time after drawing the update to the screen
//		uint32 endFrameTime = _vm->_time->currentSystem();
//		uint32 frameDuration = endFrameTime - _startFrameTime;
//	}
}

void Framelimiter::reset() {
	_forceScreenUpdate   = false;
	_timeOfLastPass      = 0u;
	_timeOfCurrentPass   = 0u;
	_startFrameTime      = 0u;
	_lastFrameDurationMs = _speedLimitMs;
}

} // End of namespace BladeRunner
