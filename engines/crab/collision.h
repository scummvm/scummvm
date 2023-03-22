#pragma once

#include "Polygon.h"
#include "Rectangle.h"
#include "common_header.h"

// Find if a line and a rectangle intersect
bool CollideLineRect(int p0_x, int p0_y, int p1_x, int p1_y, const Rect &rect);

// Find if there is a clear line of sight between two rectangles
bool LineOfSight(const Rect &a, const Rect &b, const Rect &obstacle);

// Find the distance between two points
template<typename T>
double Distance2D(const T &p0_x, const T &p0_y, const T &p1_x, const T &p1_y) {
	return sqrt((p0_x - p1_x) * (p0_x - p1_x) + (p0_y - p1_y) * (p0_y - p1_y));
}