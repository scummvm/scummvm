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

//#include <assert.h>
#include "ultima/ultima6/sound/mixer/timestamp.h"
//#include "algorithm.h"

namespace Common {
/**
 * Euclid's algorithm to compute the greatest common divisor.
 */
template<class T>
T gcd(T a, T b) {
	if (a <= 0)
		a = -a;
	if (b <= 0)
		b = -b;

	while (a > 0) {
		T tmp = a;
		a = b % a;
		b = tmp;
	}

	return b;
}
}

namespace Audio {

Timestamp::Timestamp(uint32 ms, uint32 fr) {
	assert(fr > 0);

	_secs = ms / 1000;
	_framerateFactor = 1000 / Common::gcd<uint32>(1000, fr);
	_framerate = fr * _framerateFactor;

	// Note that _framerate is always divisible by 1000.
	_numFrames = (ms % 1000) * (_framerate / 1000);
}

Timestamp::Timestamp(uint32 s, uint32 frames, uint32 fr) {
	assert(fr > 0);

	_secs = s;
	_framerateFactor = 1000 / Common::gcd<uint32>(1000, fr);
	_framerate = fr * _framerateFactor;
	_numFrames = frames * _framerateFactor;

	normalize();
}

Timestamp Timestamp::convertToFramerate(uint32 newFramerate) const {
	Timestamp ts(*this);

	if (ts.framerate() != newFramerate) {
		ts._framerateFactor = 1000 / Common::gcd<uint32>(1000, newFramerate);
		ts._framerate = newFramerate * ts._framerateFactor;

		const uint32 g = Common::gcd(_framerate, ts._framerate);
		const uint32 p = _framerate / g;
		const uint32 q = ts._framerate / g;

		// Convert the frame offset to the new framerate.
		// We round to the nearest (as opposed to always
		// rounding down), to minimize rounding errors during
		// round trip conversions.
		ts._numFrames = (ts._numFrames * q + p / 2) / p;

		ts.normalize();
	}

	return ts;
}

void Timestamp::normalize() {
	// Convert negative _numFrames values to positive ones by adjusting _secs
	if (_numFrames < 0) {
		int secsub = 1 + (-_numFrames / _framerate);

		_numFrames += _framerate * secsub;
		_secs -= secsub;
	}

	// Wrap around if necessary
	_secs += (_numFrames / _framerate);
	_numFrames %= _framerate;
}

bool Timestamp::operator==(const Timestamp &ts) const {
	return cmp(ts) == 0;
}

bool Timestamp::operator!=(const Timestamp &ts) const {
	return cmp(ts) != 0;
}

bool Timestamp::operator<(const Timestamp &ts) const {
	return cmp(ts) < 0;
}

bool Timestamp::operator<=(const Timestamp &ts) const {
	return cmp(ts) <= 0;
}

bool Timestamp::operator>(const Timestamp &ts) const {
	return cmp(ts) > 0;
}

bool Timestamp::operator>=(const Timestamp &ts) const {
	return cmp(ts) >= 0;
}

int Timestamp::cmp(const Timestamp &ts) const {
	int delta = _secs - ts._secs;
	if (!delta) {
		const uint32 g = Common::gcd(_framerate, ts._framerate);
		const uint32 p = _framerate / g;
		const uint32 q = ts._framerate / g;

		delta = (_numFrames * q - ts._numFrames * p);
	}

	return delta;
}


Timestamp Timestamp::addFrames(int frames) const {
	Timestamp ts(*this);

	// The frames are given in the original framerate, so we have to
	// adjust by _framerateFactor accordingly.
	ts._numFrames += frames * _framerateFactor;
	ts.normalize();

	return ts;
}

Timestamp Timestamp::addMsecs(int ms) const {
	Timestamp ts(*this);
	ts._secs += ms / 1000;
	// Add the remaining frames. Note that _framerate is always divisible by 1000.
	ts._numFrames += (ms % 1000) * (ts._framerate / 1000);

	ts.normalize();

	return ts;
}

void Timestamp::addIntern(const Timestamp &ts) {
	_secs += ts._secs;

	if (_framerate == ts._framerate) {
		_numFrames += ts._numFrames;
	} else {
		// We need to multiply by the quotient of the two framerates.
		// We cancel the GCD in this fraction to reduce the risk of
		// overflows.
		const uint32 g = Common::gcd(_framerate, ts._framerate);
		const uint32 p = _framerate / g;
		const uint32 q = ts._framerate / g;

		_framerate *= q;
		_numFrames = _numFrames * q + ts._numFrames * p;
	}

	normalize();
}

Timestamp Timestamp::operator-() const {
	Timestamp result(*this);
	result._secs = -_secs;
	result._numFrames = -_numFrames;
	result.normalize();
	return result;
}

Timestamp Timestamp::operator+(const Timestamp &ts) const {
	Timestamp result(*this);
	result.addIntern(ts);
	return result;
}

Timestamp Timestamp::operator-(const Timestamp &ts) const {
	Timestamp result(*this);
	result.addIntern(-ts);
	return result;
}

/*
Timestamp &Timestamp::operator+=(const Timestamp &ts) {
    addIntern(ts);
    return *this;
}

Timestamp &Timestamp::operator-=(const Timestamp &ts) {
    addIntern(-ts);
    return *this;
}
*/

/*
int Timestamp::frameDiff(const Timestamp &ts) const {
    return (*this - ts).totalNumberOfFrames();
}
*/

int Timestamp::frameDiff(const Timestamp &ts) const {

	int delta = 0;
	if (_secs != ts._secs)
		delta = (_secs - ts._secs) * _framerate;

	delta += _numFrames;

	if (_framerate == ts._framerate) {
		delta -= ts._numFrames;
	} else {
		// We need to multiply by the quotient of the two framerates.
		// We cancel the GCD in this fraction to reduce the risk of
		// overflows.
		const uint32 g = Common::gcd(_framerate, ts._framerate);
		const uint32 p = _framerate / g;
		const uint32 q = ts._framerate / g;

		delta -= ((long)ts._numFrames * p + q / 2) / (long)q;
	}

	return delta / (int)_framerateFactor;
}

int Timestamp::msecsDiff(const Timestamp &ts) const {
	return msecs() - ts.msecs();
}

int Timestamp::msecs() const {
	// Note that _framerate is always divisible by 1000.
	return _secs * 1000 + _numFrames / (_framerate / 1000);
}


} // End of namespace Audio
