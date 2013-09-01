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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "common/scummsys.h"

#include "common/system.h"
#include "video/video_decoder.h"

#include "zvision/zvision.h"
#include "zvision/rlf_animation.h"


namespace ZVision {

void ZVision::playAnimation(RlfAnimation *animation, uint16 x, uint16 y, DisposeAfterUse::Flag disposeAfterUse) {
	bool skip = false;
	uint32 frameTime = animation->frameTime();
	uint width = animation->width();
	uint height = animation->height();

	uint16 newX = x + _workingWindow.left;
	uint16 newY = y + _workingWindow.top;

	uint32 accumulatedTime = 0;

	// Only continue while the video is still playing
	while (!shouldQuit() && !skip && !animation->endOfAnimation()) {
		_clock.update();
		uint32 currentTime = _clock.getLastMeasuredTime();
		accumulatedTime += _clock.getDeltaTime();

		// Check for engine quit and video stop key presses
		while (_eventMan->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_KEYDOWN:
				switch (_event.kbd.keycode) {
				case Common::KEYCODE_q:
					if (_event.kbd.hasFlags(Common::KBD_CTRL))
						quitGame();
					break;
				case Common::KEYCODE_SPACE:
					skip = true;
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		while (accumulatedTime >= frameTime && !animation->endOfAnimation()) {
			accumulatedTime -= frameTime;

			_system->copyRectToScreen(animation->getNextFrame(), width * sizeof(uint16), newX, newY, width, height);
		}

		// Always update the screen so the mouse continues to render
		_system->updateScreen();

		// Calculate the frame delay based off a desired frame time
		int delay = _desiredFrameTime - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
	}
}

void ZVision::playAnimation(Video::VideoDecoder *animation, uint16 x, uint16 y, DisposeAfterUse::Flag disposeAfterUse) {
	_clock.stop();
	animation->start();

	// Only continue while the video is still playing
	while (!shouldQuit() && !animation->endOfVideo() && animation->isPlaying()) {
		// Check for engine quit and video stop key presses
		while (!animation->endOfVideo() && animation->isPlaying() && _eventMan->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_KEYDOWN:
				switch (_event.kbd.keycode) {
				case Common::KEYCODE_q:
					if (_event.kbd.hasFlags(Common::KBD_CTRL))
						quitGame();
					break;
				case Common::KEYCODE_SPACE:
					animation->stop();
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		if (animation->needsUpdate()) {
			const Graphics::Surface *frame = animation->decodeNextFrame();

			if (frame) {
				_system->copyRectToScreen((const byte *)frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
			}
		}

		// Always update the screen so the mouse continues to render
		_system->updateScreen();

		_system->delayMillis(animation->getTimeToNextFrame());
	}

	_clock.start();
}

} // End of namespace ZVision
