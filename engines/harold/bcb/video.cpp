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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

namespace BCB {

bool BCBEngine::playNumberedMovie(int movieId, bool blankScreenMovie) {
	stopSceneIdleMovie();
	stopActiveMovie();
	_sceneChoiceActive = false;
	_mapControlsActive = false;
	_activeMovieOnBlankScreen = blankScreenMovie;
	loadTimedCaptionRecord(movieId);

	const Common::String movieName = Common::String::format("mov%04d.avi", movieId);
	Common::SeekableReadStream *stream = createMovieReadStream(movieId);
	if (!stream) {
		warning("BCBEngine::playNumberedMovie(): Missing %s", movieName.c_str());
		stream = createMovieReadStream(0);
	}
	if (!stream)
		return false;

	_activeMovieVideo = new Video::AVIDecoder();
	_activeMovieVideo->setSoundType(Audio::Mixer::kSFXSoundType);
	if (!_activeMovieVideo->loadStream(stream)) {
		delete _activeMovieVideo;
		_activeMovieVideo = nullptr;
		return false;
	}

	_activeMovieVideo->setOutputPixelFormat(g_system->getScreenFormat());
	_activeMovieVideo->start();
	_activeMoviePaused = false;
	_activeMovieDispatchOnFinish = true;
	return true;
}

Common::Point BCBEngine::getActiveMoviePosition() const {
	if (_currentDisplayState == 0)
		return Common::Point(0x78, 0x64);
	if (_currentDisplayState == 8)
		return Common::Point(0x78, 0x38);
	return Common::Point(0x10, 0x30);
}

void BCBEngine::stopActiveMovie() {
	if (!_activeMovieVideo)
		return;

	_activeMovieVideo->stop();
	delete _activeMovieVideo;
	_activeMovieVideo = nullptr;
	_activeMovieOnBlankScreen = false;
	_activeMoviePaused = false;
}

void BCBEngine::stopActiveMovieAndDispatch() {
	if (!_activeMovieVideo)
		return;

	const bool dispatchAfterStop = _activeMovieDispatchOnFinish;
	stopActiveMovie();
	playUiSound();
	if (!dispatchAfterStop)
		return;

	_activeMovieDispatchOnFinish = false;
	if (_currentDisplayState == 7 || _currentDisplayState == 8) {
		processQueuedMovieSequence();
	} else if (_currentDisplayState == 1) {
		drawMainSceneScreen(false);
		processGameStateDispatcher(true);
	} else {
		if (_currentDisplayState == 2)
			drawClockAndChipIndicator();
		processGameStateDispatcher(true);
	}
}

bool BCBEngine::processPendingMovieRequest() {
	const int movieId = stateDword(kStateMovieToPlayId);
	if (movieId == 0)
		return false;

	playNumberedMovie(movieId);

	uint32 &movieHistoryCount = stateDword(kStateMovieHistoryCount);
	stateDword(kStateMovieHistory + MIN<uint32>(movieHistoryCount, 15) * 4) = movieId;
	if (movieHistoryCount < 15)
		++movieHistoryCount;

	stateDword(kStateMovieToPlayId) = 0;
	return true;
}

void BCBEngine::playUiSound() {
	Common::SeekableReadStream *stream = createProgramReadStream("se.wav");
	if (!stream)
		return;

	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!wavStream)
		return;

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_uiSoundHandle, wavStream, -1, Audio::Mixer::kMaxChannelVolume);
}

void BCBEngine::startMenuBackgroundMusic() {
	stopMenuBackgroundMusic();

	Common::SeekableReadStream *stream = createDiscReadStream(1, "bgm/system.avi");
	if (!stream)
		return;

	_menuBackgroundVideo = new Video::AVIDecoder();
	_menuBackgroundVideo->setSoundType(Audio::Mixer::kMusicSoundType);
	if (!_menuBackgroundVideo->loadStream(stream)) {
		delete _menuBackgroundVideo;
		_menuBackgroundVideo = nullptr;
		return;
	}

	_menuBackgroundVideo->decodeNextFrame();
	_menuBackgroundVideo->start();
}

void BCBEngine::stopMenuBackgroundMusic() {
	if (_menuBackgroundVideo) {
		delete _menuBackgroundVideo;
		_menuBackgroundVideo = nullptr;
	}
}

void BCBEngine::startGameOverMusic() {
	stopGameOverMusic();

	Common::SeekableReadStream *stream = createActiveDiscReadStream("bgm/gameover.avi");
	if (!stream) {
		warning("BCBEngine::startGameOverMusic(): Missing gameover.avi for active media disc %d", _activeMediaDiscNumber);
		return;
	}

	_gameOverVideo = new Video::AVIDecoder();
	_gameOverVideo->setSoundType(Audio::Mixer::kMusicSoundType);
	if (!_gameOverVideo->loadStream(stream)) {
		warning("BCBEngine::startGameOverMusic(): Could not load gameover.avi");
		delete _gameOverVideo;
		_gameOverVideo = nullptr;
		return;
	}

	_gameOverVideo->decodeNextFrame();
	_gameOverVideo->start();
}

void BCBEngine::stopGameOverMusic() {
	if (!_gameOverVideo)
		return;

	_gameOverVideo->stop();
	delete _gameOverVideo;
	_gameOverVideo = nullptr;
}

void BCBEngine::startSceneIdleMovie() {
	if (_sceneIdleVideo || _activeMovieVideo || _currentDisplayState != 1)
		return;

	const char *idleMoviePath = stateDword(kStateMainSceneNightFlag) != 0 ? "bgm/night.avi" : "bgm/day.avi";
	Common::SeekableReadStream *stream = nullptr;
	for (int discNumber = 1; discNumber <= 5 && !stream; ++discNumber)
		stream = createDiscReadStream(discNumber, idleMoviePath);
	if (!stream)
		return;

	_sceneIdleVideo = new Video::AVIDecoder();
	_sceneIdleVideo->setSoundType(Audio::Mixer::kMusicSoundType);
	if (!_sceneIdleVideo->loadStream(stream)) {
		delete _sceneIdleVideo;
		_sceneIdleVideo = nullptr;
		return;
	}

	_sceneIdleVideo->setOutputPixelFormat(g_system->getScreenFormat());
	_sceneIdleVideo->start();
	_sceneIdleMovieTimerTicks = 0;
}

void BCBEngine::stopSceneIdleMovie() {
	if (!_sceneIdleVideo)
		return;

	_sceneIdleVideo->stop();
	delete _sceneIdleVideo;
	_sceneIdleVideo = nullptr;
}

void BCBEngine::processQueuedMovieSequence() {
	error("BCB: processQueuedMovieSequence is not implemented");
}

} // End of namespace BCB
