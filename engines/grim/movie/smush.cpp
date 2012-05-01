/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush.h"
#include "engines/grim/movie/codecs/vima.h"

#include "engines/grim/resource.h"
#include "engines/grim/grim.h"

namespace Grim {

MoviePlayer *CreateSmushPlayer(bool demo) {
	return new SmushPlayer(demo);
}

SmushPlayer::SmushPlayer(bool demo) : MoviePlayer(), _demo(demo) {
	_smushDecoder = new SmushDecoder();
	_videoDecoder = _smushDecoder;
	_smushDecoder->setDemo(_demo);
}

bool SmushPlayer::loadFile(Common::String filename) {
	if (!_demo)
		return _videoDecoder->loadStream(g_resourceloader->openNewStreamFile(filename.c_str()));
	else
		return _videoDecoder->loadFile(filename);
}

void SmushPlayer::init() {
	if (_demo) {
		_x = _smushDecoder->getX();
		_y = _smushDecoder->getY();
	} else {
		_smushDecoder->setLooping(_videoLooping);
	}
	MoviePlayer::init();
}

void SmushPlayer::handleFrame() {
	if (_videoDecoder->endOfVideo()) {
		// If we're not supposed to loop (or looping fails) then end the video
		if (!_videoLooping ) {
			_videoFinished = true;
			g_grim->setMode(GrimEngine::NormalMode);
			deinit();
			return;
		} else {
// 			getDecoder()->rewind(); // This doesnt handle if looping fails.
		}
	}
}

void SmushPlayer::postHandleFrame() {
	if (_demo) {
		_x = _smushDecoder->getX();
		_y = _smushDecoder->getY();
	}
}

void SmushPlayer::restoreState(SaveGame *state) {
	MoviePlayer::restoreState(state);
	if (isPlaying()) {
		_smushDecoder->seekToTime((uint32)_movieTime); // Currently not fully working (out of synch)
	}
}

} // end of namespace Grim
