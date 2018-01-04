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

#include "titanic/support/movie.h"
#include "titanic/core/game_object.h"
#include "titanic/events.h"
#include "titanic/messages/messages.h"
#include "titanic/support/avi_surface.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

#define CLIP_WIDTH 600
#define CLIP_WIDTH_REDUCED (CLIP_WIDTH / 2)
#define CLIP_HEIGHT 340
#define CLIP_HEIGHT_REDUCED (CLIP_HEIGHT / 2)

CMovieList *CMovie::_playingMovies;
CVideoSurface *CMovie::_movieSurface;

CMovie::CMovie() : ListItem(), _handled(false), _hasVideoFrame(false) {
}

CMovie::~CMovie() {
	removeFromPlayingMovies();
}

void CMovie::init() {
	_playingMovies = new CMovieList();
	_movieSurface = nullptr;
}

void CMovie::deinit() {
	// At this point, there shouldn't be any playing movies left,
	// since their owning objects should have freed them
	assert(_playingMovies->empty());
	delete _playingMovies;
	delete _movieSurface;
}

void CMovie::addToPlayingMovies() {
	if (!isActive())
		_playingMovies->push_back(this);
}

void CMovie::removeFromPlayingMovies() {
	_playingMovies->remove(this);
}

bool CMovie::isActive() const {
	return _playingMovies->contains(this);
}

bool CMovie::hasVideoFrame() {
	if (_hasVideoFrame) {
		_hasVideoFrame = 0;
		return true;
	} else {
		return false;
	}
}

/*------------------------------------------------------------------------*/

OSMovie::OSMovie(const CResourceKey &name, CVideoSurface *surface) :
		_aviSurface(name), _videoSurface(surface) {
	_field18 = 0;
	_field24 = 0;
	_field28 = 0;
	_field2C = 0;

	surface->resize(_aviSurface.getWidth(), _aviSurface.getHeight());
	_aviSurface.setVideoSurface(surface);
}

OSMovie::~OSMovie() {
}

void OSMovie::play(uint flags, CGameObject *obj) {
	_aviSurface.play(flags, obj);

	if (_aviSurface.isPlaying())
		movieStarted();
}

void OSMovie::play(uint startFrame, uint endFrame, uint flags, CGameObject *obj) {
	_aviSurface.play(startFrame, endFrame, flags, obj);

	if (_aviSurface.isPlaying())
		movieStarted();
}

void OSMovie::play(uint startFrame, uint endFrame, uint initialFrame, uint flags, CGameObject *obj) {
	_aviSurface.play(startFrame, endFrame, initialFrame, flags, obj);

	if (_aviSurface.isPlaying())
		movieStarted();
}

bool OSMovie::playCutscene(const Rect &drawRect, uint startFrame, uint endFrame) {
	if (!_movieSurface)
		_movieSurface = CScreenManager::_screenManagerPtr->createSurface(600, 340, 32);

	// Set a new event target whilst the clip plays, so standard scene drawing isn't called
	CEventTarget eventTarget;
	g_vm->_events->addTarget(&eventTarget);

	bool result = _aviSurface.playCutscene(drawRect, startFrame, endFrame);

	g_vm->_events->removeTarget();
	return result;
}

void OSMovie::pause() {
	_aviSurface.pause();
}

void OSMovie::stop() {
	_aviSurface.stop();
	removeFromPlayingMovies();
}

void OSMovie::addEvent(int frameNumber, CGameObject *obj) {
	if (_aviSurface.addEvent(&frameNumber, obj)) {
		CMovieFrameMsg frameMsg(frameNumber, 0);
		frameMsg.execute(obj);
	}
}

void OSMovie::setFrame(uint frameNumber) {
	_aviSurface.setFrame(frameNumber);
	_videoSurface->setTransparencySurface(_aviSurface.getSecondarySurface());
}

bool OSMovie::handleEvents(CMovieEventList &events) {
	// WORKAROUND: If a movie is paused as part of initial
	// scene loading, now's the time to un-pause it
	_aviSurface.resume();

	if (!_aviSurface.isPlaying())
		return false;

	// Handle updating the frame
	while (_aviSurface.isPlaying() && _aviSurface.isNextFrame()) {
		_aviSurface.handleEvents(events);
		_videoSurface->setTransparencySurface(_aviSurface.getSecondarySurface());

		// Flag there's a video frame
		_hasVideoFrame = true;
	}

	return _aviSurface.isPlaying();
}

const CMovieRangeInfoList *OSMovie::getMovieRangeInfo() const {
	return _aviSurface.getMovieRangeInfo();
}

void OSMovie::setSoundManager(CSoundManager *soundManager) {
	_aviSurface._soundManager = soundManager;
}

int OSMovie::getFrame() const {
	return _aviSurface.getFrame();
}

void OSMovie::movieStarted() {
	//if (_aviSurface._hasAudio)
	//	_aviSurface._soundManager->movieStarted();

	// Register the movie in the playing list
	addToPlayingMovies();
	_hasVideoFrame = true;
}

void OSMovie::setFrameRate(double rate) {
	_aviSurface.setFrameRate(rate);
}

void OSMovie::setPlaying(bool playingFlag) {
	_aviSurface.setPlaying(playingFlag);
}

Graphics::ManagedSurface *OSMovie::duplicateTransparency() const {
	return _aviSurface.duplicateTransparency();
}

} // End of namespace Titanic
