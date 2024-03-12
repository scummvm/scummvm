/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/movie_pc.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/sound/music_manager.h"
#include "engines/icb/cluster_manager_pc.h"
#include "engines/icb/res_man_pc.h"
#include "engines/icb/global_objects.h"

#include "common/system.h"
#include "common/keyboard.h"

namespace ICB {

// Instance our global bink handler for movies
MovieManager *g_theSequenceManager;

MovieManager::MovieManager() {
	_binkDecoder = nullptr;
	_x = 0;
	_y = 0;
	_fadeCounter = 255;
	_fadeRate = 4;
	_haveFaded = false;
	_rater = true;
	_haveClearedScreen = false;
	_loop = FALSE8;
}

MovieManager::~MovieManager() {
	kill();
}

bool MovieManager::registerMovie(const char *fileName, bool8 fade, bool8 loop) {
	// Release any currently held sequence
	kill();
	_x = 0;
	_y = 0;

	if (g_theMusicManager)
		g_theMusicManager->StopMusic();

	_binkDecoder = new Video::BinkDecoder();

	Common::SeekableReadStream *stream = openDiskFileForBinaryStreamRead(fileName);
	if (!stream) {
		return false;
	}
	if (!_binkDecoder->loadStream(stream)) {
		return false;
	}

	_binkDecoder->setOutputPixelFormat(Graphics::PixelFormat(4, 8, 8, 8, 0, 16, 8, 0, 24));

	if (_binkDecoder->getWidth() != SCREEN_WIDTH) {
		_x = (SCREEN_WIDTH / 2) - (_binkDecoder->getWidth() / 2);
	}
	if (_binkDecoder->getHeight() != SCREEN_DEPTH) {
		_y = (SCREEN_DEPTH / 2) - (_binkDecoder->getHeight() / 2);
	}

	// Should we fade the screen out before playing the movie
	if (fade) {
		_fadeCounter = 1; // Yes we should
	} else {
		_fadeCounter = 255; // No thanks
	}

	_haveFaded = false;
	_loop = loop;

	return true;
}

uint32 MovieManager::getMovieHeight() {
	if (!busy())
		return 0;
	return _binkDecoder->getHeight();
}

uint32 MovieManager::getMovieWidth() {
	if (!busy())
		return 0;
	return _binkDecoder->getWidth();
}

uint32 MovieManager::getMovieFrames() {
	if (!busy())
		return 0;
	return _binkDecoder->getFrameCount();
}

bool MovieManager::busy() {
	return _binkDecoder != nullptr;
}

int32 MovieManager::getFrameNumber() {
	if (busy())
		return _binkDecoder->getCurFrame() + 1;

	return 0;
}

uint32 MovieManager::drawFrame(uint32 surface_id) {
	if (!_binkDecoder) {
		return FINISHED;
	}

	// Non-looping movies can be quit using the escape key
	if (Read_DI_once_keys(Common::KEYCODE_ESCAPE) && !_loop) {
		kill();
		return FINISHED;
	}

	// Don't do bink, fade the screen
	if (_fadeCounter < (255 / _fadeRate)) {
		fadeScreen(surface_id);
		_haveFaded = true;
		return FADING;
	}

	// Have we performed a fade
	if (_haveFaded) {
		// Reassign correct surface id
		surface_id = working_buffer_id;
		_haveFaded = false;
		if (getFrameNumber() == 0)
			_binkDecoder->start();
	} else {
		// Black out the screen before we play the movie
		if ((getFrameNumber() == 0) && !_haveClearedScreen) {
			surface_manager->Clear_surface(working_buffer_id);
			_haveClearedScreen = true;
			_binkDecoder->start();
		}
	}

	// Decompress a frame
	const Graphics::Surface *surface = _binkDecoder->decodeNextFrame();
	if (!surface || _binkDecoder->endOfVideo()) {
		if (_loop) {
			_binkDecoder->rewind();
			surface = _binkDecoder->decodeNextFrame();
		} else {
			kill();
			surface_manager->Clear_surface(working_buffer_id);

			return FINISHED;
		}
	}

	if (_rater) {
		uint32 waitTime = _binkDecoder->getTimeToNextFrame();
		g_system->delayMillis(waitTime);
	}

	// For access to buffer
	int32 pitch;
	uint8 *surface_address;

	// Lock the surface
	surface_address = surface_manager->Lock_surface(surface_id);
	pitch = surface_manager->Get_pitch(surface_id);
	uint32 height = surface_manager->Get_height(surface_id);

	for (int32 i = 0; i < surface->h; i++) {
		if (i + _y >= (int32)height) {
			break;
		}
		memcpy(surface_address + (i + _y) * pitch, surface->getBasePtr(0, i), MIN(surface->pitch, pitch));
	}

	// Unlock the buffer
	surface_manager->Unlock_surface(surface_id);
	return JUSTFINE;
}

void MovieManager::setRate() {
	// Toggle
	if (_rater)
		_rater = false;
	else
		_rater = true;

	if (_binkDecoder) {
		// Mute the sound if necessary
		if (_rater)
			setVolume(GetMusicVolume());
		else
			setVolume(0);
	}
}

void MovieManager::fadeScreen(uint32 surface_id) {
	// Pointer to subtractive table block
	uint8 subtractive[8];

	// Fade by table
	subtractive[4] = subtractive[0] = (uint8)_fadeRate;
	subtractive[5] = subtractive[1] = (uint8)_fadeRate;
	subtractive[6] = subtractive[2] = (uint8)_fadeRate;
	subtractive[7] = subtractive[3] = 0;

	// Lock the directdraw surface
	uint8 *surface_address = surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);

	for (uint32 lines = 0; lines < SCREEN_DEPTH; lines++) {
		for (int32 xPos = 0; xPos < SCREEN_WIDTH; xPos++) {
			// 32-bit BGRA pixel
			uint8 *pixel = &surface_address[xPos * 4];
			// Subtract from RGB components
			for (int32 i = 0; i < 3; i++) {
				pixel[i] = MAX(0, pixel[i] - subtractive[i]);
			}
		}
		// Next line
		surface_address += pitch;
	}

	// Unlock the buffer
	surface_manager->Unlock_surface(surface_id);

	// Increment fade level (pixel shift right)
	_fadeCounter++;
}

void MovieManager::setVolume(int32 vol) {
	if (_binkDecoder) {
		float volumeConversion = Audio::Mixer::kMaxChannelVolume / 128.0f;
		_binkDecoder->setVolume(volumeConversion * vol);
	}
}

void MovieManager::kill() {
	if (_binkDecoder) {
		_binkDecoder->close();
		delete _binkDecoder;
		_binkDecoder = nullptr;
	}

	// Reset blitting coordinates
	_x = 0;
	_y = 0;

	_fadeCounter = 255;

	_haveClearedScreen = false;
}

} // End of namespace ICB
