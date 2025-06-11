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

#include "graphics/framelimiter.h"

#include "common/util.h"

namespace Graphics {

FrameLimiter::FrameLimiter(OSystem *system, const uint framerate, const bool deferToVsync) :
	_system(system),
	_deferToVsync(deferToVsync) {
	initialize(framerate);
}

void FrameLimiter::initialize() {
	_enabled = (_frameLimit != 0) && !(_deferToVsync && _system->getFeatureState(OSystem::kFeatureVSync));
	_frameDuration = _frameLimit;
}

void FrameLimiter::initialize(const uint framerate) {
	_frameLimit = (framerate > 0) ? 1000.0f / CLIP<uint>(framerate, 1, 100) : 0;
	initialize();
}

uint FrameLimiter::startFrame() {
	_now = _system->getMillis();
	if (_frameStart != 0) {
		_frameDuration = _now - _frameStart;
		_drawDuration = _now - _drawStart;
	}
	_frameStart = _now;
	return _frameDuration;
}

bool FrameLimiter::delayBeforeSwap() {
	_now = _system->getMillis();
	_loopDuration = _now - _frameStart;
	if (_enabled) {
		//_delay = _frameLimit - _loopDuration;  // Original functionality, will tend to undershoot target framerate slightly due to finite screen.update() time.
		_delay = _frameLimit - (_now - _drawStart); // Ensure that EXACTLY the specified frame duration has elapsed since last screen.update() was called.
		if (_delay > 0)
			_system->delayMillis(_delay);
	}
	_drawStart = _system->getMillis();
	return (_delay < 0); // Check if frame is late
}

void FrameLimiter::pause(bool pause) {
	if (!pause)
		_frameStart = 0; // Ensure that the frame duration value is consistent when resuming
}

} // End of namespace Graphics
