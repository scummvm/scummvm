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

#ifndef TESORO_CORE_WALKAREA_H
#define TESORO_CORE_WALKAREA_H

#include "common/scummsys.h"

namespace Tesoro {

struct AnimStep;

/**
 * Anything that can be a target for Jim's walk, it can be a WalkArea, an InteractiveObject, a PathObject or a ScriptTask */
class WalkTarget {
public:
	virtual ~WalkTarget() {}
	/** Returns 0 or 1 to go on, 2 or 3 to stop the frame's object pass (as an object update does). */
	virtual int arrived() = 0;
};

/**
 * The shape of a place Jim may walk in (object kind 8): the points of its current step. They give the picture its
 * size (a scale for every point) and the corners of the route around an obstacle.
 */
class WalkArea {
public:
	WalkArea() {}

	void setShape(const AnimStep *step) { _shape = step; }
	void setPosition(int32 x, int32 y) { _x = x; _y = y; }

	/** How big things are at a point, 0x100 meaning life size */
	int32 scaleAt(int32 x, int32 y) const;

private:
	const AnimStep *_shape = nullptr;
	int32 _x = 0, _y = 0;
};

} // End of namespace Tesoro

#endif
