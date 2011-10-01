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

#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush.h"

#include "engines/grim/savegame.h"

#include "engines/grim/grim.h"

#ifdef USE_SMUSH

namespace Grim {

MoviePlayer *CreateSmushPlayer() {
	return new SmushPlayer();
}

SmushPlayer::SmushPlayer() : MoviePlayer() {
	g_movie = this;
	_speed = 5000;
	_videoDecoder = new Grim::SmushDecoder();
}

SmushPlayer::~SmushPlayer() {
	deinit();
	delete _videoDecoder;
}

SmushDecoder* SmushPlayer::getDecoder() {
	return dynamic_cast<SmushDecoder*>(_videoDecoder);
}

void SmushPlayer::init() {
	if (g_grim->getGameFlags() & ADGF_DEMO) {
		_x = getDecoder()->getX();
		_y = getDecoder()->getY();
	} else {
		getDecoder()->setLooping(_videoLooping);
	}
	MoviePlayer::init();
}

void SmushPlayer::handleFrame() {
	if(!basicHandleFrame())
		return;

	if (g_grim->getGameFlags() & ADGF_DEMO) {
		_x = getDecoder()->getX();
		_y = getDecoder()->getY();
	}

	if (_videoDecoder->endOfVideo()) {
		// If we're not supposed to loop (or looping fails) then end the video
		if (!_videoLooping ) {
			_videoFinished = true;
			g_grim->setMode(ENGINE_MODE_NORMAL);
			deinit();
			return;
		} else {
			getDecoder()->rewind(); // This doesnt handle if looping fails.
		}
	}
}

void SmushPlayer::restoreState(SaveGame *state) {
	MoviePlayer::restoreState(state);
	getDecoder()->seekToTime((uint32)_movieTime); // Currently not fully working (out of synch)
}

} // end of namespace Grim

#endif // USE_SMUSH
