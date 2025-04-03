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
		_deferToVsync(deferToVsync),
		frameStart(0),
		frameLimit(0),
		frameDuration(0),
		drawStart(0),
		drawDuration(0),
		loopDuration(0),
		delay(0) {
  initialize(framerate);
}

void FrameLimiter::initialize(const uint framerate) {
	_enabled =  (framerate != 0) && !(_deferToVsync && _system->getFeatureState(OSystem::kFeatureVSync));
	if (_enabled)
		frameLimit = 1000.0f / CLIP<uint>(framerate, 1, 100);
	frameDuration = frameLimit;
};

uint FrameLimiter::startFrame() {
	now = _system->getMillis();
	if (frameStart != 0) {
	  frameDuration = now - frameStart;
    drawDuration = now - drawStart;
  }
	frameStart = now;
	return frameDuration;
}

bool FrameLimiter::delayBeforeSwap() {
  now = _system->getMillis();
  loopDuration = now - frameStart;
	if(_enabled) {
	  //delay = frameLimit - loopDuration;  //Original functionality, will tend to undershoot target framerate slightly due to finite screen.update() time.
  	delay = frameLimit - (now - drawStart); //Ensure EXACTLY the specified frame duration has elapsed since last screen.update() was called.
    if(delay > 0)
  	  _system->delayMillis(delay);
	}
	drawStart = _system->getMillis();
	return (delay < 0); //Check if frame is late
}

void FrameLimiter::pause(bool pause) {
	if (!pause)
		frameStart = 0; // Ensure the frame duration value is consistent when resuming
}

} // End of namespace Graphics
