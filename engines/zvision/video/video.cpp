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
#ifdef USE_MPEG2
#include "video/mpegps_decoder.h"
#endif
#include "engines/util.h"
#include "graphics/surface.h"

#include "zvision/zvision.h"
#include "zvision/core/clock.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/subtitles.h"
#include "zvision/video/rlf_decoder.h"
#include "zvision/video/zork_avi_decoder.h"

namespace ZVision {

Video::VideoDecoder *ZVision::loadAnimation(const Common::String &fileName) {
	Common::String tmpFileName = fileName;
	tmpFileName.toLowercase();
	Video::VideoDecoder *animation = NULL;

	if (tmpFileName.hasSuffix(".rlf"))
		animation = new RLFDecoder();
	else if (tmpFileName.hasSuffix(".avi"))
		animation = new ZorkAVIDecoder();
#ifdef USE_MPEG2
	else if (tmpFileName.hasSuffix(".vob"))
		animation = new Video::MPEGPSDecoder();
#endif
	else
		error("Unknown suffix for animation %s", fileName.c_str());

	Common::File *_file = getSearchManager()->openFile(tmpFileName);
	if (!_file)
		error("Error opening %s", tmpFileName.c_str());

	bool loaded = animation->loadStream(_file);
	if (!loaded)
		error("Error loading animation %s", tmpFileName.c_str());

	return animation;
}

void ZVision::playVideo(Video::VideoDecoder &vid, const Common::Rect &destRect, bool skippable, Subtitle *sub) {
	Common::Rect dst = destRect;
	// If destRect is empty, no specific scaling was requested. However, we may choose to do scaling anyway
	if (dst.isEmpty())
		dst = Common::Rect(vid.getWidth(), vid.getHeight());

	Graphics::Surface *scaled = NULL;

	if (vid.getWidth() != dst.width() || vid.getHeight() != dst.height()) {
		scaled = new Graphics::Surface;
		scaled->create(dst.width(), dst.height(), vid.getPixelFormat());
	}

	uint16 x = _workingWindow.left + dst.left;
	uint16 y = _workingWindow.top + dst.top;
	uint16 finalWidth = dst.width() < _workingWindow.width() ? dst.width() : _workingWindow.width();
	uint16 finalHeight = dst.height() < _workingWindow.height() ? dst.height() : _workingWindow.height();
	bool showSubs = (_scriptManager->getStateValue(StateKey_Subtitles) == 1);

	_clock.stop();
	vid.start();
	_videoIsPlaying = true;

	// Only continue while the video is still playing
	while (!shouldQuit() && !vid.endOfVideo() && vid.isPlaying()) {
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
					if (skippable) {
						vid.stop();
					}
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		if (vid.needsUpdate()) {
			const Graphics::Surface *frame = vid.decodeNextFrame();
			if (sub && showSubs)
				sub->process(vid.getCurFrame());

			if (frame) {
				if (scaled) {
					_renderManager->scaleBuffer(frame->getPixels(), scaled->getPixels(), frame->w, frame->h, frame->format.bytesPerPixel, scaled->w, scaled->h);
					frame = scaled;
				}
				Common::Rect rect = Common::Rect(x, y, x + finalWidth, y + finalHeight);
				_renderManager->copyToScreen(*frame, rect, 0, 0);
				_renderManager->processSubs(0);
			}
		}

		// Always update the screen so the mouse continues to render
		_system->updateScreen();

		_system->delayMillis(vid.getTimeToNextFrame() / 2);
	}

	_videoIsPlaying = false;
	_clock.start();

	if (scaled) {
		scaled->free();
		delete scaled;
	}
}

} // End of namespace ZVision
