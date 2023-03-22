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
#include "crab/common_header.h"

// Find if a line and a rectangle intersect
bool CollideLineRect(int p0_x, int p0_y, int p1_x, int p1_y, const Rect &rect);

// Find if there is a clear line of sight between two rectangles
bool LineOfSight(const Rect &a, const Rect &b, const Rect &obstacle);

// Find the distance between two points
template<typename T>
double Distance2D(const T &p0_x, const T &p0_y, const T &p1_x, const T &p1_y) {
	return sqrt((p0_x - p1_x) * (p0_x - p1_x) + (p0_y - p1_y) * (p0_y - p1_y));
}

#endif // CRAB_COLLISION_H
