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

Timestamp::Timestamp(uint32 m, int frameRate) :
	_msecs(m), _frameRate(frameRate), _frameOffset(0) {
	assert(_frameRate > 0);
}


Timestamp Timestamp::addFrames(int frames) const {
	Timestamp timestamp(*this);
	timestamp._frameOffset += frames;

	if (timestamp._frameOffset < 0) {
		int secsub = 1 + (-timestamp._frameOffset / timestamp._frameRate);

		timestamp._frameOffset += timestamp._frameRate * secsub;
		timestamp._msecs -= secsub * 1000;
	}

	timestamp._msecs += (timestamp._frameOffset / timestamp._frameRate) * 1000;
	timestamp._frameOffset %= timestamp._frameRate;

	return timestamp;
}

Timestamp Timestamp::addMsecs(int ms) const {
	Timestamp timestamp(*this);
	timestamp._msecs += ms;
	return timestamp;
}

int Timestamp::frameDiff(const Timestamp &b) const {
	assert(_frameRate == b._frameRate);

	int msecdelta = 0;
	if (_msecs != b._msecs)
		msecdelta = (long(_msecs) - long(b._msecs)) * _frameRate / 1000;

	return msecdelta + _frameOffset - b._frameOffset;
}

int Timestamp::msecsDiff(const Timestamp &b) const {
	return long(msecs()) - long(b.msecs());
}

uint32 Timestamp::msecs() const {
	return _msecs + _frameOffset * 1000L / _frameRate;
}


} // End of namespace Audio
