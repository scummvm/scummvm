/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush.h"

#include "engines/grim/resource.h"
#include "engines/grim/grim.h"

#include "video/theora_decoder.h"

namespace Grim {

MoviePlayer *CreateSmushPlayer(bool demo) {
	return new SmushPlayer(demo);
}

SmushPlayer::~SmushPlayer() {
	delete _theoraDecoder;
}

SmushPlayer::SmushPlayer(bool demo) : MoviePlayer(), _demo(demo) {
	_smushDecoder = new SmushDecoder();
	_videoDecoder = _smushDecoder;
#if defined (USE_THEORADEC)
	_theoraDecoder = new Video::TheoraDecoder();
#else
	warning("VideoTheoraPlayer::initialize - Theora support not compiled in, video will be skipped");
	_theoraDecoder = nullptr;
#endif
	//_smushDecoder->setDemo(_demo);
}

bool SmushPlayer::loadFile(const Common::String &filename) {
	warning("Play video %s", filename.c_str());
	bool success = false;
	_videoDecoder = _smushDecoder;
	if (!_demo)
		success = _videoDecoder->loadStream(g_resourceloader->openNewStreamFile(filename.c_str()));
	else
		success = _videoDecoder->loadFile(filename);

	if (!success) {
#if defined (USE_THEORADEC)
		Common::String theoraFilename = "MoviesHD/" + filename;
		theoraFilename.erase(theoraFilename.size() - 4);
		theoraFilename += ".ogv";
		warning("Trying to open %s", theoraFilename.c_str());
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
				_smushDecoder->rewind(); // This doesnt handle if looping fails.
				_smushDecoder->start();
			}
		}
	}
}

void SmushPlayer::postHandleFrame() {
	if (_demo && !_currentVideoIsTheora) {
		_x = _smushDecoder->getX();
		_y = _smushDecoder->getY();
	}
}

void SmushPlayer::restore(SaveGame *state) {
	if (isPlaying() && !_currentVideoIsTheora) {
		_smushDecoder->seek((uint32)_movieTime);
		_smushDecoder->start();
		timerCallback(this);
	}
}

} // end of namespace Grim
