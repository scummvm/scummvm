/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
    BS_Vertex
    ---------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_VERTEX_H
#define SWORD25_VERTEX_H

// Includes
#include <math.h>
#include "sword25/kernel/common.h"
#include "sword25/util/lua/lua.h"

#if defined(MACOSX) || defined(SOLARIS) || defined(__MINGW32__)
// Older versions of Mac OS X didn't supply a powf function, so using it
// will cause a binary incompatibility when trying to run a binary built
// on a newer OS X release on an olderr one. And Solaris 8 doesn't provide
// powf, floorf, fabsf etc. at all.
// Cross-compiled MinGW32 toolchains suffer from a cross-compile bug in
// libstdc++. math/stubs.o should be empty, but it comes with a symbol for
// powf, resulting in a linker error because of multiple definitions.
// Hence we re-define them here. The only potential drawback is that it
// might be a little bit slower this way.
#define powf(x,y)	((float)pow(x,y))
#define floorf(x)	((float)floor(x))
#define fabsf(x)	((float)fabs(x))
#define sqrtf(x)	((float)sqrt(x))
#define atan2f(x,y)	((float)atan2(x,y))
#endif

namespace Sword25 {

/**
 * Defines a 2-D Vertex
 */
class Vertex {
public:
	Vertex() : x(0), y(0) {}
	Vertex(int x_, int y_) {
		this->x = x_;
		this->y = y_;
	}

	int x;
	int y;

	/**
	 * Compares two Vertecies.
	 */
	inline bool operator==(const Vertex &rhs) const {
		if (x == rhs.x && y == rhs.y) return true;
		return false;
	}
	/**
	 * Compares two Vertecies.
	 */
	inline bool operator!=(const Vertex &rhs) const {
		if (x != rhs.x || y != rhs.y) return true;
		return false;
	}
	/**
	 * Adds a vertex to vertex
	 */
	inline void operator+=(const Vertex &delta) {
		x += delta.x;
		y += delta.y;
	}

	/**
	 * Subtracts a vertex from a vertex
	 */
	inline void operator-=(const Vertex &delta) {
		x -= delta.x;
		y -= delta.y;
	}

	/**
	 * Adds two vertecies
	 */
	inline Vertex operator+(const Vertex &delta) const {
		return Vertex(x + delta.x, y + delta.y);
	}

	/**
	 * Subtracts two vertecies
	 */
	inline Vertex operator-(const Vertex &delta) const {
		return Vertex(x - delta.x, y - delta.y);
	}

	/**
	 * Calculates the square of the distance between two Vertecies.
	 * @param Vertex        The vertex for which the distance is to be calculated
	 * @return              Returns the square of the distance between itself and the passed vertex
	 * @remark              If only distances should be compared, this method should be used because
	 * it is faster than Distance()
	 */
	inline int distance2(const Vertex &vertex) const {
		return (x - vertex.x) * (x - vertex.x) + (y - vertex.y) * (y - vertex.y);
	}

	/**
	 * Calculates the square of the distance between two Vertecies.
	 * @param Vertex        The vertex for which the distance is to be calculated
	 * @return              Returns the square of the distance between itself and the passed vertex
	 * @remark              If only distances should be compared, Distance2() should be used, since it is faster.
	 */
	inline int distance(const Vertex &vertex) const {
		return (int)(sqrtf(static_cast<float>(distance2(vertex))) + 0.5);
	}

	/**
	 * Calculates the cross product of the vertex with another vertex. Here the Vertecies will be
	 * interpreted as vectors.
	 * @param Vertex        The second vertex
	 * @return              Returns the cross product of this vertex and the passed vertex.
	 */
	inline int computeCrossProduct(const Vertex &vertex) const {
		return x * vertex.y - vertex.x * y;
	}

	/**
	 * Returns the dot product of this vertex with another vertex. Here the Vertecies are interpreted as vectors.
	 * @param Vertex        The second vertex
	 * @return              Returns the dot product of this vertex and the passed vertex.
	 */
	inline int computeDotProduct(const Vertex &vertex) const {
		return x * vertex.x + y * vertex.y;
	}

	/**
	 * Calculates the angle between this vertex and another vertex. Here the Vertecies are interpreted as vectors.
	 * @param Vertex        The second vertex
	 * @return              Returns the angle between this vertex and the passed vertex in radians.
	 */
	inline float computeAngle(const Vertex &vertex) const {
		return atan2f(static_cast<float>(computeCrossProduct(vertex)), static_cast<float>(computeDotProduct(vertex)));
	}

	/**
	 * Calculates the length of the vector
	 */
	inline float computeLength() const {
		return sqrtf(static_cast<float>(x * x + y * y));
	}

	static Vertex &luaVertexToVertex(lua_State *L, int StackIndex, Vertex &vertex);
	static void vertexToLuaVertex(lua_State *L, const Vertex &vertex);
};

} // End of namespace Sword25

#endif
