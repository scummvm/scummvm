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
 * $URL$
 * $Id$
 *
 */

#include "graphics/video/video_decoder.h"

#include "common/file.h"
#include "common/system.h"

namespace Graphics {

VideoDecoder::VideoDecoder() {
	reset();
}

bool VideoDecoder::loadFile(const Common::String &filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		return false;
	}

	return load(*file);
}

uint32 VideoDecoder::getElapsedTime() const {
	return g_system->getMillis() - _startTime;
}

void VideoDecoder::setSystemPalette() {
	byte *vidPalette = getPalette();
	byte *sysPalette = new byte[256 * 4];

	for (uint16 i = 0; i < 256; i++) {
		sysPalette[i * 4]     = vidPalette[i * 3];
		sysPalette[i * 4 + 1] = vidPalette[i * 3 + 1];
		sysPalette[i * 4 + 2] = vidPalette[i * 3 + 2];
		sysPalette[i * 4 + 3] = 0;
	}

	g_system->setPalette(sysPalette, 0, 256);
	delete[] sysPalette;
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
	} else {
		assert(_pauseLevel); // We can't go negative
		_pauseLevel--;
	}

	if (_pauseLevel == 1 && pause) {
		_pauseStartTime = g_system->getMillis(); // Store the starting time from pausing to keep it for later
		pauseVideoIntern(true);
	} else if (_pauseLevel == 0) {
		pauseVideoIntern(false);
		addPauseTime(g_system->getMillis() - _pauseStartTime);
	}
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

} // End of namespace Graphics
