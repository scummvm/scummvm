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

#include "Polygon.h"
#include "vectors.h"

// The kind of shape
enum ShapeType { SHAPE_RECT,
				 SHAPE_POLYGON,
				 SHAPE_ELLIPSE };

struct CollisionData {
	// Store the type of shape
	ShapeType type;

	// Do the two shapes intersect?
	bool intersect;

	// If Shape is Polygon, the .x and .y of this rectangle contain the minimum translation vector
	// If Shape is Rectangle, this contains the colliding rectangle
	Rect data;

	CollisionData() {
		type = SHAPE_RECT;
		intersect = false;
	}
};

class Shape {
public:
	// The type of shape
	ShapeType type;

	// This stores both the ellipse and rectangle data
	Rect rect;

	// This stores the polygon data
	Polygon2D poly;

	Shape() { type = SHAPE_RECT; }
	~Shape() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	CollisionData Collide(Rect box);

	bool Contains(const Vector2i &pos);

	void Draw(const int &XOffset = 0, const int &YOffset = 0,
			  const Uint8 &r = 0, const Uint8 &g = 0, const Uint8 &b = 0.0f, const Uint8 &a = 255);
};

#endif // CRAB_SHAPE_H
