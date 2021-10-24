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

#ifndef PLAYGROUND3D_GFX_FRAMELIMITER_H
#define PLAYGROUND3D_GFX_FRAMELIMITER_H

#include "common/system.h"

namespace Playground3d {

namespace Gfx {

/**
 * A framerate limiter
 *
 * Ensures the framerate does not exceed the specified value
 * by delaying until all of the timeslot allocated to the frame
 * is consumed.
 * Allows to curb CPU usage and have a stable framerate.
 */
class FrameLimiter {
public:
	FrameLimiter(OSystem *system, const uint framerate);

	void startFrame();
	void delayBeforeSwap();

	void pause(bool pause);

	uint getLastFrameDuration() const;
private:
	OSystem *_system;

	bool _enabled;
	uint _speedLimitMs;
	uint _startFrameTime;
	uint _lastFrameDurationMs;
};

} // End of namespace Gfx
} // End of namespace Playground3d

#endif // PLAYGROUND3D_GFX_FRAMELIMITER_H
