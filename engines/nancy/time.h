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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_TIME_H
#define NANCY_TIME_H

#include "common/scummsys.h"

namespace Nancy {

// A small utility struct to measure/accumulate time
struct Time {
public:
	Time() { _milliseconds = 0; }
	Time(const uint32 &t) { _milliseconds = t; }
	Time(const Time &t) = default;
	~Time() = default;
	explicit operator uint32() const { return _milliseconds; }
	Time &operator=(const Time &t)                          { if (this != &t) _milliseconds = t._milliseconds; return *this; }
	Time &operator=(const uint32 &t)                        { _milliseconds = t; return *this; }
	Time &operator+=(const Time &t)                         { _milliseconds += t._milliseconds; return *this; }
	Time &operator+=(const uint32 &t)                       { _milliseconds+=t; return *this; }
	Time &operator-=(const Time &t)                         { _milliseconds -= t._milliseconds; return *this; }
	Time &operator-=(const uint32 &t)                       { _milliseconds-=t; return *this; }

	friend Time operator+(Time l, const Time &r)            { l += r; return l; }
	friend Time operator+(Time l, const uint32 &r)          { l += r; return l; }
	friend Time operator+(const uint32 &l, Time r)          { r += l; return r; }
	friend Time operator-(Time l, const Time &r)            { l -= r; return l; }
	friend Time operator-(Time l, const uint32 &r)          { l -= r; return l; }
	friend Time operator-(const uint32 &l, Time r)          { r -= l; return r; }

	friend bool operator== (const Time &l, const Time &r)   { return l._milliseconds == r._milliseconds; }
	friend bool operator== (const Time &l, const uint32 &r) { return l._milliseconds == r; }
	friend bool operator== (const uint32 &l, const Time &r) { return l == r._milliseconds; }
	friend bool operator!= (const Time &l, const Time &r)   { return l._milliseconds != r._milliseconds; }
	friend bool operator!= (const Time &l, const uint32 &r) { return l._milliseconds != r; }
	friend bool operator!= (const uint32 &l, const Time &r) { return l != r._milliseconds; }
	friend bool operator< (const Time &l, const Time &r)    { return l._milliseconds < r._milliseconds; }
	friend bool operator< (const Time &l, const uint32 &r)  { return l._milliseconds < r; }
	friend bool operator< (const uint32 &l, const Time &r)  { return l < r._milliseconds; }
	friend bool operator> (const Time &l, const Time &r)    { return r < l; }
	friend bool operator> (const Time &l, const uint32 &r)  { return r < l; }
	friend bool operator> (const uint32 &l, const Time &r)  { return r < l; }
	friend bool operator<= (const Time &l, const Time &r)   { return !(l > r); }
	friend bool operator<= (const Time &l, const uint32 &r) { return !(l > r); }
	friend bool operator<= (const uint32 &l, const Time &r) { return !(l > r); }
	friend bool operator>= (const Time &l, const Time &r)   { return !(l < r); }
	friend bool operator>= (const Time &l, const uint32 &r) { return !(l < r); }
	friend bool operator>= (const uint32 &l, const Time &r) { return !(l < r); }

	uint16 getSeconds()         { return (_milliseconds / 1000) % 60; }
	uint16 getMinutes()         { return (_milliseconds / 60000) % 60; }
	uint16 getTotalHours()      { return _milliseconds / 3600000; }

	uint16 getHours()           { return (_milliseconds / 3600000) % 24; } // Used for player time
	uint16 getDays()            { return _milliseconds / 86400000; } // up to 49.7 days

private:
	uint32 _milliseconds;
};

} // End of namespace Nancy

#endif // NANCY_TIME_H
