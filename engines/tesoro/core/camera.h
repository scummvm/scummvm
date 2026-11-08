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

#ifndef TESORO_CORE_CAMERA_H
#define TESORO_CORE_CAMERA_H

#include "common/scummsys.h"

namespace Tesoro {

class Object;

/**
 * The view onto a level.
 */
class Camera {
public:

	void init();

	/** The object the camera follows (null for none). */
	void setTarget(const Object *target) { _target = target; }

	/** Limits for the center position; the visible area is kept inside them. */
	void setBounds(int32 left, int32 top, int32 right, int32 bottom);

	/** Moves the centre to (x, y) pulled inside the bounds. Returns whether it was already inside. */
	bool setPositionClamped(int32 x, int32 y);

	/** Goes to (x, y): at once when duration is 0, otherwise gliding for duration milliseconds. */
	void moveTo(int32 x, int32 y, int32 duration, int32 now);

	/**
	 * Per-frame step. Starts a scroll when the target has left the dead zone, advances a glide in progress, and
	 * records the position scaled by 300 / divisor (the divisor comes from the room the camera is shown for).
	 */
	void updateToward(int32 divisor, int32 now);

	/** Moves the centre without any clamping or gliding. */
	void shift(int32 dx, int32 dy) { _x += dx; _y += dy; }
	int32 x() const { return _x; }
	int32 y() const { return _y; }
	int32 scaledX() const { return _scaledX; }
	int32 scaledY() const { return _scaledY; }

private:
	const Object *_target;
	int32 _startX, _startY;
	int32 _startTime;
	int32 _duration;		// 0 when no glide is running
	int32 _goalX, _goalY;
	int32 _boundLeft, _boundTop, _boundRight, _boundBottom;
	int32 _x, _y;
	int32 _scaledX, _scaledY;
};

} // End of namespace Tesoro

#endif
