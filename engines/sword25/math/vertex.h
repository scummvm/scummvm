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

namespace {

// Forward declarations
struct lua_State;

}

namespace Sword25 {

/**
 * Defines a 2-D Vertex
 */
class BS_Vertex {
public:
	BS_Vertex() : X(0), Y(0) {};
	BS_Vertex(int X_, int Y_) { this->X = X_; this->Y = Y_; }

	int X;
	int Y;

	/**
	 * Compares two Vertecies.
	 */
	inline bool operator==(const BS_Vertex& rhs) const { if (X == rhs.X && Y == rhs.Y) return true; return false; }
	/**
	 * Compares two Vertecies.
	 */
	inline bool operator!=(const BS_Vertex& rhs) const { if (X != rhs.X || Y != rhs.Y) return true; return false; }
	/**
	 * Adds a vertex to vertex
	 */
	inline void operator+=(const BS_Vertex& Delta) { X += Delta.X; Y += Delta.Y; }

	/**
	 * Subtracts a vertex from a vertex
	 */
	inline void operator-=(const BS_Vertex& Delta) { X -= Delta.X; Y -= Delta.Y; }

	/**
	 * Adds two vertecies
	 */
	inline BS_Vertex operator+(const BS_Vertex& Delta) const { return BS_Vertex(X + Delta.X, Y + Delta.Y); }

	/**
	 * Subtracts two vertecies
	 */
	inline BS_Vertex operator-(const BS_Vertex& Delta) const { return BS_Vertex(X - Delta.X, Y - Delta.Y); }

	/**
	 * Calculates the square of the distance between two Vertecies.
	 * @param Vertex		The vertex for which the distance is to be calculated
	 * @return				Returns the square of the distance between itself and the passed vertex
	 * @remark				If only distances should be compared, this method should be used because
	 * it is faster than Distance()
	 */
	inline int Distance2(const BS_Vertex& Vertex) const	{
		return (X - Vertex.X) * (X - Vertex.X) + (Y - Vertex.Y) * (Y - Vertex.Y);
	}

	/**
	 * Calculates the square of the distance between two Vertecies.
	 * @param Vertex		The vertex for which the distance is to be calculated
	 * @return				Returns the square of the distance between itself and the passed vertex
	 * @remark				If only distances should be compared, Distance2() should be used, since it is faster.
	 */
	inline int Distance(const BS_Vertex& Vertex) const {
		return (int)(sqrtf(static_cast<float>(Distance2(Vertex))) + 0.5);
	}

	/**
	 * Calculates the cross product of the vertex with another vertex. Here the Vertecies will be
	 * interpreted as vectors.
	 * @param Vertex		The second vertex
	 * @return				Returns the cross product of this vertex and the passed vertex.
	 */
	inline int ComputeCrossProduct(const BS_Vertex& Vertex) const {
		return X * Vertex.Y - Vertex.X * Y;
	}

	/**
	 * Returns the dot product of this vertex with another vertex. Here the Vertecies are interpreted as vectors.
	 * @param Vertex		The second vertex
	 * @return				Returns the dot product of this vertex and the passed vertex.
	 */
	inline int ComputeDotProduct(const BS_Vertex& Vertex) const
	{
		return X * Vertex.X + Y * Vertex.Y;
	}

	/**
	 * Calculates the angle between this vertex and another vertex. Here the Vertecies are interpreted as vectors.
	 * @param Vertex		The second vertex
	 * @return				Returns the angle between this vertex and the passed vertex in radians.
	 */
	inline float ComputeAngle(const BS_Vertex& Vertex) const {
		return atan2f(static_cast<float>(ComputeCrossProduct(Vertex)), static_cast<float>(ComputeDotProduct(Vertex)));
	}

	/**
	 * Calculates the length of the vector
	 */
	inline float ComputeLength() const {
		return sqrtf(static_cast<float>(X * X + Y * Y));
	}

	static BS_Vertex & LuaVertexToVertex(lua_State * L, int StackIndex, BS_Vertex & Vertex);
	static void VertexToLuaVertex(lua_State * L, const BS_Vertex & Vertex);
};

} // End of namespace Sword25

#endif
