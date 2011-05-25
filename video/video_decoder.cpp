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

#include "video/video_decoder.h"

#include "common/rational.h"
#include "common/file.h"
#include "common/system.h"

#include "graphics/palette.h"

namespace Video {

VideoDecoder::VideoDecoder() {
	reset();
}

bool VideoDecoder::loadFile(const Common::String &filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		return false;
	}

	return loadStream(file);
}

uint32 VideoDecoder::getElapsedTime() const {
	return g_system->getMillis() - _startTime;
}

void VideoDecoder::setSystemPalette() {
	g_system->getPaletteManager()->setPalette(getPalette(), 0, 256);
}

bool VideoDecoder::needsUpdate() const {
	return !endOfVideo() && getTimeToNextFrame() == 0;
}

void VideoDecoder::reset() {
	_curFrame = -1;
	_startTime = 0;
	_pauseLevel = 0;
}

bool VideoDecoder::endOfVideo() const {
	return !isVideoLoaded() || (getCurFrame() >= (int32)getFrameCount() - 1);
}

void VideoDecoder::pauseVideo(bool pause) {
	if (pause) {
		_pauseLevel++;

	// We can't go negative
	} else if (_pauseLevel) {
		_pauseLevel--;

	// Do nothing
	} else {
		return;
	}

	if (_pauseLevel == 1 && pause) {
		_pauseStartTime = g_system->getMillis(); // Store the starting time from pausing to keep it for later
		pauseVideoIntern(true);
	} else if (_pauseLevel == 0) {
		pauseVideoIntern(false);
		addPauseTime(g_system->getMillis() - _pauseStartTime);
	}
}

void VideoDecoder::resetPauseStartTime() {
	if (isPaused())
		_pauseStartTime = g_system->getMillis();
}

uint32 FixedRateVideoDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	uint32 elapsedTime = getElapsedTime();
	uint32 nextFrameStartTime = getFrameBeginTime(_curFrame + 1);

	// If the time that the next frame should be shown has past
	// the frame should be shown ASAP.
	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

uint32 FixedRateVideoDecoder::getFrameBeginTime(uint32 frame) const {
	Common::Rational beginTime = frame * 1000;
	beginTime /= getFrameRate();
	return beginTime.toInt();
}

} // End of namespace Video
