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

#include "prince/prince.h"
#include "engines/util.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace Prince {

void PrinceEngine::playVideo(const Common::Path &videoFilename) {
		// Set the correct video mode
		initGraphics(640, 480, nullptr);
		if (_system->getScreenFormat().bytesPerPixel == 1) {
				warning("Couldn't switch to a RGB color video mode to play a video.");
				return;
		}

		debug(2, "Screen format: %s", _system->getScreenFormat().toString().c_str());

	Video::VideoDecoder *videoDecoder = new Video::AVIDecoder();
	if (!videoDecoder->loadFile(videoFilename)) {
		delete videoDecoder;
		warning("Unable to open video %s", videoFilename.toString().c_str());
		initGraphics(640, 480);
		return;
	}

	videoDecoder->start();

	bool skipVideo = false;

	while (!shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				if (frame->format.bytesPerPixel > 1) {
					Graphics::Surface *frame1 = frame->convertTo(_system->getScreenFormat());
					_system->copyRectToScreen(frame1->getPixels(), frame1->pitch, 0, 0, frame1->w, frame1->h);
					frame1->free();
					delete frame1;
				} else {
					_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
				}
				_system->updateScreen();
			}
		}

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		_system->delayMillis(10);
	}

	delete videoDecoder;

	initGraphics(640, 480);
}

} // End of namespace Prince
