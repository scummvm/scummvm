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

#include "bbvs/bbvs.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace Bbvs {

void BbvsEngine::playVideo(int videoNum) {
	Common::String videoFilename;

	if (videoNum >= 100)
		videoFilename = Common::String::format("snd/snd%05d.aif", videoNum + 1400);
	else
		videoFilename = Common::String::format("vid/video%03d.avi", videoNum - 1);

	// Set the correct video mode
	initGraphics(320, 240, false, 0);
	if (_system->getScreenFormat().bytesPerPixel == 1) {
		warning("Couldn't switch to a RGB color video mode to play a video.");
		return;
	}
		
	Video::VideoDecoder *videoDecoder = new Video::AVIDecoder();
	if (!videoDecoder->loadFile(videoFilename)) {
		delete videoDecoder;
		warning("Unable to open video %s", videoFilename.c_str());
		return;
	}

	videoDecoder->start();

	bool skipVideo = false;

	while (!shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
				_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		_system->delayMillis(10);
	}

	delete videoDecoder;
	
	initGraphics(320, 240, false);

}

} // End of namespace Bbvs
