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

#ifndef SOUND_TIMESTAMP_H
#define SOUND_TIMESTAMP_H

#include "common/scummsys.h"

namespace Audio {

/**
 * Timestamps allow measuring times with a sub-millisecond granularity,
 * and without rounding losses. This is achieved by measuring frames
 * instead of milliseconds: Specify an initial time in milliseconds
 * plus framerate (in frames per second).
 */
class Timestamp {
protected:
	/**
	 * The seconds part of this timestamp.
	 * The total time in seconds represented by this timestamp can be
	 * computed as follows:
	 *   _secs + (double)_numberOfFrames / _framerate
	 */
	uint _secs;

	/**
	 * The number of frames which together with _secs encodes
	 * the timestamp. The total number of frames represented
	 * by this timestamp is computed as follows:
	 *   _numberOfFrames + _secs * _framerate
	 */
	int _numberOfFrames;

	/**
	 * The internal framerate, i.e. the number of frames per second.
	 * This is computed as the least common multiple of the framerate
	 * specified by the client code, and 1000.
	 * This way, we ensure that we can store both frames and
	 * milliseconds without any rounding losses.
	 */
	int _framerate;

	/**
	 * Factor by which the original framerate specified by the client
	 * code was multipled to obtain the internal _framerate value.
	 */
	int _framerateFactor;

public:
	/**
	 * Set up a timestamp with a given time and framerate.
	 * @param msecs     starting time in milliseconds
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint32 msecs, int framerate);

	/**
	 * Set up a timestamp with a given time, frames and framerate.
	 * @param secs      starting time in seconds
	 * @param frames    starting frames
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint secs, int frames, int framerate);

	/**
	 * Return a timestamp which represents as closely as possible
	 * the point in time describes by this timestamp, but with
	 * a different framerate.
	 */
	Timestamp convertToFramerate(int newFramerate) const;

	/**
	 * Check whether to timestamps describe the exact same moment
	 * in time. This means that two timestamps can compare
	 * as equal even if they use different framerates.
	 */
	bool operator==(const Timestamp &ts) const;
	bool operator!=(const Timestamp &ts) const;
	bool operator<(const Timestamp &ts) const;
	bool operator<=(const Timestamp &ts) const;
	bool operator>(const Timestamp &ts) const;
	bool operator>=(const Timestamp &ts) const;

	/**
	 * Returns a new timestamp, which corresponds to the time encoded
	 * by this timestamp with the given number of frames added.
	 * @param frames     number of frames to add
	 */
	Timestamp addFrames(int frames) const;

	/**
	 * Returns a new timestamp, which corresponds to the time encoded
	 * by this timestamp with the given number of milliseconds added.
	 * @param msecs     number of milliseconds to add
	 */
	Timestamp addMsecs(int msecs) const;

	/**
	 * Computes the number of frames between this timestamp and ts.
	 * The frames are with respect to the framerate used by this
	 * Timestamp (which may differ from the framerate used by ts).
	 */
	int frameDiff(const Timestamp &ts) const;

	/** Computes the number off milliseconds between this timestamp and ts. */
	int msecsDiff(const Timestamp &ts) const;

	/**
	 * Determines the time in milliseconds described by this timestamp,
	 * rounded down.
	 */
	uint32 msecs() const;

	/**
	 * Determines the time in seconds described by this timestamp,
	 * rounded down.
	 */
	inline uint32 secs() const { return _secs; }

	/**
	 * Determines the frames described by this timestamp.
	 */
	inline int numberOfFrames() const { return _numberOfFrames / _framerateFactor; }

	/** Return the framerate used by this timestamp. */
	inline int framerate() const { return _framerate / _framerateFactor; }

protected:

	int cmp(const Timestamp &ts) const;
};


} // End of namespace Audio

#endif
