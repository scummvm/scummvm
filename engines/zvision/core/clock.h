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

#ifndef ZVISION_CLOCK_H
#define ZVISION_CLOCK_H

#include "common/types.h"

class OSystem;

namespace ZVision {

/* Class for handling frame to frame deltaTime while keeping track of time pauses/un-pauses */
class Clock {
public:
	Clock(OSystem *system);

private:
	OSystem *_system;
	uint32 _lastTime;
	int32 _deltaTime;
	uint32 _pausedTime;
	bool _paused;

public:
	/**
	 * Updates _deltaTime with the difference between the current time and
	 * when the last update() was called.
	 */
	void update();

	/**
	 * Get the delta time since the last frame. (The time between update() calls)
	 *
	 * @return    Delta time since the last frame (in milliseconds)
	 */
	uint32 getDeltaTime() const {
		return _deltaTime;
	}

	/**
	 * Get the time from the program starting to the last update() call
	 *
	 * @return Time from program start to last update() call (in milliseconds)
	 */
	uint32 getLastMeasuredTime() {
		return _lastTime;
	}

	/**
	* Un-pause the clock.
	* Has no effect if the clock is already un-paused.
	*/
	void start();

	/**
	 * Pause the clock. Any future delta times will take this pause into account.
	 * Has no effect if the clock is already paused.
	 */
	void stop();
};

} // End of namespace ZVision

#endif
