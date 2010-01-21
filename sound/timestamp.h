/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "common/sys.h"

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
	 *   _secs + (double)_numFrames / _framerate
	 */
	int _secs;

	/**
	 * The number of frames which together with _secs encodes
	 * the timestamp. The total number of frames represented
	 * by this timestamp is computed as follows:
	 *   _numFrames + _secs * _framerate
	 *
	 * This is always a value greater or equal to zero.
	 * The only reason this is an int and not an uint is to
	 * allow intermediate negative values.
	 */
	int _numFrames;

	/**
	 * The internal framerate, i.e. the number of frames per second.
	 * This is computed as the least common multiple of the framerate
	 * specified by the client code, and 1000.
	 * This way, we ensure that we can store both frames and
	 * milliseconds without any rounding losses.
	 */
	uint _framerate;

	/**
	 * Factor by which the original framerate specified by the client
	 * code was multipled to obtain the internal _framerate value.
	 */
	uint _framerateFactor;

public:
	/**
	 * Set up a timestamp with a given time and framerate.
	 * @param msecs     starting time in milliseconds
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint msecs, uint framerate);

	/**
	 * Set up a timestamp with a given time, frames and framerate.
	 * @param secs      starting time in seconds
	 * @param frames    starting frames
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint secs, uint frames, uint framerate);

	/**
	 * Return a timestamp which represents as closely as possible
	 * the point in time describes by this timestamp, but with
	 * a different framerate.
	 */
	Timestamp convertToFramerate(uint newFramerate) const;

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


	// unary minus
	Timestamp operator-() const;

	Timestamp operator+(const Timestamp &ts) const;
	Timestamp operator-(const Timestamp &ts) const;

//	Timestamp &operator+=(const Timestamp &ts);
//	Timestamp &operator-=(const Timestamp &ts);

	/**
	 * Computes the number of frames between this timestamp and ts.
	 * The frames are with respect to the framerate used by this
	 * Timestamp (which may differ from the framerate used by ts).
	 */
	int frameDiff(const Timestamp &ts) const;

	/** Computes the number off milliseconds between this timestamp and ts. */
	int msecsDiff(const Timestamp &ts) const;

	/**
	 * Return the time in milliseconds described by this timestamp,
	 * rounded down.
	 */
	int msecs() const;

	/**
	 * Return the time in seconds described by this timestamp,
	 * rounded down.
	 */
	inline int secs() const {
		return _secs;
	}

	/**
	 * Return the time in frames described by this timestamp.
	 */
	inline int totalNumberOfFrames() const {
		return _numFrames / (int)_framerateFactor + _secs * (int)(_framerate / _framerateFactor);
	}

	/**
	 * A timestamp consists of a number of seconds, plus a number
	 * of frames, the latter describing a fraction of a second.
	 * This method returns the latter number.
	 */
	inline int numberOfFrames() const {
		return _numFrames / (int)_framerateFactor;
	}

	/** Return the framerate used by this timestamp. */
	inline uint framerate() const { return _framerate / _framerateFactor; }

protected:

	/**
	 * Compare this timestamp to another one and return
	 * a value similar to strcmp.
	 */
	int cmp(const Timestamp &ts) const;

	/**
	 * Normalize this timestamp by making _numFrames non-negative
	 * and reducing it modulo _framerate.
	 */
	void normalize();

	/**
	 * Add another timestamp to this one and normalize the result.
	 */
	void addIntern(const Timestamp &ts);
};


} // End of namespace Audio

#endif
