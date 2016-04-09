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

#include "image/codecs/cinepak.h"
#include "titanic/support/avi_decoder.h"
#include "titanic/support/movie.h"
#include "titanic/titanic.h"

namespace Titanic {

CMovie::CMovie() : ListItem(), _state(MOVIE_STOPPED), _field10(0) {
}

bool CMovie::isActive() const {
	return g_vm->_activeMovies.contains(this);
}

bool CMovie::get10() {
	if (_field10) {
		_field10 = 0;
		return true;
	} else {
		return false;
	}
}

/*------------------------------------------------------------------------*/

OSMovie::OSMovie(const CResourceKey &name, CVideoSurface *surface) :
		_videoSurface(surface), _gameObject(nullptr) {
	_video = new AVIDecoder();
	if (!_video->loadFile(name.getString()))
		error("Could not open video - %s", name.getString().c_str());
}

OSMovie::~OSMovie() {
	g_vm->_activeMovies.remove(this);
	delete _video;
}

void OSMovie::proc8(int v1, CVideoSurface *surface) {
	warning("TODO: OSMovie::proc8");
}

void OSMovie::proc9(int v1, int v2, int v3, bool v4) {
	warning("TODO: OSMovie::proc9");
	//setFrame(v1); ?
	_video->start();
	g_vm->_activeMovies.push_back(this);
	_state = MOVIE_NONE;
}

void OSMovie::proc10() {
	warning("TODO: OSMovie::proc10");
}

void OSMovie::proc11() {
	warning("TODO: OSMovie::proc11");
}

void OSMovie::proc12() {
	warning("TODO: OSMovie::proc12");
}

void OSMovie::stop() {
	warning("TODO: OSMovie::proc13");
}

void OSMovie::proc14() {
	warning("TODO: OSMovie::proc14");
}

void OSMovie::setFrame(uint frameNumber) {
	_video->seekToFrame(frameNumber);
	decodeFrame();
}

void OSMovie::proc16() {
	warning("TODO: OSMovie::proc16");
}

void OSMovie::proc17() {
	warning("TODO: OSMovie::proc17");
}

void OSMovie::proc18() {
	warning("TODO: OSMovie::proc18");
}

int OSMovie::proc19() {
	warning("TODO: OSMovie::proc19");
	return 0;
}

void OSMovie::proc20() {
	warning("TODO: OSMovie::proc20");
}

void *OSMovie::proc21() {
	warning("TODO: OSMovie::proc21");
	return nullptr;
}

MovieState OSMovie::getState() {
	if (!_video)
		_state = MOVIE_STOPPED;
	return _state;
}

void OSMovie::update() {
	if (_state != MOVIE_STOPPED) {
		if (_video->isPlaying()) {
			if (_video->needsUpdate()) {
				decodeFrame();
				_state = MOVIE_FRAME;
			} else {
				_state = MOVIE_NONE;
			}
		} else {
			_state = MOVIE_STOPPED;
		}
	}
}

void OSMovie::decodeFrame() {
	const Graphics::Surface *frame = _video->decodeNextFrame();
	OSVideoSurface *videoSurface = static_cast<OSVideoSurface *>(_videoSurface);
	assert(videoSurface);

	// If the video surface doesn't yet have an underlying surface, create it
	if (!videoSurface->hasSurface())
		videoSurface->resize(frame->w, frame->h);

	// Lock access to the surface
	videoSurface->lock();
	assert(videoSurface->_rawSurface);

	if (frame->format == videoSurface->_rawSurface->format) {
		// Matching format, so we can copy straight from the video frame
		videoSurface->_rawSurface->blitFrom(*frame);
	} else {
		// Different formats so we have to convert it first
		Graphics::Surface *s = frame->convertTo(videoSurface->_rawSurface->format);
		videoSurface->_rawSurface->blitFrom(*s);

		s->free();
		delete s;
	}

	// Unlock the surface
	videoSurface->unlock();
}

} // End of namespace Titanic
