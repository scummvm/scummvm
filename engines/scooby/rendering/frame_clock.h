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

#ifndef SCOOBY_FRAME_CLOCK_H
#define SCOOBY_FRAME_CLOCK_H

#include "graphics/framelimiter.h"

// Schedules authored updates at the port's fixed 60-frame-per-second cadence.
//
// One monotonic clock owner replaces the NTSC retrace cadence. raylib VSync
// and target-FPS limiting remain disabled so a second scheduler cannot stack
// with recovered frame waits.

namespace Scooby {

class FrameClock {
public:
	FrameClock();

	// Waits until the next fixed update without enabling an independent
	// raylib frame limiter.
	void waitForNextFrame();
	void startFrame();

private:
	static const int kFramesPerSecond = 60;

	Graphics::FrameLimiter _limiter;
	// Std::chrono::steady_clock::time_point _nextFrameTimestamp;
};
} // namespace Scooby

#endif // SCOOBY_FRAME_CLOCK_H
