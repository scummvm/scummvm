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

#include "pegasus/pegasus.h"
#include "pegasus/video.h"

#include "common/events.h"
#include "graphics/scaler.h"
#include "video/qt_decoder.h"

namespace Pegasus {
	
VideoManager::VideoManager(PegasusEngine *vm) : _vm(vm) {
	_timeZoneVideo = new Video::QuickTimeDecoder();
}
	
VideoManager::~VideoManager() {
	stopVideos();
	delete _timeZoneVideo;
}

void VideoManager::setTimeZoneVideo(const Common::String &filename) {
	if (!_timeZoneVideo->loadFile(filename))
		error("Could not load time zone video '%s'", filename.c_str());

	// Set it on pause
	_timeZoneVideo->pauseVideo(true);
}

void VideoManager::drawTimeZoneVideoFrame(uint32 time) {
	assert(_timeZoneVideo->isVideoLoaded());

	if (!_timeZoneVideo->isPaused())
		_timeZoneVideo->pauseVideo(true);

	_timeZoneVideo->seekToTime(Audio::Timestamp(0, time, 600));

	const Graphics::Surface *frame = _timeZoneVideo->decodeNextFrame();

	if (!frame)	
		error("Could not find frame at time %d", time);

	// TODO
}

void VideoManager::playTimeZoneVideoSegment(uint32 startTime, uint32 endTime) {
	assert(_timeZoneVideo->isVideoLoaded());

	if (_timeZoneVideo->isPaused())
		_timeZoneVideo->pauseVideo(false);

	_timeZoneVideo->seekToTime(Audio::Timestamp(0, startTime, 600));

	// TODO
}

void VideoManager::pauseVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i]->pauseVideo(true);
}

void VideoManager::resumeVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i]->pauseVideo(false);
}

void VideoManager::stopVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++) {
		delete _videoStreams[i].video;
		_videoStreams[i].video = 0;
	}
}

void VideoManager::playMovie(Common::String filename, uint16 x, uint16 y) {
	VideoHandle videoHandle = playBackgroundMovie(filename, x, y, false);

	if (videoHandle != NULL_VID_HANDLE)
		waitUntilMovieEnds(videoHandle);
}

void VideoManager::playMovieCentered(Common::String filename) {
	VideoHandle videoHandle = playBackgroundMovie(filename, 0, 0, false);

	if (videoHandle == NULL_VID_HANDLE)
		return;

	_videoStreams[videoHandle].x = (_vm->_system->getWidth() - _videoStreams[videoHandle]->getWidth()) / 2;
	_videoStreams[videoHandle].y = (_vm->_system->getHeight() - _videoStreams[videoHandle]->getHeight()) / 2;

	waitUntilMovieEnds(videoHandle);
}
	
void VideoManager::waitUntilMovieEnds(VideoHandle videoHandle) {
	bool continuePlaying = true;

	while (!_videoStreams[videoHandle]->endOfVideo() && !_vm->shouldQuit() && continuePlaying) {
		if (updateBackgroundMovies())
			_vm->_system->updateScreen();

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				continuePlaying = false;
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					continuePlaying = false;
					break;
				default:
					break;
			}
			default:
				break;
			}
		}

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	delete _videoStreams[videoHandle].video;
	_videoStreams.clear();
}

bool VideoManager::updateBackgroundMovies() {
	bool updateScreen = false;

	for (uint32 i = 0; i < _videoStreams.size() && !_vm->shouldQuit(); i++) {
		// Skip deleted videos
		if (!_videoStreams[i].video)
			continue;

		// Remove any videos that are over
		if (_videoStreams[i]->endOfVideo()) {
			if (_videoStreams[i].loop) {
				_videoStreams[i]->rewind();
			} else {
				delete _videoStreams[i].video;
				memset(&_videoStreams[i], 0, sizeof(VideoEntry));
				_videoStreams[i].video = NULL;
				continue;
			}
		}

		// Check if we need to draw a frame
		if (_videoStreams[i]->needsUpdate()) {
			const Graphics::Surface *frame = _videoStreams[i]->decodeNextFrame();

			if (frame) {			
				if (frame->bytesPerPixel == 1)
					error("Unhandled 8bpp frames"); // Cut out because Pegasus Prime shouldn't need this
				
				// Clip the width/height to make sure we stay on the screen
				uint16 width = MIN<int32>(_videoStreams[i]->getWidth(), _vm->_system->getWidth() - _videoStreams[i].x);
				uint16 height = MIN<int32>(_videoStreams[i]->getHeight(), _vm->_system->getHeight() - _videoStreams[i].y);

				if (width == 320 && height == 240) {
					// TODO: Is this right? At least "Big Movie" and the "Sub Chase Movie" need to be scaled...
					// FIXME: Normal2x is only compiled in when USE_SCALERS is defined
					_videoStreams[i].x = 0;
					_videoStreams[i].y = 0;
					Graphics::Surface scaledSurf;
					scaledSurf.create(frame->w * 2, frame->h * 2, frame->bytesPerPixel);
					Normal2x((byte *)frame->pixels, frame->pitch, (byte *)scaledSurf.pixels, scaledSurf.pitch, frame->w, frame->h);
					_vm->_system->copyRectToScreen((byte*)scaledSurf.pixels, scaledSurf.pitch, _videoStreams[i].x, _videoStreams[i].y, width * 2, height * 2);
					scaledSurf.free();
				} else
					_vm->_system->copyRectToScreen((byte*)frame->pixels, frame->pitch, _videoStreams[i].x, _videoStreams[i].y, width, height);
					

				// We've drawn something to the screen, make sure we update it
				updateScreen = true;
			}
		}
	}

	// Return true if we need to update the screen
	return updateScreen;
}

VideoHandle VideoManager::playBackgroundMovie(Common::String filename, int x, int y, bool loop) {
	// First, check to see if that video is already playing
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].filename == filename)
			return i;

	// Otherwise, create a new entry
	VideoEntry entry;
	entry.video = new Video::QuickTimeDecoder();
	entry.x = x;
	entry.y = y;
	entry.filename = filename;
	entry.loop = loop;
	
	if (!entry->loadFile(filename))
		return NULL_VID_HANDLE;
	
	// Search for any deleted videos so we can take a formerly used slot
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (!_videoStreams[i].video) {
			_videoStreams[i] = entry;
			return i;
		}
			
	// Otherwise, just add it to the list
	_videoStreams.push_back(entry);
	return _videoStreams.size() - 1;
}

void VideoManager::seekToTime(VideoHandle handle, uint32 time) {
	if (handle != NULL_VID_HANDLE)
		_videoStreams[handle]->seekToTime(Audio::Timestamp(0, time, 600));
}

} // End of namespace Pegasus
