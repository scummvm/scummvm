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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/video_window.h"

#include "common/system.h"
#include "common/keyboard.h"
#include "graphics/surface.h"
#include "video/avi_decoder.h"

namespace Buried {

VideoWindow::VideoWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent), _video(nullptr), _mode(kModeClosed), _lastFrame(nullptr) {
	_vm->addVideo(this);
	_needsPalConversion = false;
	_ownedFrame = nullptr;
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

	_vm->_gfx->toggleCursor(false);
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

	// We do not hide the mouse cursor here, as this
	// is used to play background or asynchronous
	// animations
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
		_vm->_gfx->toggleCursor(true);
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
	_video->setSoundType(Audio::Mixer::kSFXSoundType);

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
	_rect.right = _rect.left + _video->getWidth();
	_rect.bottom = _rect.top + _video->getHeight();
	return true;
}

void VideoWindow::closeVideo() {
	if (_video) {
		delete _video;
		_video = nullptr;
		_vm->_gfx->toggleCursor(true);
		_mode = kModeClosed;
		_lastFrame = nullptr;

		if (_ownedFrame) {
			_ownedFrame->free();
			delete _ownedFrame;
			_ownedFrame = nullptr;
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
					_ownedFrame = nullptr;
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
						_ownedFrame = _vm->_gfx->remapPalettedFrame(frame, _video->getPalette());
						_lastFrame = _ownedFrame;
					} else {
						// Assume it's in the right format from dithering
						_lastFrame = frame;
					}
				}
			}

			// Invalidate the window so it gets updated
			invalidateWindow(false);
		}

		if (_video->isPlaying() && _video->endOfVideo()) {
			_video->stop();
			_vm->_gfx->toggleCursor(true);
			_mode = kModeStopped;
		}
	}
}

void VideoWindow::onPaint() {
	if (_lastFrame) {
		Common::Rect absoluteRect = getAbsoluteRect();

		if (_srcRect.isEmpty() && _dstRect.isEmpty())
			_vm->_gfx->blit(_lastFrame, absoluteRect.left, absoluteRect.top, absoluteRect.width(), absoluteRect.height());
		else
			_vm->_gfx->crossBlit(_vm->_gfx->getScreen(), absoluteRect.left + _dstRect.left, absoluteRect.top + _dstRect.top, _dstRect.width(), _dstRect.height(), _lastFrame, _srcRect.left, _srcRect.top);
	}
}

void VideoWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	if (key.keycode == Common::KEYCODE_ESCAPE)
		stopVideo();
}

void VideoWindow::setSourceRect(const Common::Rect &srcRect) {
	_srcRect = srcRect;
}

void VideoWindow::setDestRect(const Common::Rect &dstRect) {
	_dstRect = dstRect;
}

void VideoWindow::setAudioTrack(int track) {
	if (_video)
		_video->setAudioTrack(track);
}

void VideoWindow::pauseVideo() {
	if (_video)
		_video->pauseVideo(true);
}

void VideoWindow::resumeVideo() {
	if (_video)
		_video->pauseVideo(false);
}

} // End of namespace Buried
