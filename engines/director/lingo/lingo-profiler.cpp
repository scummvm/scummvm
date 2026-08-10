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

#include "common/system.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-profiler.h"

namespace Director {

LingoProfiler::LingoProfiler() {
	_enabled = false;
	_full = false;
	_seq = 0;
	_maxEvents = 500000;
	_haveLast = false;
	_lastFrame = 0;
	_lastMovieId = 0;
	_lastMoviePtr = nullptr;
	_curMovieId = 0;

	_strings.push_back(Common::String());
	_intern[Common::String()] = 0;
}

uint32 LingoProfiler::intern(const Common::String &s) {
	Common::HashMap<Common::String, uint32>::iterator it = _intern.find(s);
	if (it != _intern.end())
		return it->_value;

	uint32 id = _strings.size();
	_strings.push_back(s);
	_intern[s] = id;
	return id;
}

const Common::String &LingoProfiler::internedName(uint32 id) const {
	if (id >= _strings.size())
		return _strings[0];
	return _strings[id];
}

void LingoProfiler::record(uint8 type, uint32 nameId) {
	if (_full)
		return;
	if (_events.size() >= _maxEvents) {
		_full = true;
		return;
	}

	// Walk to the current movie once and derive frame, movie and timestamp
	// from it, so the hot path avoids repeated pointer chases.
	Window *window = g_director ? g_director->getCurrentWindow() : nullptr;
	Movie *movie = window ? window->getCurrentMovie() : nullptr;
	uint32 frame = (movie && movie->getScore()) ? movie->getScore()->getCurrentFrameNum() : 0;

	// The interned movie id only changes when the movie does; recompute (which
	// copies the name and hits the hashmap) just on a movie switch.
	if ((const void *)movie != _lastMoviePtr) {
		_lastMoviePtr = movie;
		Common::String name = movie ? movie->getMacName() : Common::String();
		if (name.empty())
			name = movie ? "movie" : "?";
		_curMovieId = intern(name);
	}
	uint32 movieId = _curMovieId;

	uint32 ts = g_system->getMillis();

	if (!_haveLast || frame != _lastFrame || movieId != _lastMovieId) {
		ProfilerEvent fe;
		fe.type = kProfFrame;
		fe.seq = _seq++;
		fe.ts = ts;
		fe.frame = frame;
		fe.depth = 0;
		fe.nameId = 0;
		fe.movieId = movieId;
		_events.push_back(fe);

		_haveLast = true;
		_lastFrame = frame;
		_lastMovieId = movieId;
	}

	uint16 depth = 0;
	if (g_lingo && g_lingo->_state)
		depth = (uint16)g_lingo->_state->callstack.size();

	ProfilerEvent e;
	e.type = type;
	e.seq = _seq++;
	e.ts = ts;
	e.frame = frame;
	e.depth = depth;
	e.nameId = nameId;
	e.movieId = movieId;
	_events.push_back(e);
}

void LingoProfiler::onPushContext() {
	if (!_enabled)
		return;

	Common::String name("<anon>");
	if (g_lingo && g_lingo->_state && !g_lingo->_state->callstack.empty()) {
		Symbol &sym = g_lingo->_state->callstack.back()->sp;
		if (sym.name)
			name = *sym.name;
	}
	record(kProfBegin, intern(name));
}

void LingoProfiler::onPopContext() {
	if (!_enabled)
		return;
	record(kProfEnd, 0);
}

void LingoProfiler::onFreeze() {
	if (!_enabled)
		return;
	record(kProfFreeze, 0);
}

void LingoProfiler::onThaw() {
	if (!_enabled)
		return;
	record(kProfThaw, 0);
}

void LingoProfiler::clear() {
	_events.clear();
	_strings.clear();
	_intern.clear();
	_strings.push_back(Common::String());
	_intern[Common::String()] = 0;
	_seq = 0;
	_full = false;
	_haveLast = false;
	_lastFrame = 0;
	_lastMovieId = 0;
	_lastMoviePtr = nullptr;
	_curMovieId = 0;
}

} // End of namespace Director
