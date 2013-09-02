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

#include "video/avi_decoder.h"

namespace Buried {

VideoWindow::VideoWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent), _video(0), _mode(kModeClosed), _lastFrame(0) {
	_vm->addVideo(this);
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

	_mode = kModeOpen;
	return true;
}

void VideoWindow::closeVideo() {
	if (_video) {
		delete _video;
		_video = 0;
		_mode = kModeClosed;
		_lastFrame = 0;
	}
}

void VideoWindow::updateVideo() {
	if (_video) {
		if (_video->needsUpdate()) {
			// Store the frame for later
			const Graphics::Surface *frame = _video->decodeNextFrame();
			if (frame)
				_lastFrame = frame;

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

} // End of namespace Buried
