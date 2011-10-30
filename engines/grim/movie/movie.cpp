/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "graphics/surface.h"

#include "common/system.h"
#include "common/timer.h"

#include "engines/grim/movie/movie.h"
#include "engines/grim/grim.h"
#include "engines/grim/debug.h"
#include "engines/grim/savegame.h"

namespace Grim {

MoviePlayer *g_movie;

MoviePlayer::MoviePlayer() {
	_speed = 0;
	_channels = -1;
	_freq = 22050;
	_videoFinished = false;
	_videoLooping = false;
	_videoPause = true;
	_updateNeeded = false;
	_movieTime = 0;
	_frame = -1;
	_x = 0;
	_y = 0;
	_videoDecoder = NULL;
	_internalSurface = NULL;
	_externalSurface = new Graphics::Surface();
}

MoviePlayer::~MoviePlayer() {
	deinit();
	delete _videoDecoder;
}

void MoviePlayer::pause(bool p) {
	_videoPause = p;
	_videoDecoder->pauseVideo(p);
}

void MoviePlayer::stop() {
	deinit();
	g_grim->setMode(GrimEngine::NormalMode);
}

void MoviePlayer::timerCallback(void *) {
	g_movie->_frameMutex.lock();
	if (g_movie->prepareFrame())
		g_movie->handleFrame();
	g_movie->_frameMutex.unlock();
}

bool MoviePlayer::prepareFrame() {
	if (_videoDecoder->endOfVideo())
		_videoFinished = true;

	if (_videoPause)
		return false;

	if (_videoFinished) {
		g_grim->setMode(GrimEngine::NormalMode);
		_videoPause = true;
		return false;
	}

	if (_videoDecoder->getTimeToNextFrame() > 0)
		return false;

	_internalSurface = _videoDecoder->decodeNextFrame();
	_updateNeeded = true;

	_movieTime = _videoDecoder->getElapsedTime();
	_frame = _videoDecoder->getCurFrame();

	return true;
}

Graphics::Surface *MoviePlayer::getDstSurface() {
	_frameMutex.lock();
	if (_updateNeeded && _internalSurface) {
		_externalSurface->copyFrom(*_internalSurface);
	}
	_frameMutex.unlock();

	return _externalSurface;
}

void MoviePlayer::init() {
	_frame = -1;
	_movieTime = 0;
	_updateNeeded = false;
	_videoFinished = false;
}

void MoviePlayer::deinit() {
	_frameMutex.unlock();
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
	
	if (_videoDecoder)
		_videoDecoder->close();
	
	_internalSurface = NULL;
	
	if (_externalSurface)
		_externalSurface->free();

	_videoPause = false;
	_videoFinished = true;
}

bool MoviePlayer::play(Common::String filename, bool looping, int x, int y) {
	deinit();
	_x = x;
	_y = y;
	_fname = filename;
	_videoLooping = looping;

	if (!loadFile(_fname))
		return false;

	Debug::debug(Debug::Movie, "Playing video '%s'.\n", filename.c_str());

	init();

	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
	_internalSurface = NULL;

	return true;
}

bool MoviePlayer::loadFile(Common::String filename) {
	return _videoDecoder->loadFile(filename);
}

void MoviePlayer::saveState(SaveGame *state) {
	state->beginSection('SMUS');

	state->writeString(_fname);

	state->writeLESint32(_frame);
	state->writeFloat(_movieTime);
	state->writeLESint32(_videoFinished);
	state->writeLESint32(_videoLooping);

	state->writeLESint32(_x);
	state->writeLESint32(_y);

	state->endSection();
}

void MoviePlayer::restoreState(SaveGame *state) {
	state->beginSection('SMUS');

	_fname = state->readString();

	int32 frame = state->readLESint32();
	float movieTime = state->readFloat();
	bool videoFinished = state->readLESint32();
	bool videoLooping = state->readLESint32();

	int x = state->readLESint32();
	int y = state->readLESint32();

	if (!videoFinished) {
		play(_fname.c_str(), videoLooping, x, y);
	}
	_frame = frame;
	_movieTime = movieTime;

	state->endSection();
}

#if !defined(USE_MPEG2) || !defined(USE_SMUSH) || !defined(USE_BINK)
#define NEED_NULLPLAYER
#endif

// Fallback for when USE_MPEG2 / USE_BINK / USE_SMUSH isnt defined

#ifdef NEED_NULLPLAYER
class NullPlayer : public MoviePlayer {
public:
	NullPlayer(const char* codecID) {
		warning("%s-playback not compiled in, but needed", codecID);
		_videoFinished = true; // Rigs all movies to be completed.
	}
	~NullPlayer() {}
	bool play(Common::String filename, bool looping, int x, int y) {return true;}
	bool loadFile(Common::String filename) { return true; }
	void stop() {}
	void pause(bool p) {}
	void saveState(SaveGame *state) {}
	void restoreState(SaveGame *state) {}
private:
	static void timerCallback(void *ptr) {}
	void handleFrame() {}
	bool prepareFrame() { return false; }
	void init() {}
	void deinit() {}
};
#endif

#ifndef USE_MPEG2
MoviePlayer *CreateMpegPlayer() {
	return new NullPlayer("MPEG2");
}
#endif

#ifndef USE_SMUSH
MoviePlayer *CreateSmushPlayer(bool demo) {
	return new NullPlayer("SMUSH");
}
#endif

#ifndef USE_BINK
MoviePlayer *CreateBinkPlayer(bool demo) {
	return new NullPlayer("BINK");
}
#endif

} // end of namespace Grim
