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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sound/timestamp.h"

namespace Audio {

static uint gcd(uint a, uint b) {
	while (a > 0) {
		int tmp = a;
		a = b % a;
		b = tmp;
	}
	return b;
}

Timestamp::Timestamp(uint32 m, int framerate) :
	_msecs(m), _framerate(framerate), _numberOfFrames(0) {
	assert(_framerate > 0);
}


Timestamp Timestamp::convertToFramerate(int newFramerate) const {
	Timestamp ts(*this);

	if (ts._framerate != newFramerate) {
		ts._framerate = newFramerate;

		const uint g = gcd(_framerate, ts._framerate);
		const uint p = _framerate / g;
		const uint q = ts._framerate / g;

		// Convert the frame offset to the new framerate.
		// We round to the nearest (as opposed to always
		// rounding down), to minimize rounding errors during
		// round trip conversions.
		ts._numberOfFrames = (ts._numberOfFrames * q + p/2) / p;

		ts._msecs += (ts._numberOfFrames / ts._framerate) * 1000;
		ts._numberOfFrames %= ts._framerate;
	}

	return ts;
}

bool Timestamp::operator==(const Timestamp &ts) const {
	// TODO: Improve this
	return (ts.msecs() == msecs()) && !frameDiff(ts);
}

bool Timestamp::operator!=(const Timestamp &ts) const {
	return !(*this == ts);
}


Timestamp Timestamp::addFrames(int frames) const {
	Timestamp ts(*this);
	ts._numberOfFrames += frames;

	if (ts._numberOfFrames < 0) {
		int secsub = 1 + (-ts._numberOfFrames / ts._framerate);

		ts._numberOfFrames += ts._framerate * secsub;
		ts._msecs -= secsub * 1000;
	}

	ts._msecs += (ts._numberOfFrames / ts._framerate) * 1000;
	ts._numberOfFrames %= ts._framerate;

	return ts;
}

Timestamp Timestamp::addMsecs(int ms) const {
	Timestamp ts(*this);
	ts._msecs += ms;
	return ts;
}

int Timestamp::frameDiff(const Timestamp &ts) const {

	int delta = 0;
	if (_msecs != ts._msecs)
		delta = (long(_msecs) - long(ts._msecs)) * _framerate / 1000;

	delta += _numberOfFrames;

	if (_framerate == ts._framerate) {
		delta -= ts._numberOfFrames;
	} else {
		// We need to multiply by the quotient of the two framerates.
		// We cancel the GCD in this fraction to reduce the risk of
		// overflows.
		const uint g = gcd(_framerate, ts._framerate);
		const uint p = _framerate / g;
		const uint q = ts._framerate / g;

		delta -= (ts._numberOfFrames * p + q/2) / q;
	}

	return delta;
}

int Timestamp::msecsDiff(const Timestamp &ts) const {
	return long(msecs()) - long(ts.msecs());
}

uint32 Timestamp::msecs() const {
	return _msecs + _numberOfFrames * 1000L / _framerate;
}


} // End of namespace Audio
