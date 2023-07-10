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

#ifndef CRAB_RECTANGLE_H
#define CRAB_RECTANGLE_H

#include "crab/Line.h"
#include "crab/common_header.h"
#include "crab/vectors.h"

namespace Crab {

//------------------------------------------------------------------------
// Purpose: A simple rectangle class
//------------------------------------------------------------------------
struct Rect {

	int x;
	int y;
	int w;
	int h;

	Rect(int X = 0, int Y = 0, int W = 0, int H = 0) {
		x = X;
		y = Y;
		w = W;
		h = H;
	}

	bool load(rapidxml::xml_node<char> *node, const bool &echo = true, const Common::String &x_name = "x",
			  const Common::String &y_name = "y", const Common::String &w_name = "w", const Common::String &h_name = "h");

	// Is this rectangle colliding with another rectangle?
	bool Collide(Rect box);

	// Extend this rectangle to fully contain another rectangle
	void Extend(Rect box);

	// Resolving a collision means we need to correct the position of the target rectangle to just outside of the reference rectangle
	// To do this, first we need to determine which edges of the rectangles are colliding

	// Resolve a collision in the X plane
	Direction ResolveX(Rect collider);

	// Resolve a collision in the Y plane
	Direction ResolveY(Rect collider);

	// Does this rectangle contain a point?
	bool Contains(int X, int Y) { return (X > x && X < x + w && Y > y && Y < y + h); }
	bool Contains(Vector2i v) { return Contains(v.x, v.y); }

	// Does this rectangle contain another rectangle?
	bool Contains(Rect box) { return (x < box.x && x + w > box.x + box.w && y < box.y && y + h > box.y + box.h); }

	// Flip a rectangle with respect to an axis
	void Flip(const TextureFlipType &flip, const Vector2i &axis);

	// Draw the borders of the rectangle
	void Draw(const int &XOffset = 0, const int &YOffset = 0, const uint8 &r = 0, const uint8 &g = 0, const uint8 &b = 0, const uint8 &a = 255);

	// Check if a rectangle is the same as another
	bool operator==(const Rect &r) { return r.x == x && r.y == y && r.w == w && r.h == h; }

	// Save to file
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name);
};

} // End of namespace Crab

#endif // CRAB_RECTANGLE_H
