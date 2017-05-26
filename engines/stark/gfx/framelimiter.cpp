/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/gfx/framelimiter.h"

#include "common/util.h"

namespace Stark {
namespace Gfx {

FrameLimiter::FrameLimiter(OSystem *system, const uint framerate) :
		_system(system),
		_speedLimitMs(0),
		_startFrameTime(0),
		_lastFrameDurationMs(_speedLimitMs) {
	// The frame limiter is disabled when vsync is enabled.
	_enabled = !_system->getFeatureState(OSystem::kFeatureVSync) && framerate != 0;

	if (_enabled) {
		_speedLimitMs = 1000 / CLIP<uint>(framerate, 0, 100);
	}
}

void FrameLimiter::startFrame() {
	uint currentTime = _system->getMillis();

	if (_startFrameTime != 0) {
		_lastFrameDurationMs = currentTime - _startFrameTime;
	}

	_startFrameTime = currentTime;
}

void FrameLimiter::delayBeforeSwap() {
	uint endFrameTime = _system->getMillis();
	uint frameDuration = endFrameTime - _startFrameTime;

	if (_enabled && frameDuration < _speedLimitMs) {
		_system->delayMillis(_speedLimitMs - frameDuration);
	}
}

void FrameLimiter::pause(bool pause) {
	if (!pause) {
		// Make sure the frame duration value is consistent when resuming
		_startFrameTime = 0;
	}
}

uint FrameLimiter::getLastFrameDuration() const {
	return _lastFrameDurationMs;
}

} // End of namespace Gfx
} // End of namespace Stark
