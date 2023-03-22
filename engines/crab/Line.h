#pragma once

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