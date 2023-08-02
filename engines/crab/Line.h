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

#ifndef CRAB_LINE_H
#define CRAB_LINE_H

#include "crab/ScreenSettings.h"
#include "crab/common_header.h"
#include "crab/vectors.h"

namespace Crab {

// Find if 2 lines intersect and store the point of intersection
template<typename T>
bool collideLine(const T &p0X, const T &p0Y, const T &p1X, const T &p1Y,
				 const T &p2X, const T &p2Y, const T &p3X, const T &p3Y,
				 T *x = nullptr, T *y = nullptr) {
	Vector2D<T> s1, s2;
	s1.x = p1X - p0X;
	s1.y = p1Y - p0Y;
	s2.x = p3X - p2X;
	s2.y = p3Y - p2Y;

	float d = (-s2.x * s1.y + s1.x * s2.y);

	if (d != 0) {
		float s, t;
		s = (-s1.y * (p0X - p2X) + s1.x * (p0Y - p2Y)) / d;
		t = (s2.x * (p0Y - p2Y) - s2.y * (p0X - p2X)) / d;

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
			// Collision detected
			if (x != nullptr)
				*x = p0X + (t * s1.x);
			if (y != nullptr)
				*y = p0Y + (t * s1.y);

			return true;
		}
	}

	return false; // No collision
}

// Draw a line OpenGL style
void drawLine(const int &x1, const int &y1, const int &x2, const int &y2,
			  const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a);

} // End of namespace Crab

#endif // CRAB_LINE_H
