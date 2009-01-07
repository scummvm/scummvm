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

#include "common/archive.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "common/array.h"
#include "common/endian.h"

#include "graphics/video/video_player.h"
#include "graphics/surface.h"

namespace Graphics {

VideoPlayer::VideoPlayer() : _fileStream(0), _skipVideo(false) {
}

VideoPlayer::~VideoPlayer() {
	closeFile();
}

int VideoPlayer::getWidth() {
	if (!_fileStream)
		return 0;
	return _videoInfo.width;
}

int VideoPlayer::getHeight() {
	if (!_fileStream)
		return 0;
	return _videoInfo.height;
}

int32 VideoPlayer::getCurFrame() {
	if (!_fileStream)
		return -1;
	return _videoInfo.currentFrame;
}

int32 VideoPlayer::getFrameCount() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameCount;
}

int32 VideoPlayer::getFrameRate() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameRate;
}

int32 VideoPlayer::getFrameDelay() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameDelay;
}

int32 VideoPlayer::getAudioLag() {
	if (!_fileStream)
		return 0;

	/* No audio.
	   Calculate the lag by how much time has gone by since the first frame
	   and how much time *should* have passed.
	*/
	int32 audioTime = (g_system->getMillis() - _videoInfo.startTime) * 100;
	int32 videoTime = _videoInfo.currentFrame * getFrameDelay();

	return videoTime - audioTime;
}

uint32 VideoPlayer::getFrameWaitTime() {
	int32 waitTime = (getFrameDelay() + getAudioLag()) / 100;

	if (waitTime < 0)
		return 0;

	return waitTime;
}

bool VideoPlayer::loadFile(const char *fileName) {
	return false;
}

void VideoPlayer::closeFile() {
}

void VideoPlayer::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = getHeight();
	uint w = getWidth();

	byte *src = _videoFrameBuffer;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += w;
	} while (--h);
}

void VideoPlayer::setPalette(byte *pal) {
	for (int i = 0; i < 256; i++) {
		_videoPalette[i * 4 + 0] = *pal++;
		_videoPalette[i * 4 + 1] = *pal++;
		_videoPalette[i * 4 + 2] = *pal++;
		_videoPalette[i * 4 + 3] = 0;
	}

	g_system->setPalette(_videoPalette, 0, 256);
}

bool VideoPlayer::decodeNextFrame() {
	return false;
}

void VideoPlayer::performPostProcessing(byte *screen) {
}

void VideoPlayer::processVideoEvents(Common::List<Common::Event> *stopEvents) {
	Common::Event curEvent;
	Common::EventManager *eventMan = g_system->getEventManager();

	// Process events, and skip video if esc is pressed
	while (eventMan->pollEvent(curEvent)) {
		if (curEvent.type == Common::EVENT_RTL || curEvent.type == Common::EVENT_QUIT) {
			_skipVideo = true;
		}

		for (Common::List<Common::Event>::const_iterator iter = stopEvents->begin(); iter != stopEvents->end(); iter++) {
			if (curEvent.type == iter->type) {
				if (iter->type == Common::EVENT_KEYDOWN || iter->type == Common::EVENT_KEYUP) {
					if (curEvent.kbd.keycode == iter->kbd.keycode) {
						_skipVideo = true;
						break;
					}
				} else {
					_skipVideo = true;
					break;
				}
			}
		}
	}
}

bool VideoPlayer::playVideo(const char *filename, Common::List<Common::Event> *stopEvents) {
	_skipVideo = false;
	debug(0, "Playing video %s", filename);

	if (!loadFile(filename)) {
		warning("Failed to load video file %s", filename);
		return false;
	}

	while (getCurFrame() < getFrameCount() && !_skipVideo) {
		processVideoEvents(stopEvents);

		uint32 startTime = 0;
		decodeNextFrame();

		Graphics::Surface *screen = g_system->lockScreen();
		copyFrameToBuffer((byte *)screen->pixels,
							(g_system->getWidth() - getWidth()) / 2,
							(g_system->getHeight() - getHeight()) / 2,
							g_system->getWidth());
		performPostProcessing((byte *)screen->pixels);
		g_system->unlockScreen();

		uint32 waitTime = getFrameWaitTime();

		if (!waitTime) {
			warning("dropped frame %i", getCurFrame());
			continue;
		}

		// Update the screen
		g_system->updateScreen();

		startTime = g_system->getMillis();

		// Wait before showing the next frame
		while (g_system->getMillis() < startTime + waitTime && !_skipVideo) {
			processVideoEvents(stopEvents);
			g_system->delayMillis(10);
		}
	}

	closeFile();

	return true;
}

} // End of namespace Graphics
