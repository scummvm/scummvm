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

#ifndef AGS_STD_CHRONO_H
#define AGS_STD_CHRONO_H

#include "common/system.h"

namespace AGS3 {
namespace std {

namespace chrono {

class duration {
private:
	uint32 _value;
public:
	duration() : _value(0) {
	}
	duration(uint32 value) : _value(value) {
	}

	size_t count() const {
		// durations for ScummVM are hardcoded to be in milliseconds
		return 1000;
	}

	operator uint32() const {
		return _value;
	}

	inline bool operator>=(const duration &rhs) const {
		return _value >= rhs._value;
	}
};

class milliseconds : public duration {
public:
	milliseconds() : duration(0) {}
	milliseconds(uint32 val) : duration(val) {}

	static milliseconds zero() {
		return milliseconds();
	}
};

class microseconds : public duration {
public:
	microseconds() : duration(0) {}
	microseconds(long val) : duration(val / 1000) {}
};


struct system_clock {
};


struct steady_clock { // wraps QueryPerformanceCounter
	using rep = uint32;
	using period = milliseconds;
	using duration = milliseconds;
	using time_point = uint32;
	static constexpr bool is_steady = true;

	static time_point now() { // get current time
		return g_system->getMillis();
	}
};

using high_resolution_clock = steady_clock;

template<class T>
duration duration_cast(T param);

template<class T>
duration duration_cast(T param) {
	return duration(param);
}

} // namespace chrono

} // namespace std
} // namespace AGS3

#endif
