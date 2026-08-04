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

#ifndef DIRECTOR_LINGO_LINGO_PROFILER_H
#define DIRECTOR_LINGO_LINGO_PROFILER_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/str-array.h"

namespace Common {
class Path;
}

namespace Director {

enum ProfilerEventType {
	kProfBegin = 0,
	kProfEnd,
	kProfFrame,
	kProfFreeze,
	kProfThaw
};

struct ProfilerEvent {
	uint8 type;
	uint32 seq;
	uint32 frame;
	uint16 depth;
	uint32 nameId;
	uint32 movieId;
};

class LingoProfiler {
public:
	LingoProfiler();

	bool isEnabled() const { return _enabled; }
	void setEnabled(bool enabled) { _enabled = enabled; }

	void onPushContext();
	void onPopContext();
	void onFreeze();
	void onThaw();

	void clear();
	bool exportChromeTrace(const Common::Path &path);

	const Common::Array<ProfilerEvent> &events() const { return _events; }
	const Common::String &internedName(uint32 id) const;
	bool isFull() const { return _full; }
	uint32 maxEvents() const { return _maxEvents; }

private:
	uint32 intern(const Common::String &s);
	void record(uint8 type, uint32 nameId);
	uint32 currentFrame() const;
	Common::String currentMovieName() const;

	bool _enabled;
	bool _full;
	uint32 _seq;
	uint32 _maxEvents;

	bool _haveLast;
	uint32 _lastFrame;
	uint32 _lastMovieId;

	Common::Array<ProfilerEvent> _events;
	Common::StringArray _strings;
	Common::HashMap<Common::String, uint32> _intern;
};

} // End of namespace Director

#endif
