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

#include "sludge/sludge.h"
#include "sludge/event.h"
#include "sludge/fileset.h"
#include "sludge/graphics.h"
#include "sludge/movie.h"
#include "sludge/newfatal.h"
#include "sludge/sound.h"

#include "common/substream.h"
#include "video/mkv_decoder.h"
#include "graphics/blit.h"

namespace Sludge {

MovieStates movieIsPlaying = kMovieNothing;
int movieIsEnding = 0;
float movieAspect = 1.6F;

int playMovie(int fileNumber) {
	uint fsize;
	if (!(fsize = g_sludge->_resMan->openFileFromNum(fileNumber)))
		return fatal("playMovie(): Can't open movie");

	Video::MKVDecoder decoder;

	Common::SeekableReadStream *stream = g_sludge->_resMan->getData();
	Common::SeekableSubReadStream video(stream, stream->pos(), stream->pos() + fsize);

	if (decoder.loadStream(&video))
		movieIsPlaying = kMoviePlaying;

	decoder.start();

	warning("movieIsPlaying %d", movieIsPlaying);
	while (movieIsPlaying) {
		g_sludge->_evtMan->checkInput();
		if (g_sludge->_evtMan->quit())
			break;

		g_sludge->_evtMan->handleInput();

		if (decoder.isVideoLoaded()) {
			if (decoder.endOfVideo()) {
				warning("End of video");
				// Movie complete, so unload the movie
				break;
			} else if (decoder.needsUpdate()) {
				const Graphics::Surface *s = decoder.decodeNextFrame();
				if (s) {
					// Transfer the next frame
					assert(s->format.bytesPerPixel == 4);
					if (((uint)s->w != g_sludge->_gfxMan->getWinWidth()) || ((uint)s->h != g_sludge->_gfxMan->getWinHeight())) {
						Graphics::Surface *surf = g_system->lockScreen();
						Graphics::scaleBlit((byte*)surf->getPixels(), (const byte*)s->getPixels(), surf->pitch, s->pitch,
						                    g_sludge->_gfxMan->getWinWidth(), g_sludge->_gfxMan->getWinHeight(), s->w, s->h, s->format);
						g_system->unlockScreen();
					} else {
						g_system->copyRectToScreen(s->getPixels(), s->pitch, 0, 0, s->w, s->h);
					}
					g_system->updateScreen();
				} else {
					warning("s is false");
				}
			}
		}
	}

	movieIsPlaying = kMovieNothing;

	g_sludge->_resMan->finishAccess();
	setResourceForFatal(-1);
	return 0;
}

int stopMovie() {
	int r = movieIsPlaying;
	movieIsPlaying = kMovieNothing;
	return r;
}

int pauseMovie() {
#if 0
	if (movieIsPlaying == kMoviePlaying) {
		ALuint source = getSoundSource(movieAudioIndex);
		if (source) {

			alurePauseSource(source);

		}
		movieIsPlaying = kMoviePaused;
		fprintf(stderr, "** Pausing **\n");
	} else if (movieIsPlaying == kMoviePaused) {
		ALuint source = getSoundSource(movieAudioIndex);
		if (source) {

			alureResumeSource(source);

		}
		fprintf(stderr, "** Restarted movie ** sound: %d source: %d\n", movieSoundPlaying, source);
		movieIsPlaying = kMoviePlaying;
	}
#endif
	return movieIsPlaying;
}

} // End of namespace Sludge
