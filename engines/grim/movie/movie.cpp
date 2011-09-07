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

#include "engines/grim/movie/movie.h"

#if !defined(USE_MPEG2) || !defined(USE_SMUSH) || !defined(USE_BINK)
#define NEED_NULLPLAYER
#endif

namespace Grim {

MoviePlayer *g_movie;

void MoviePlayer::pause(bool p) {
  _videoPause = p;
  g_system->getMixer()->pauseHandle(_soundHandle, p);
}

// Fallback for when USE_MPEG2 isnt defined, might want to do something similar
// for USE_BINK if that comes over from ScummVM

#ifdef NEED_NULLPLAYER
class NullPlayer : public MoviePlayer {
public:
	NullPlayer(const char* codecID) {
		warning("%s-playback not compiled in, but needed", codecID);
		_videoFinished = true; // Rigs all movies to be completed.
	} 
	~NullPlayer() {}
	bool play(const char* filename, bool looping, int x, int y) {return true;}
	void stop() {}
	void saveState(SaveGame *state) {}
	void restoreState(SaveGame *state) {}
private:
	static void timerCallback(void *ptr) {}
	void handleFrame() {}
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
MoviePlayer *CreateSmushPlayer() {
	return new NullPlayer("SMUSH");
}
#endif

#ifndef USE_BINK
MoviePlayer *CreateBinkPlayer() {
	return new NullPlayer("BINK");
}
#endif

} // end of namespace Grim
