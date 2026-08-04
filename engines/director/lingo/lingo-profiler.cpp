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

#include "common/file.h"
#include "common/path.h"

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

uint32 LingoProfiler::currentFrame() const {
	if (!g_director)
		return 0;
	Window *window = g_director->getCurrentWindow();
	if (!window)
		return 0;
	Movie *movie = window->getCurrentMovie();
	if (!movie || !movie->getScore())
		return 0;
	return movie->getScore()->getCurrentFrameNum();
}

Common::String LingoProfiler::currentMovieName() const {
	if (!g_director)
		return Common::String("?");
	Window *window = g_director->getCurrentWindow();
	if (!window)
		return Common::String("?");
	Movie *movie = window->getCurrentMovie();
	if (!movie)
		return Common::String("?");
	Common::String name = movie->getMacName();
	return name.empty() ? Common::String("movie") : name;
}

void LingoProfiler::record(uint8 type, uint32 nameId) {
	if (_full)
		return;
	if (_events.size() >= _maxEvents) {
		_full = true;
		return;
	}

	uint32 frame = currentFrame();
	uint32 movieId = intern(currentMovieName());

	if (!_haveLast || frame != _lastFrame || movieId != _lastMovieId) {
		ProfilerEvent fe;
		fe.type = kProfFrame;
		fe.seq = _seq++;
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
}

static Common::String jsonEscape(const Common::String &s) {
	Common::String out;
	for (uint i = 0; i < s.size(); i++) {
		char c = s[i];
		if (c == '"' || c == '\\') {
			out += '\\';
			out += c;
		} else if (c == '\n') {
			out += "\\n";
		} else if (c == '\t') {
			out += "\\t";
		} else if ((byte)c < 0x20) {
		} else {
			out += c;
		}
	}
	return out;
}

bool LingoProfiler::exportChromeTrace(const Common::Path &path) {
	Common::DumpFile out;
	if (!out.open(path))
		return false;

	out.writeString("{\"traceEvents\":[\n");

	for (uint32 id = 1; id < _strings.size(); id++) {
		Common::String line = Common::String::format(
			"{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":1,\"tid\":%u,\"args\":{\"name\":\"%s\"}},\n",
			id, jsonEscape(_strings[id]).c_str());
		out.writeString(line);
	}

	for (uint i = 0; i < _events.size(); i++) {
		const ProfilerEvent &e = _events[i];
		Common::String line;
		switch (e.type) {
		case kProfBegin:
			line = Common::String::format(
				"{\"name\":\"%s\",\"ph\":\"B\",\"ts\":%u,\"pid\":1,\"tid\":%u},\n",
				jsonEscape(internedName(e.nameId)).c_str(), e.seq, e.movieId);
			break;
		case kProfEnd:
			line = Common::String::format(
				"{\"ph\":\"E\",\"ts\":%u,\"pid\":1,\"tid\":%u},\n", e.seq, e.movieId);
			break;
		case kProfFrame:
			line = Common::String::format(
				"{\"name\":\"frame %u\",\"ph\":\"i\",\"ts\":%u,\"pid\":1,\"tid\":%u,\"s\":\"g\"},\n",
				e.frame, e.seq, e.movieId);
			break;
		case kProfFreeze:
			line = Common::String::format(
				"{\"name\":\"freeze\",\"ph\":\"i\",\"ts\":%u,\"pid\":1,\"tid\":%u,\"s\":\"t\"},\n",
				e.seq, e.movieId);
			break;
		case kProfThaw:
			line = Common::String::format(
				"{\"name\":\"thaw\",\"ph\":\"i\",\"ts\":%u,\"pid\":1,\"tid\":%u,\"s\":\"t\"},\n",
				e.seq, e.movieId);
			break;
		default:
			break;
		}
		out.writeString(line);
	}

	out.writeString("{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":1,\"args\":{\"name\":\"Lingo\"}}\n");
	out.writeString("]}\n");
	out.close();
	return true;
}

} // End of namespace Director
