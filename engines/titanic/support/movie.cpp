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

#include "video/avi_decoder.h"
#include "titanic/support/movie.h"
#include "titanic/titanic.h"

namespace Titanic {

CMovie::CMovie() : ListItem(), _state(MOVIE_STOPPED), _field10(0),
		_field14(0) {
}

CMovie::~CMovie() {
	g_vm->_activeMovies.remove(this);
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
		_videoSurface(surface), _gameObject(nullptr), _endFrame(-1) {
	Video::AVIDecoder *decoder = new Video::AVIDecoder();
	_video = decoder;
	_field14 = 1;

	if (!_video->loadFile(name.getString()))
		error("Could not open video - %s", name.getString().c_str());
}

OSMovie::OSMovie(Common::SeekableReadStream *stream, CVideoSurface *surface) :
		_videoSurface(surface), _gameObject(nullptr), _endFrame(-1) {
	_video = new Video::AVIDecoder();
	if (!_video->loadStream(stream))
		error("Could not parse movie stream");
}

OSMovie::~OSMovie() {
	g_vm->_activeMovies.remove(this);
	delete _video;
}

void OSMovie::play(uint flags, CVideoSurface *surface) {
	uint endFrame = _video->getFrameCount();
	play(0, endFrame, 0, 0);
}

void OSMovie::play(uint startFrame, uint endFrame, int v3, bool v4) {
	warning("TODO: OSMovie::play properly");

	_video->start();
	_video->seekToFrame(startFrame);
	_endFrame = endFrame;

	g_vm->_activeMovies.push_back(this);
	_state = MOVIE_NONE;
}

void OSMovie::play(const Rect &rect, int v1, int v2) {
	warning("TODO: OSMovie::play 3");
}

void OSMovie::playClip(const Rect &rect, uint startFrame, uint endFrame) {
	warning("TODO: OSMovie::playClip");
}

void OSMovie::proc11() {
	warning("TODO: OSMovie::proc11");
}

void OSMovie::proc12(int v1, int v2, int frameNumber, int flags, CGameObject *obj) {
	warning("TODO: OSMovie::proc12");
}

void OSMovie::stop() {
	_video->stop();
	_state = MOVIE_STOPPED;
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

const Common::List<CMovieRangeInfo *> OSMovie::getMovieRangeInfo() const {
	warning("TODO: OSMovie::getMovieRangeInfo");
	return Common::List<CMovieRangeInfo *>();
}

void OSMovie::proc18() {
	warning("TODO: OSMovie::proc18");
}

int OSMovie::getFrame() {
	assert(_video);
	return _video->getCurFrame();
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
			if (_video->getCurFrame() >= _endFrame) {
				_video->stop();
				_state = MOVIE_FINISHED;
			} else if (_video->needsUpdate()) {
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

	if (_gameObject)
		_gameObject->makeDirty();
}

} // End of namespace Titanic
