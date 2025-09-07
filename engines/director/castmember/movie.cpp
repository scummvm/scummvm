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

#include "director/director.h"
#include "director/debugger.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/sprite.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/bitmap.h"

#include "director/castmember/movie.h"

#include "director/lingo/lingo-the.h"

namespace Director {

// This SubWindow will not be responsible for rendering the movie cast member
// But we still need the event processing
SubWindow::SubWindow(Window *parent, Common::Rect rect)
		: Window(g_director->getMacWindowManager()->getNextId(), false, false, false, g_director->getMacWindowManager(), g_director, false), _parent(parent) {
	_parent = parent;
}

void SubWindow::setAsCurrent() {
	_parent->setAsCurrent();
}

bool SubWindow::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	debugC(3, kDebugMovieCast, "MovieCastMember::SubWindow: render: Movie cast member is not allowed to render");
	return false;
}

void SubWindow::setStageColor(uint32 stageColor, bool forceReset) {
	debugC(3, kDebugMovieCast, "MovieCastMember::SubWindow: setStageColor: Marking the rect dirty in the main window");
}

uint32 SubWindow::getStageColor() {
	return _parent->getStageColor();
}

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: CastMember(cast, castId, stream) {
	_type = kCastMovie;

	_enableScripts = true;

	// We are ignoring some of the bits in the flags
	if (cast->_version >= kFileVer400 && cast->_version < kFileVer500) {
		_initialRect = Movie::readRect(stream);
		uint32 flags = stream.readUint32BE();
		uint16 unk1 = stream.readUint16BE();
		debugC(5, kDebugLoading, "FilmLoopCastMember::FilmLoopCastMember(): flags: %d, unk1: %d", flags, unk1);
		_looping = flags & 64 ? 0 : 1;
		_enableSound = flags & 8 ? 1 : 0;
		_crop = flags & 2 ? 0 : 1;
		_center = flags & 1 ? 1 : 0;
	} else if (cast->_version >= kFileVer500 && cast->_version < kFileVer600) {
		_initialRect = Movie::readRect(stream);
		uint32 flags = stream.readUint32BE();
		uint16 unk1 = stream.readUint16BE();
		debugC(5, kDebugLoading, "FilmLoopCastMember::FilmLoopCastMember(): flags: %d, unk1: %d", flags, unk1);
		_looping = flags & 32 ? 0 : 1;
		_enableSound = flags & 8 ? 1 : 0;
		_crop = flags & 2 ? 0 : 1;
		_center = flags & 1 ? 1 : 0;
	}

	_window = new SubWindow(g_director->getCurrentWindow(), _initialRect);
	_window->incRefCount();
	_movie = nullptr;

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "MovieCastMember(): rect:");
	debugC(2, kDebugLoading, "MovieCastMember(): flags: (%d 0x%04x)", _flags, _flags);
	debugC(2, kDebugLoading, "_looping: %d, _enableScripts %d, _enableSound: %d, _crop %d, _center: %d",
			_looping, _enableScripts, _enableSound, _crop, _center);
}

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, MovieCastMember &source)
	: CastMember(cast, castId) {
	_type = kCastMovie;

	_enableScripts = source._enableScripts;
}

Common::Array<Channel> *MovieCastMember::getSubChannels(Common::Rect &bbox) {
	Common::Rect widgetRect(bbox.width() ? bbox.width() : _initialRect.width(), bbox.height() ? bbox.height() : _initialRect.height());

	if (_needsReload) {
		_loaded = false;
		load();
	}

	_subchannels.clear();

	// Since the main movie and the linked movie cast member share lingo execution, it is global
	// We need to make sure that the movie that is processing lingo scripts is the main Window current movie
	Movie *mainMovie = _window->getParent()->getCurrentMovie();
	_window->getParent()->setCurrentMovie(_movie);
	_movie->getScore()->step();
	_movie->getScore()->loadFrame(_movie->getScore()->_curFrameNumber, true);
	_window->getParent()->setCurrentMovie(mainMovie);

	debugC(3, kDebugMovieCast, "MovieCastMember::getSubChannels():: Current Frame number of movie %s: %d, fps: %d", _filename.toString().c_str(), _movie->getScore()->_curFrameNumber, _movie->getScore()->_currentFrameRate);

	Common::Array<Sprite *> sprites = _movie->getScore()->_currentFrame->_sprites;

	// copy the sprites in order to the list
	for (auto src: sprites) {
		if (!src->_cast)
			continue;
		// translate sprite relative to the global bounding box
		int16 relX = (src->_startPoint.x - _initialRect.left) * widgetRect.width() / _initialRect.width();
		int16 relY = (src->_startPoint.y - _initialRect.top) * widgetRect.height() / _initialRect.height();
		int16 absX = relX + bbox.left;
		int16 absY = relY + bbox.top;
		int16 width = src->_width * widgetRect.width() / _initialRect.width();
		int16 height = src->_height * widgetRect.height() / _initialRect.height();

		// Re-inject the translated position into the Sprite.
		// This saves the hassle of having to force the Channel to be in puppet mode.
		src->_width = width;
		src->_height = height;
		src->_startPoint = Common::Point(absX, absY);
		src->_stretch = true;

		// Film loop frames are constructed as a series of Channels, much like how a normal frame
		// is rendered by the Score. We don't include a pointer to the current Score here,
		// that's only for querying the constraint channel which is not used.
		Channel chan(_movie->getScore(), src);
		_subchannels.push_back(chan);
	}
	// Initialise the widgets on all of the subchannels.
	// This has to be done once the list has been constructed, otherwise
	// the list grow operation will erase the widgets as they aren't
	// part of the Channel assignment constructor.
	for (auto &iter : _subchannels) {
		iter.replaceWidget();
	}

	return &_subchannels;
}

CastMemberID MovieCastMember::getSubChannelSound1() {
	warning("STUB: MovieCastMember:: getSubChannelSound1(): Not implemented");
	return CastMemberID();
}

CastMemberID MovieCastMember::getSubChannelSound2() {
	warning("STUB: MovieCastMember:: getSubChannelSound2(): Not implemented");
	return CastMemberID();
}

void MovieCastMember::load() {
	debug("Called MovieCastMember::load on cast id: %d", _castId);
	if (_loaded) {
		debug("Called twice but returned");
		return;
	}

	if (_filename.empty()) {
		debugC(5, kDebugLoading, "MovieCastMember::load(): load called on MovieCastMember before filename was set");
		return;
	}

	debugC(6, kDebugLoading, "MovieCastMember::load(): loading archive: %s", _filename.toString().c_str());

	Common::String pathString = _filename.toString();
	pathString.trim();
	pathString += g_director->getCurrentPath();
	Common::Path resPath = findPath(pathString, true, true, false);

	_archive = g_director->openArchive(resPath);

	if (_movie) {
		delete _movie;
	}

	_movie = new Movie(_window);
	_window->setCurrentMovie(_movie);

	_movie->setArchive(_archive);
	_movie->loadArchive();
	_movie->_isCastMember = true;

	_movie->getScore()->startPlay();
	_movie->getScore()->setCurrentFrame(1);

	g_debugger->movieHook();

	_loaded = true;
	_needsReload = false;
}

bool MovieCastMember::hasField(int field) {
	switch (field) {
	case kTheCenter:
	case kTheIdleHandlerPeriod:
	case kThePaletteMapping:
	case kTheScoreSelection:
	case kTheScriptsEnabled:
	case kTheSound:
	case kTheUpdateLock:
		return true;
	default:
		break;
	}
	return CastMember::hasField(field);
}

Datum MovieCastMember::getField(int field) {
	Datum d;

	switch (field) {
	case kTheCenter:
		d = Datum((int)_center);
		break;
	case kTheIdleHandlerPeriod:
		warning("STUB: MovieCastMember::getField(): idleHandlerPeriod not implemented");
		break;
	case kThePaletteMapping:
		warning("STUB: MovieCastMember::getField(): paletteMapping not implemented");
		break;
	case kTheScoreSelection:
		warning("STUB: MovieCastMember::getField(): scoreSelection not implemented");
		break;
	case kTheScriptsEnabled:
		d = Datum(_enableScripts);
		break;
	case kTheSound:
		d = Datum(_enableSound);
		break;
	case kTheUpdateLock:
		warning("STUB: MovieCastMember::getField(): updateLock not implemented");
		break;
	default:
		d = CastMember::getField(field);
		break;
	}

	return d;
}

bool MovieCastMember::isModified() {
	if (_frames.size())
		return true;

	if (_initialRect.width() && _initialRect.height())
		return true;

	return false;
}

bool MovieCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheCenter:
		_center = (bool)d.asInt();
		return true;
	case kTheIdleHandlerPeriod:
		warning("STUB: MovieCastMember::setField(): idleHandlerPeriod not implemented");
		return false;
	case kThePaletteMapping:
		warning("STUB: MovieCastMember::setField(): paletteMapping not implemented");
		return false;
	case kTheScoreSelection:
		warning("STUB: MovieCastMember::setField(): scoreSelection not implemented");
		return false;
	case kTheScriptsEnabled:
		_enableScripts = (bool)d.asInt();
		return true;
	case kTheSound:
		_enableSound = (bool)d.asInt();
		return true;
	case kTheUpdateLock:
		warning("STUB: MovieCastMember::setField(): updateLock not implemented");
		return false;
	case kTheFileName:
		_filename = Common::Path(d.asString());
		break;default:
		break;
	}

	return CastMember::setField(field, d);
}

Common::String MovieCastMember::formatInfo() {
	return Common::String::format(
		"initialRect: %dx%d@%d,%d, boundingRect: %dx%d@%d,%d, enableScripts: %d, enableSound: %d, looping: %d, crop: %d, center: %d",
		_initialRect.width(), _initialRect.height(),
		_initialRect.left, _initialRect.top,
		_boundingRect.width(), _boundingRect.height(),
		_boundingRect.left, _boundingRect.top,
		_enableScripts, _enableSound, _looping,
		_crop, _center
	);
}

} // End of namespace Director
