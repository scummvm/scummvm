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

#ifndef CRAB_SHAPE_H
#define CRAB_SHAPE_H

#include "crab/Polygon.h"
#include "crab/vectors.h"

namespace Crab {

// The kind of shape
enum ShapeType {
	SHAPE_RECT,
	SHAPE_POLYGON,
	SHAPE_ELLIPSE
};

struct CollisionData {
	// Store the type of shape
	ShapeType _type;

	// Do the two shapes intersect?
	bool _intersect;

	// If Shape is Polygon, the .x and .y of this rectangle contain the minimum translation vector
	// If Shape is Rectangle, this contains the colliding rectangle
	Rect _data;

	CollisionData() {
		_type = SHAPE_RECT;
		_intersect = false;
	}
};

class Shape {
public:
	// The type of shape
	ShapeType _type;

	// This stores both the ellipse and rectangle data
	Rect _rect;

	// This stores the polygon data
	Polygon2D _poly;

	Shape() {
		_type = SHAPE_RECT;
	}

	~Shape() {}

	void load(rapidxml::xml_node<char> *node, const bool &echo = true);
	CollisionData collide(Rect box);

	bool contains(const Vector2i &pos);

	void draw(const int &xOffset = 0, const int &yOffset = 0,
			  const uint8 &r = 0, const uint8 &g = 0, const uint8 &b = 0, const uint8 &a = 255);
};

} // End of namespace Crab

#endif // CRAB_SHAPE_H
