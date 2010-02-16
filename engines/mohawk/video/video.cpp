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

#include "mohawk/resource.h"
#include "mohawk/video/video.h"
#include "mohawk/video/qt_player.h"

#include "common/events.h"

namespace Mohawk {

VideoManager::VideoManager(MohawkEngine* vm) : _vm(vm) {
	_pauseStart = 0;
}

VideoManager::~VideoManager() {
	_mlstRecords.clear();
	stopVideos();
}

void VideoManager::pauseVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i]->pauseAudio();
	_pauseStart = _vm->_system->getMillis() * 100;
}

void VideoManager::resumeVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++) {
		_videoStreams[i]->addPauseTime(_vm->_system->getMillis() * 100 - _pauseStart);
		_videoStreams[i]->resumeAudio();
	}

	_pauseStart = 0;
}

void VideoManager::stopVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		delete _videoStreams[i].video;
	_videoStreams.clear();
}

void VideoManager::playMovie(Common::String filename, uint16 x, uint16 y, bool clearScreen) {
	VideoHandle videoHandle = createVideoHandle(filename, x, y, false);
	if (videoHandle == NULL_VID_HANDLE)
		return;

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	waitUntilMovieEnds(videoHandle);
}

void VideoManager::playMovieCentered(Common::String filename, bool clearScreen) {
	VideoHandle videoHandle = createVideoHandle(filename, 0, 0, false);
	if (videoHandle == NULL_VID_HANDLE)
		return;

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

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
				case Common::KEYCODE_SPACE:
					_vm->pauseGame();
					break;
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

	_videoStreams[videoHandle]->stop();
	_videoStreams.clear();
}

void VideoManager::playBackgroundMovie(Common::String filename, int16 x, int16 y, bool loop) {
	VideoHandle videoHandle = createVideoHandle(filename, x, y, loop);
	if (videoHandle == NULL_VID_HANDLE)
		return;

	// Center x if requested
	if (x < 0)
		_videoStreams[videoHandle].x = (_vm->_system->getWidth() - _videoStreams[videoHandle]->getWidth()) / 2;

	// Center y if requested
	if (y < 0)
		_videoStreams[videoHandle].y = (_vm->_system->getHeight() - _videoStreams[videoHandle]->getHeight()) / 2;
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
				_videoStreams[i]->reset();
			} else {
				delete _videoStreams[i].video;
				memset(&_videoStreams[i], 0, sizeof(VideoEntry));
				_videoStreams[i].video = NULL;
				continue;
			}
		}

		// Check if we need to draw a frame
		if (_videoStreams[i]->needsUpdate()) {
			Graphics::Surface *frame = _videoStreams[i]->getNextFrame();
			bool deleteFrame = false;

			if (frame && _videoStreams[i].enabled) {
				// Convert from 8bpp to the current screen format if necessary
				if (frame->bytesPerPixel == 1) {
					Graphics::Surface *newFrame = new Graphics::Surface();
					Graphics::PixelFormat pixelFormat = _vm->_system->getScreenFormat();
					byte *palette = _videoStreams[i]->getPalette();
					assert(palette);

					newFrame->create(frame->w, frame->h, pixelFormat.bytesPerPixel);

					for (uint16 j = 0; j < frame->h; j++) {
						for (uint16 k = 0; k < frame->w; k++) {
							byte palIndex = *((byte *)frame->getBasePtr(k, j));
							byte r = palette[palIndex * 4];
							byte g = palette[palIndex * 4 + 1];
							byte b = palette[palIndex * 4 + 2];
							if (pixelFormat.bytesPerPixel == 2)
								*((uint16 *)newFrame->getBasePtr(k, j)) = pixelFormat.RGBToColor(r, g, b);
							else
								*((uint32 *)newFrame->getBasePtr(k, j)) = pixelFormat.RGBToColor(r, g, b);
						}
					}

					frame = newFrame;
					deleteFrame = true;
				}

				// Check if we're drawing at a 2x or 4x resolution (because of
				// evil QuickTime scaling it first).
				if (_videoStreams[i]->getScaleMode() == kScaleHalf || _videoStreams[i]->getScaleMode() == kScaleQuarter) {
					byte scaleFactor = (_videoStreams[i]->getScaleMode() == kScaleHalf) ? 2 : 4;

					Graphics::Surface scaledSurf;
					scaledSurf.create(_videoStreams[i]->getWidth() / scaleFactor, _videoStreams[i]->getHeight() / scaleFactor, frame->bytesPerPixel);

					for (uint32 j = 0; j < scaledSurf.h; j++)
						for (uint32 k = 0; k < scaledSurf.w; k++)
							memcpy(scaledSurf.getBasePtr(k, j), frame->getBasePtr(k * scaleFactor, j * scaleFactor), frame->bytesPerPixel);

					_vm->_system->copyRectToScreen((byte*)scaledSurf.pixels, scaledSurf.pitch, _videoStreams[i].x, _videoStreams[i].y, scaledSurf.w, scaledSurf.h);
					scaledSurf.free();
				} else {
					// Clip the width/height to make sure we stay on the screen (Myst does this a few times)
					uint16 width = MIN<int32>(_videoStreams[i]->getWidth(), _vm->_system->getWidth() - _videoStreams[i].x);
					uint16 height = MIN<int32>(_videoStreams[i]->getHeight(), _vm->_system->getHeight() - _videoStreams[i].y);
					_vm->_system->copyRectToScreen((byte*)frame->pixels, frame->pitch, _videoStreams[i].x, _videoStreams[i].y, width, height);
				}

				// We've drawn something to the screen, make sure we update it
				updateScreen = true;

				// Delete the frame if we're using the buffer from the 8bpp conversion
				if (deleteFrame) {
					frame->free();
					delete frame;
				}
			}
		}

		// Update the audio buffer too
		_videoStreams[i]->updateAudioBuffer();
	}

	// Return true if we need to update the screen
	return updateScreen;
}

void VideoManager::activateMLST(uint16 mlstId, uint16 card) {
	Common::SeekableReadStream *mlstStream = _vm->getRawData(ID_MLST, card);
	uint16 recordCount = mlstStream->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		MLSTRecord mlstRecord;
		mlstRecord.index = mlstStream->readUint16BE();
		mlstRecord.movieID = mlstStream->readUint16BE();
		mlstRecord.code = mlstStream->readUint16BE();
		mlstRecord.left = mlstStream->readUint16BE();
		mlstRecord.top = mlstStream->readUint16BE();

		for (byte j = 0; j < 2; j++)
			if (mlstStream->readUint16BE() != 0)
				warning("u0[%d] in MLST non-zero", j);

		if (mlstStream->readUint16BE() != 0xFFFF)
			warning("u0[2] in MLST not 0xFFFF");

		mlstRecord.loop = mlstStream->readUint16BE();
		mlstRecord.volume = mlstStream->readUint16BE();
		mlstRecord.u1 = mlstStream->readUint16BE();

		if (mlstRecord.u1 != 1)
			warning("mlstRecord.u1 not 1");

		if (mlstRecord.index == mlstId) {
			_mlstRecords.push_back(mlstRecord);
			break;
		}
	}

	delete mlstStream;
}

void VideoManager::playMovie(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (non-blocking) at (%d, %d) %s", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].loop != 0 ? "looping" : "non-looping");
			createVideoHandle(_mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].loop != 0);
			return;
		}
}

void VideoManager::playMovieBlocking(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (blocking) at (%d, %d)", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top);
			VideoHandle videoHandle = createVideoHandle(_mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, false);
			waitUntilMovieEnds(videoHandle);
			return;
		}
}

void VideoManager::stopMovie(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			warning("STUB: Stop tMOV %d", _mlstRecords[i].movieID);
			return;
		}
}

void VideoManager::enableMovie(uint16 id) {
	debug(2, "Enabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint16 j = 0; j < _videoStreams.size(); j++)
				if (_mlstRecords[i].movieID == _videoStreams[j].id) {
					_videoStreams[j].enabled = true;
					return;
				}
}

void VideoManager::disableMovie(uint16 id) {
	debug(2, "Disabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint16 j = 0; j < _videoStreams.size(); j++)
				if (_mlstRecords[i].movieID == _videoStreams[j].id) {
					_videoStreams[j].enabled = false;
					return;
				}
}

void VideoManager::disableAllMovies() {
	debug(2, "Disabling all movies");
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i].enabled = false;
}

VideoHandle VideoManager::createVideoHandle(uint16 id, uint16 x, uint16 y, bool loop) {
	// First, check to see if that video is already playing
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].id == id)
			return i;

	// Otherwise, create a new entry
	VideoEntry entry;
	entry.video = new QTPlayer();
	entry.x = x;
	entry.y = y;
	entry.filename = "";
	entry.id = id;
	entry.loop = loop;
	entry.enabled = true;
	entry->setChunkBeginOffset(_vm->getResourceOffset(ID_TMOV, id));
	entry->loadFile(_vm->getRawData(ID_TMOV, id));
	entry->startAudio();

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

VideoHandle VideoManager::createVideoHandle(Common::String filename, uint16 x, uint16 y, bool loop) {
	// First, check to see if that video is already playing
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].filename == filename)
			return i;

	// Otherwise, create a new entry
	VideoEntry entry;
	entry.video = new QTPlayer();
	entry.x = x;
	entry.y = y;
	entry.filename = filename;
	entry.id = 0;
	entry.loop = loop;
	entry.enabled = true;
	
	Common::File *file = new Common::File();
	if (!file->open(filename)) {
		delete file;
		return NULL_VID_HANDLE;
	}
	
	entry->loadFile(file);
	entry->startAudio();
	
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

} // End of namespace Mohawk
