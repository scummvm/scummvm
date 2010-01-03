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
	uint32 _msecs;
	int _frameRate;
	int _frameOffset;
	/* Total time: msecs + frame_offset/frame_rate */

public:
	/**
	 * Set up a timestamp with a given time and framerate.
	 * @param msecs     staring time in milliseconds
	 * @param frameRate number of frames per second (must be > 0)
	 */
	Timestamp(uint32 msecs, int frameRate);

	/** Adds a number of frames to a timestamp. */
	Timestamp addFrames(int frames) const;

	/** Adds a number of milliseconds to a timestamp. */
	Timestamp addMsecs(int ms) const;

	/** Computes the difference (# of frames) between this timestamp and b. */
	int frameDiff(const Timestamp &b) const;

	/** Computes the difference (# of milliseconds) between this timestamp and b. */
	int msecsDiff(const Timestamp &b) const;

	/** Determines the time in milliseconds described by this timestamp. */
	uint32 msecs() const;
};


} // End of namespace Audio

#endif
