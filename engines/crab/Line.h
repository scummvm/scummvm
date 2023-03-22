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

#include "ScreenSettings.h"
#include "common_header.h"
#include "vectors.h"

// Find if 2 lines intersect and store the point of intersection
template<typename T>
bool CollideLine(const T &p0_x, const T &p0_y, const T &p1_x, const T &p1_y,
				 const T &p2_x, const T &p2_y, const T &p3_x, const T &p3_y,
				 T *x = NULL, T *y = NULL) {
	Vector2D<T> s1, s2;
	s1.x = p1_x - p0_x;
	s1.y = p1_y - p0_y;
	s2.x = p3_x - p2_x;
	s2.y = p3_y - p2_y;

	float d = (-s2.x * s1.y + s1.x * s2.y);

	if (d != 0) {
		float s, t;
		s = (-s1.y * (p0_x - p2_x) + s1.x * (p0_y - p2_y)) / d;
		t = (s2.x * (p0_y - p2_y) - s2.y * (p0_x - p2_x)) / d;

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
			// Collision detected
			if (x != NULL)
				*x = p0_x + (t * s1.x);
			if (y != NULL)
				*y = p0_y + (t * s1.y);

			return true;
		}
	}

	return false; // No collision
}

// Draw a line OpenGL style
void DrawLine(const int &x1, const int &y1, const int &x2, const int &y2,
			  const Uint8 &r, const Uint8 &g, const Uint8 &b, const Uint8 &a);

#endif // CRAB_LINE_H
