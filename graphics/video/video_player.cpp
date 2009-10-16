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
#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "common/array.h"
#include "common/endian.h"

#include "graphics/video/video_player.h"
#include "graphics/surface.h"

namespace Graphics {

VideoDecoder::VideoDecoder() : _fileStream(0) {
	_curFrameBlack = 0;
	_curFrameWhite = 255;
}

VideoDecoder::~VideoDecoder() {
}

int VideoDecoder::getWidth() {
	if (!_fileStream)
		return 0;
	return _videoInfo.width;
}

int VideoDecoder::getHeight() {
	if (!_fileStream)
		return 0;
	return _videoInfo.height;
}

int32 VideoDecoder::getCurFrame() {
	if (!_fileStream)
		return -1;
	return _videoInfo.currentFrame;
}

int32 VideoDecoder::getFrameCount() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameCount;
}

int32 VideoDecoder::getFrameRate() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameRate;
}

int32 VideoDecoder::getFrameDelay() {
	if (!_fileStream)
		return 0;
	return _videoInfo.frameDelay;
}

int32 VideoDecoder::getAudioLag() {
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

uint32 VideoDecoder::getFrameWaitTime() {
	int32 waitTime = (getFrameDelay() + getAudioLag()) / 100;

	if (waitTime < 0)
		return 0;

	return waitTime;
}

void VideoDecoder::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
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

void VideoDecoder::setPalette(byte *pal) {
	byte videoPalette[256 * 4];

	uint32 maxWeight = 0;
	uint32 minWeight = 0xFFFFFFFF;
	uint32 weight = 0;
	byte r, g, b;

	for (int i = 0; i < 256; i++) {
		videoPalette[i * 4 + 0] = *pal++;
		videoPalette[i * 4 + 1] = *pal++;
		videoPalette[i * 4 + 2] = *pal++;
		videoPalette[i * 4 + 3] = 0;

		// Try and find the white and black colors for the current palette
		r = videoPalette[i * 4 + 0];
		g = videoPalette[i * 4 + 1];
		b = videoPalette[i * 4 + 2];

		weight = 3 * r * r + 6 * g * g + 2 * b * b;

		if (weight >= maxWeight) {
			_curFrameWhite = i;
			maxWeight = weight;
		}

		if (weight <= minWeight) {
			_curFrameBlack = i;
			minWeight = i;
		}
	}

	g_system->setPalette(videoPalette, 0, 256);
}


/*
 *  VideoPlayer
 */

void VideoPlayer::processVideoEvents(Common::List<Common::Event> &stopEvents) {
	Common::Event curEvent;
	Common::EventManager *eventMan = g_system->getEventManager();

	// Process events, and skip video if esc is pressed
	while (eventMan->pollEvent(curEvent)) {
		if (curEvent.type == Common::EVENT_RTL || curEvent.type == Common::EVENT_QUIT) {
			_skipVideo = true;
		}

		for (Common::List<Common::Event>::const_iterator iter = stopEvents.begin(); iter != stopEvents.end(); iter++) {
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

bool VideoPlayer::playVideo(Common::List<Common::Event> &stopEvents) {
	_skipVideo = false;
	debug(0, "Playing video");

	g_system->fillScreen(0);

	int frameX = (g_system->getWidth() - _decoder->getWidth()) / 2;
	int frameY = (g_system->getHeight() - _decoder->getHeight()) / 2;

	while (_decoder->getCurFrame() < _decoder->getFrameCount() && !_skipVideo) {
		processVideoEvents(stopEvents);

		uint32 startTime = 0;
		_decoder->decodeNextFrame();

		Graphics::Surface *screen = g_system->lockScreen();
		_decoder->copyFrameToBuffer((byte *)screen->pixels, frameX, frameY, g_system->getWidth());
		performPostProcessing((byte *)screen->pixels);
		g_system->unlockScreen();

		uint32 waitTime = _decoder->getFrameWaitTime();

		if (!waitTime) {
			warning("dropped frame %i", _decoder->getCurFrame());
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

	return !_skipVideo;
}

bool VideoPlayer::playVideo() {
	Common::Event stopEvent;
	Common::List<Common::Event> stopEvents;
	stopEvents.clear();
	stopEvent.type = Common::EVENT_KEYDOWN;
	stopEvent.kbd = Common::KEYCODE_ESCAPE;
	stopEvents.push_back(stopEvent);

	return playVideo(stopEvents);
}

void VideoPlayer::performPostProcessing(byte *screen) {
}

} // End of namespace Graphics
