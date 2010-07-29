// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

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

// Forward-Declarations
struct lua_State;

/**
	@brief Ein 2D-Vertex.
*/
class BS_Vertex
{
public:
	BS_Vertex() : X(0), Y(0) {};
	BS_Vertex(int X, int Y) { this->X = X; this->Y = Y; }

	int X;
	int Y;

	/**
		@brief Vergleicht zwei Vertecies.
	*/
	inline bool operator==(const BS_Vertex& rhs) const { if (X == rhs.X && Y == rhs.Y) return true; return false; }
	/**
	@brief Vergleicht zwei Vertecies.
	*/
	inline bool operator!=(const BS_Vertex& rhs) const { if (X != rhs.X || Y != rhs.Y) return true; return false; }
	/**
		@brief Addiert ein Vertex zum Vertex.
	*/
	inline void operator+=(const BS_Vertex& Delta) { X += Delta.X; Y += Delta.Y; }

	/**
		@brief Subtrahiert ein Vertex vom Vertex.
	*/
	inline void operator-=(const BS_Vertex& Delta) { X -= Delta.X; Y -= Delta.Y; }

	/**
		@brief Addiert zwei Vertecies
	*/
	inline BS_Vertex operator+(const BS_Vertex& Delta) const { return BS_Vertex(X + Delta.X, Y + Delta.Y); }

	/**
		@brief Subtrahiert zwei Vertecies
	*/
	inline BS_Vertex operator-(const BS_Vertex& Delta) const { return BS_Vertex(X - Delta.X, Y - Delta.Y); }

	/**
		@brief Berechnet das Quadrat des Abstandes zweier Vertecies.
		@param Vertex das Vertex zu dem der Abstand berechnet werden soll.
		@return Gibt das Quadrat des Abstandes zwischen diesem Objekt und Vertex zurück.
		@remark Falls nur Abstände verglichen werden sollen, sollte diese Methode benutzt werden, da sie schneller ist, als Distance().
	*/
	inline int Distance2(const BS_Vertex& Vertex) const
	{
		return (X - Vertex.X) * (X - Vertex.X) + (Y - Vertex.Y) * (Y - Vertex.Y);
	}

	/**
		@brief Berechnet den Abstand zweier Vertecies.
		@param Vertex das Vertex zu dem der Abstand berechnet werden soll.
		@return Gibt den Abstand zwischen diesem Objekt und Vertex zurück.
		@remark Falls nur Abstände verglichen werden sollen, sollte diese Methode Distance2(), die das Quadrat des Abstandes berechnet.
				Sie ist schneller.
	*/
	inline int Distance(const BS_Vertex& Vertex) const
	{
		return (int)(sqrtf(static_cast<float>(Distance2(Vertex))) + 0.5);
	}

	/**
		@brief Berechnet das Kreuzprodukt dieses Vertex mit einem weiteren Vertex. Hierbei werden die Vertecies als Vektoren aufgefasst.
		@param Vertex das zweite Vertex
		@return Gibt das Kreuzprodukt von diesem Vertex und dem Parameter Vertex zurück.
	*/
	inline int ComputeCrossProduct(const BS_Vertex& Vertex) const
	{
		return X * Vertex.Y - Vertex.X * Y;
	}

	/**
		@brief Berechnet das Skalarprodukt dieses Vertex mit einem weiteren Vertex. Hierbei werden die Vertecies als Vektoren aufgefasst.
		@param Vertex das zweite Vertex
		@return Gibt das Skalarprodukt von diesem Vertex und dem Parameter Vertex zurück.
	*/
	inline int ComputeDotProduct(const BS_Vertex& Vertex) const
	{
		return X * Vertex.X + Y * Vertex.Y;
	}

	/**
		@brief Berechnet den Winkel zwischen diesem Vertex und einem weiteren Vertex.  Hierbei werden die Vertecies als Vektoren aufgefasst.
		@param Vertex das zweite Vertex
		@return Gibt den Winkel zwischen diesem Vertex und dem Parameter Vertex im Bogenmaß zurück.
	*/
	inline float ComputeAngle(const BS_Vertex& Vertex) const
	{
		return atan2f(static_cast<float>(ComputeCrossProduct(Vertex)), static_cast<float>(ComputeDotProduct(Vertex)));
	}

	/**
		@brief Berechnet die Länge des Vektors
	*/
	inline float ComputeLength() const
	{
		return sqrtf(static_cast<float>(X * X + Y * Y));
	}

	static BS_Vertex & LuaVertexToVertex(lua_State * L, int StackIndex, BS_Vertex & Vertex);
	static void VertexToLuaVertex(lua_State * L, const BS_Vertex & Vertex);
};

#endif
