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

#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush.h"
#include "engines/grim/movie/smush_animator.h"
#include "engines/grim/movie/smush_mixer.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_timer.h"
#include "engines/grim/movie/smush_wave.h"

#include "engines/grim/resource.h"
#include "engines/grim/grim.h"

#include "video/theora_decoder.h"

namespace Grim {

MoviePlayer *CreateSmushPlayer(bool demo) {
	return new SmushPlayer(demo);
}

SmushPlayer::~SmushPlayer() {
	{
		Common::StackLock lock(_frameMutex);
		SmushAnimPlay *anim = _smushAnim;
		SmushAnimator *animator = _smushAnimator;
		SmushWave *wave = _smushWave;
		SmushSound *sound = _smushSound;
		SmushDriver *driver = _smushDriver;
		SmushTimer *timer = _smushTimer;
		_smushAnim = nullptr;
		_smushAnimator = nullptr;
		_smushWave = nullptr;
		_smushSound = nullptr;
		_smushDriver = nullptr;
		_smushTimer = nullptr;

		delete anim;
		delete animator;
		if (wave)
			wave->moduleShutdown();
		if (sound)
			sound->moduleShutdown();
		if (timer)
			timer->moduleShutdown();

		delete wave;
		delete sound;
		delete driver;
		delete timer;
	}

	delete _theoraDecoder;
}

SmushPlayer::SmushPlayer(bool demo) : MoviePlayer(), _demo(demo) {
	_smushDecoder = new SmushDecoder();
	_videoDecoder = _smushDecoder;

	_smushTimer = nullptr;
	_smushDriver = nullptr;
	_smushSound = nullptr;
	_smushWave = nullptr;
	_smushAnim = nullptr;
	_smushAnimator = nullptr;

	if (!_demo) {
		_smushTimer = new SmushTimer();
		_smushDriver = new SmushDriver(_smushTimer);
		_smushSound = new SmushSound(_smushTimer, _smushDriver);
		_smushWave = new SmushWave(_smushSound);
		_smushAnim = new SmushAnimPlay(_smushTimer, _smushSound);
		_smushAnimator = new SmushAnimator(_smushDecoder);

		_smushTimer->moduleStartup();
		_smushSound->moduleStartup();
		_smushWave->moduleStartup();
		_smushDecoder->setWave(_smushWave);
	}

#if defined (USE_THEORADEC)
	_theoraDecoder = new Video::TheoraDecoder();
#else
	warning("VideoTheoraPlayer::initialize - Theora support not compiled in, video will be skipped");
	_theoraDecoder = nullptr;
#endif
	//_smushDecoder->setDemo(_demo);
}

bool SmushPlayer::loadFile(const Common::String &filename) {
	debug(2, "Play video %s", filename.c_str());
	bool success = false;
	_videoDecoder = _smushDecoder;
	if (!_demo)
		success = _videoDecoder->loadStream(g_resourceloader->openNewStreamFile(filename.c_str()));
	else
		success = _videoDecoder->loadFile(Common::Path(filename));

	if (!success) {
#if defined (USE_THEORADEC)
		Common::String baseName(filename);
		baseName.erase(baseName.size() - 4);
		baseName += ".ogv";

		Common::Path theoraFilename(Common::Path("MoviesHD").appendComponent(baseName));

		debug(2, "Trying to open %s", theoraFilename.toString().c_str());
		success = _theoraDecoder->loadFile(theoraFilename);
		_videoDecoder = _theoraDecoder;
		_currentVideoIsTheora = true;
#else
		success = false;
#endif
	} else {
		_videoDecoder = _smushDecoder;
		_currentVideoIsTheora = false;
	}
	return success;
}

void SmushPlayer::init() {
	if (!_currentVideoIsTheora) {
		if (_demo) {
			_x = _smushDecoder->getX();
			_y = _smushDecoder->getY();
		} else {
			_smushDecoder->setLooping(_videoLooping);
		}
	}
	MoviePlayer::init();

	if (_smushAnim && !_currentVideoIsTheora) {
		_smushAnim->anmStop();
		_smushAnim->anmStart(15, 24589);
		_smushAnimator->animatorNewAnim(24589, -1, -1);
	}
}

bool SmushPlayer::prepareFrame() {
	if (_demo || _currentVideoIsTheora || !_smushAnim)
		return MoviePlayer::prepareFrame();

	if (_videoPause)
		return false;

	if (_videoFinished) {
		if (g_grim->getMode() == GrimEngine::SmushMode)
			g_grim->setMode(GrimEngine::NormalMode);

		_videoPause = true;
		return false;
	}

	int state = _smushAnim->anmRender(this);
	if (state == 6)
		return false;

	if (state < 0 || state == 4) {
		_smushAnim->anmStop();

		if (_videoLooping) {
			_smushDecoder->rewind();
			_smushDecoder->start();
			// The disasm restarts looping movies with the frame rate of the header
			int delay = _smushDecoder->getFrameDelay();
			_smushAnim->anmStart(delay ? (delay / 2 + 1000000) / delay : 15, 24589);
			_smushAnimator->animatorNewAnim(24589, -1, -1);
		} else {
			_videoFinished = true;
		}
	}

	return true;
}

int SmushPlayer::animatorRender(int flags) {
	int state = _smushAnimator->animatorRender(flags);
	if (state < 0 || state == 5)
		return state;

	_internalSurface = _smushDecoder->getFrameSurface();
	_internalPalette = _smushDecoder->getPalette();

	int currentFrame = _smushDecoder->getCurFrame();
	if (_frame != currentFrame)
		_updateNeeded = true;

	_frame = currentFrame;

	_movieTime = _smushDecoder->getTime();

	return state;
}

void SmushPlayer::pause(bool p) {
	Common::StackLock lock(_frameMutex);
	MoviePlayer::pause(p);

	if (_smushAnimator && !_videoFinished && !_currentVideoIsTheora)
		_smushAnimator->animatorPause(p);
}

int32 SmushPlayer::getMovieTime() {
	if (_demo || _currentVideoIsTheora || !_smushSound)
		return MoviePlayer::getMovieTime();

	Common::StackLock lock(_frameMutex);
	if (_videoFinished)
		return 0;

	return _smushSound->souEngineQueryTime(0);
}

void SmushPlayer::resetFrameCounter() {
	_smushDecoder->resetBlocky16Frames();
}

int SmushPlayer::getFrameCounter() {
	return _smushDecoder->getBlocky16Frames();
}

void SmushPlayer::handleFrame() {
	// Force the last frame to stay in place for it's duration:
	if (_videoDecoder->endOfVideo() && _videoDecoder->getTime() >= (uint32)_videoDecoder->getDuration().msecs()) {
		// If we're not supposed to loop (or looping fails) then end the video
		if (!_videoLooping) {
			_videoFinished = true;
			g_grim->setMode(GrimEngine::NormalMode);
			deinit();
			return;
		} else {
			if (!_currentVideoIsTheora) {
				_smushDecoder->rewind(); // This doesn't handle if looping fails.
				_smushDecoder->start();
			}
		}
	}
}

void SmushPlayer::handleTimer() {
	if (_smushTimer)
		_smushTimer->timerThread();
}

void SmushPlayer::deinit() {
	// The disasm stops the sound only if the movie is cut, not when it ends by itself
	bool wasPlaying = !_videoFinished;

	if (_smushAnim)
		_smushAnim->anmStop();

	MoviePlayer::deinit();

	if (wasPlaying && _smushSound)
		_smushSound->souEngineStopAll();
}

void SmushPlayer::postHandleFrame() {
	if (_demo && !_currentVideoIsTheora) {
		_x = _smushDecoder->getX();
		_y = _smushDecoder->getY();
	}
}

void SmushPlayer::restore(SaveGame *state) {
	if (isPlaying() && !_currentVideoIsTheora) {
		if (_smushAnim) {
			_frame = -1;
			_movieTime = 0;
		} else {
			_smushDecoder->seek((uint32)_movieTime);
		}

		_smushDecoder->start();
		timerCallback(this);
	}
}

} // end of namespace Grim
