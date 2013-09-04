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

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/video_window.h"

#include "common/system.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace Buried {

VideoWindow::VideoWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent), _video(0), _mode(kModeClosed), _lastFrame(0) {
	_vm->addVideo(this);
	_needsPalConversion = false;
	_ownedFrame = 0;
}

VideoWindow::~VideoWindow() {
	closeVideo();
	_vm->removeVideo(this);
}

bool VideoWindow::playVideo() {
	if (!_video)
		return false;

	if (_video->isPlaying())
		return true;

	_video->start();
	_mode = kModePlaying;
	return true;
}

bool VideoWindow::playToFrame(int frame) {
	if (!_video)
		return false;

	_video->setEndFrame(frame);

	if (_video->isPlaying())
		return true;

	_video->start();
	_mode = kModePlaying;
	return true;
}

bool VideoWindow::seekToFrame(int frame) {
	if (!_video)
		return false;

	return _video->seekToFrame(frame);
}

void VideoWindow::stopVideo() {
	if (_video) {
		_video->stop();
		_mode = kModeStopped;
	}
}

int VideoWindow::getCurFrame() {
	if (_video)
		return _video->getCurFrame() + 1;

	return -1;
}

int VideoWindow::getFrameCount() {
	if (_video)
		return _video->getFrameCount();

	return 0;
}

bool VideoWindow::openVideo(const Common::String &fileName) {
	closeVideo();

	_video = new Video::AVIDecoder();

	if (!_video->loadFile(fileName)) {
		closeVideo();
		return false;
	}

	if (!_vm->isTrueColor()) {
		Graphics::PixelFormat videoFormat = _video->getPixelFormat();

		if (videoFormat.bytesPerPixel == 1) {
			_needsPalConversion = true;
		} else {
			_video->setDitheringPalette(_vm->_gfx->getDefaultPalette());
			_needsPalConversion = false;
		}
	}

	_mode = kModeOpen;
	_rect = Common::Rect(_video->getWidth(), _video->getHeight());
	return true;
}

void VideoWindow::closeVideo() {
	if (_video) {
		delete _video;
		_video = 0;
		_mode = kModeClosed;
		_lastFrame = 0;
		_rect = Common::Rect();

		if (_ownedFrame) {
			_ownedFrame->free();
			delete _ownedFrame;
			_ownedFrame = 0;
		}
	}
}

void VideoWindow::updateVideo() {
	if (_video) {
		if (_video->needsUpdate()) {
			// Store the frame for later
			const Graphics::Surface *frame = _video->decodeNextFrame();
			if (frame) {
				if (_ownedFrame) {
					_ownedFrame->free();
					delete _ownedFrame;
					_ownedFrame = 0;
				}

				if (_vm->isTrueColor()) {
					// Convert to the screen format if necessary
					if (frame->format == g_system->getScreenFormat()) {
						_lastFrame = frame;
					} else {
						_ownedFrame = frame->convertTo(g_system->getScreenFormat(), _video->getPalette());
						_lastFrame = _ownedFrame;
					}
				} else {
					if (_needsPalConversion) {
						// If it's a palette video, ensure it's using the screen palette
						_ownedFrame = remapPalettedFrame(frame, _video->getPalette());
						_lastFrame = _ownedFrame;
					} else {
						// Assume it's in the right format from dithering
						_lastFrame = frame;
					}
				}
			}

			// Invalidate the window so it gets updated
			invalidateWindow();
		}

		if (_video->isPlaying() && _video->endOfVideo()) {
			_video->stop();
			_mode = kModeStopped;
		}
	}
}

void VideoWindow::onPaint() {
	if (_lastFrame)
		_vm->_gfx->blit(_lastFrame, _rect.left, _rect.top);
}

Graphics::Surface *VideoWindow::remapPalettedFrame(const Graphics::Surface *frame, const byte *palette) {
	// This is pretty much the same as the Cinepak one
	// It seems to work for the one video I know that needs it (SWLOGO.BTV)

	byte palMap[256];
	const byte *screenPal = _vm->_gfx->getDefaultPalette();

	for (int i = 0; i < 256; i++) {
		int r = palette[i * 3];
		int g = palette[i * 3 + 1];
		int b = palette[i * 3 + 2];

		int diff = 0x7FFFFFFF;
		byte result = 0;

		for (int j = 0; j < 256; j++) {
			int bDiff = b - (int)screenPal[j * 3 + 2];
			int curDiffB = diff - (bDiff * bDiff);

			if (curDiffB > 0) {
				int gDiff = g - (int)screenPal[j * 3 + 1];
				int curDiffG = curDiffB - (gDiff * gDiff);

				if (curDiffG > 0) {
					int rDiff = r - (int)screenPal[j * 3];
					int curDiffR = curDiffG - (rDiff * rDiff);

					if (curDiffR > 0) {
						diff -= curDiffR;
						result = j;

						if (diff == 0)
							break;
					}
				}
			}
		}

		palMap[i] = result;
	}

	Graphics::Surface *convertedSurface = new Graphics::Surface();
	convertedSurface->create(frame->w, frame->h, frame->format);

	for (int y = 0; y < frame->h; y++)
		for (int x = 0; x < frame->w; x++)
			*((byte *)convertedSurface->getBasePtr(x, y)) = palMap[*((byte *)frame->getBasePtr(x, y))];

	return convertedSurface;
}

} // End of namespace Buried
