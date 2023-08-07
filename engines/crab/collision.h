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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_COLLISION_H
#define CRAB_COLLISION_H

#include "crab/Polygon.h"
#include "crab/Rectangle.h"

namespace Crab {

// Find if a line and a rectangle intersect
bool collideLineRect(int p0X, int p0Y, int p1X, int p1Y, const Rect &rect);

// Find if there is a clear line of sight between two rectangles
bool lineOfSight(const Rect &a, const Rect &b, const Rect &obstacle);

// Find the distance between two points
template<typename T>
double distance2D(const T &p0X, const T &p0Y, const T &p1X, const T &p1Y) {
	return sqrt((p0X - p1X) * (p0X - p1X) + (p0Y - p1Y) * (p0Y - p1Y));
}

} // End of namespace Crab

#endif // CRAB_COLLISION_H
