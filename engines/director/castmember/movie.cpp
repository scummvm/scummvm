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
#include "director/movie.h"
#include "director/sprite.h"

#include "director/castmember/movie.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/lingo/lingo-the.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/window.h"
#include "director/lingo/lingo.h"

namespace Director {

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version)
		: FilmLoopCastMember(cast, castId, stream, version) {
	_type = kCastMovie;

	_enableScripts = _flags & 0x10;
	_linkedMovie = nullptr;

	if (debugChannelSet(2, kDebugLoading))
		_initialRect.debugPrint(2, "MovieCastMember(): rect:");
	debugC(2, kDebugLoading, "MovieCastMember(): flags: (%d 0x%04x)", _flags, _flags);
	debugC(2, kDebugLoading, "_looping: %d, _enableScripts %d, _enableSound: %d, _crop %d, _center: %d",
			_looping, _enableScripts, _enableSound, _crop, _center);

}

MovieCastMember::MovieCastMember(Cast *cast, uint16 castId, MovieCastMember &source)
	: FilmLoopCastMember(cast, castId, source) {
	_type = kCastMovie;

	_enableScripts = source._enableScripts;

	// the copy loads its own linked movie (this member owns it)
	_linkedMovie = nullptr;
	_score = nullptr;
	_loaded = false;
}

MovieCastMember::~MovieCastMember() {
	delete _linkedMovie;
	delete _embeddedLingoState;
	for (auto &it : _embeddedFrozenStates)
		delete it;
	_score = nullptr;	// Prevent use-after-free in Filmloop
}

Common::Array<Channel> *MovieCastMember::getSubChannels(Common::Rect &bbox, uint frame) {
	if (_needsReload) {
		_loaded = false;
		load();
	}

	// Composite the embedded score's live channels (frame ignored) so
	// script-driven changes show, unlike a film loop's fixed frames.
	Common::Rect widgetRect(bbox.width() ? bbox.width() : _initialRect.width(),
			bbox.height() ? bbox.height() : _initialRect.height());

	_subchannels.clear();

	if (!_score || _score->_channels.empty())
		return &_subchannels;

	bool needToScale = (bbox.width() != _initialRect.width() || bbox.height() != _initialRect.height());
	float scaleX = needToScale ? (float)bbox.width() / _initialRect.width() : 1.0f;
	float scaleY = needToScale ? (float)bbox.height() / _initialRect.height() : 1.0f;

	// channel 0 is the score's own frame channel; sprites start at 1
	for (uint i = 1; i < _score->_channels.size(); ++i) {
		Sprite *chanSprite = _score->_channels[i]->_sprite;
		if (!chanSprite || chanSprite->_castId.isNull())
			continue;

		Sprite src = *chanSprite;

		if (needToScale) {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) * scaleX + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) * scaleY + bbox.top;
			src._width = widgetRect.width();
			src._height = widgetRect.height();
			src._stretch = true;
		} else {
			src._startPoint.x = (src._startPoint.x - _initialRect.left) + bbox.left;
			src._startPoint.y = (src._startPoint.y - _initialRect.top) + bbox.top;
		}

		Channel chan(nullptr, &src);
		_subchannels.push_back(chan);
	}

	for (auto &iter : _subchannels)
		iter.replaceWidget();

	return &_subchannels;
}

void MovieCastMember::load() {
	if (_loaded)
		return;

	// A reload rebuilds the linked movie
	delete _linkedMovie;
	_linkedMovie = nullptr;
	_score = nullptr;

	_loaded = true;
	_needsReload = false;

	Common::String rawMoviePath = _cast->getLinkedPath(_castId);
	if (rawMoviePath.empty()) {
		warning("MovieCastMember::load(): No filename for linked movie in castId %d", _castId);
		return;
	}

	Common::Path moviePath = findMoviePath(rawMoviePath);
	if (moviePath.empty()) {
		warning("MovieCastMember::load(): Linked movie %s not found", rawMoviePath.c_str());
		return;
	}

	Common::SharedPtr<Archive> archive = g_director->openArchive(moviePath);
	if (!archive) {
		warning("MovieCastMember::load(): Failed to load archive at %s", moviePath.toString().c_str());
		return;
	}

	// The linked movie borrows the host's window, so it must not take over
	// the stage (resize, recolour, reset palette) like a normal movie does.
	_linkedMovie = new Movie(_cast->getMovie()->getWindow());
	_linkedMovie->_isEmbedded = true;
	_linkedMovie->_parentMovie = _cast->getMovie();
	_linkedMovie->setArchive(archive);
	_linkedMovie->loadArchive();
	_score = _linkedMovie->getScore();

	// scriptsEnabled off makes the linked movie a passive flipbook: its score
	// advances and its channels refresh, but it runs no Lingo of its own.
	_score->_haveInteractivity = _enableScripts;

	// resolve the sprites against the linked movie's own cast
	for (auto &frame : _score->_scoreCache) {
		for (auto &sprite : frame->_sprites) {
			if (sprite && !sprite->_castId.isNull())
				sprite->setCast(sprite->_castId, false);
		}
	}
}

void MovieCastMember::update() {
	if (!_loaded)
		load();
	if (!_linkedMovie)
		return;

	// Step the linked score once per host frame. Its renderFrame() is
	// short-circuited (see Score::renderFrame) to refresh channels without
	// drawing to the host window; the host composites them via getSubChannels().
	Score *score = _linkedMovie->getScore();

	// Scripts resolve context via getCurrentMovie(), so point the shared
	// window at the linked movie for the step, then restore. This lets its
	// go()/globals/events act on itself.
	Window *window = _linkedMovie->getWindow();
	Movie *hostMovie = window->getCurrentMovie();
	window->setCurrentMovie(_linkedMovie);

	// With scripts enabled, give the linked movie its own Lingo state for the
	// step so its go()/freeze does not block the host's scripts. With scripts
	// disabled it runs no Lingo, so the swap and input routing do not apply.
	if (_enableScripts) {
		if (!_embeddedLingoState)
			_embeddedLingoState = new LingoState;
		window->swapLingoState(_embeddedLingoState, _embeddedFrozenStates);
		g_lingo->switchStateFromWindow();
	}

	if (score->_playState != kPlayStarted)
		score->startPlay();

	// A per-frame go() in exitFrame leaves hasJump/frozen state set, so
	// step() never drains routed input. Drain here so the embedded movie's
	// mouse handlers (e.g. mouseUp) fire.
	if (_enableScripts && !_linkedMovie->_inputEventQueue.empty())
		g_lingo->processEvents(_linkedMovie->_inputEventQueue, true);

	score->step();

	if (_enableScripts) {
		window->swapLingoState(_embeddedLingoState, _embeddedFrozenStates);
		g_lingo->switchStateFromWindow();
	}
	window->setCurrentMovie(hostMovie);
}

void MovieCastMember::routeInputEvent(LEvent event, Common::Point hostPos, const Common::Rect &bbox) {
	if (!_linkedMovie)
		return;

	// Invert getSubChannels()'s scaling to map the click into the linked
	// movie's coordinate space.
	Common::Point p = hostPos;
	if (bbox.width() && bbox.height()) {
		p.x = (hostPos.x - bbox.left) * _initialRect.width() / bbox.width() + _initialRect.left;
		p.y = (hostPos.y - bbox.top) * _initialRect.height() / bbox.height() + _initialRect.top;
	}

	_linkedMovie->queueInputEvent(event, 0, p);
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

void MovieCastMember::setField(int field, const Datum &d) {
	switch (field) {
	case kTheCenter:
		_center = (bool)d.asInt();
		return;
	case kTheIdleHandlerPeriod:
		warning("STUB: MovieCastMember::setField(): idleHandlerPeriod not implemented");
		return;
	case kThePaletteMapping:
		warning("STUB: MovieCastMember::setField(): paletteMapping not implemented");
		return;
	case kTheScoreSelection:
		warning("STUB: MovieCastMember::setField(): scoreSelection not implemented");
		return;
	case kTheScriptsEnabled:
		_enableScripts = (bool)d.asInt();
		return;
	case kTheSound:
		_enableSound = (bool)d.asInt();
		return;
	case kTheUpdateLock:
		warning("STUB: MovieCastMember::setField(): updateLock not implemented");
		return;
	default:
		break;
	}

	CastMember::setField(field, d);
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
