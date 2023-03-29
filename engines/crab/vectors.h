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

#ifndef CRAB_VECTORS_H
#define CRAB_VECTORS_H
#include "crab/common_header.h"

#include "crab/GameParam.h"
#include "crab/loaders.h"

namespace Crab {

//------------------------------------------------------------------------
// Purpose: A simple 2D vector class
//------------------------------------------------------------------------
template<typename T>
class Vector2D {
public:
	T x, y;

	Vector2D(T X = 0, T Y = 0) { Set(X, Y); }
	void Set(T X = 0, T Y = 0) {
		x = X;
		y = Y;
	}

	bool Load(rapidxml::xml_node<char> *node, const bool &echo = true) {
		return LoadNum(x, "x", node, echo) && LoadNum(y, "y", node, echo);
	}

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
		child->append_attribute(doc.allocate_attribute("x", gStrPool.Get((int)x)));
		child->append_attribute(doc.allocate_attribute("y", gStrPool.Get((int)y)));
		root->append_node(child);
	}

	double Magnitude() { return sqrt(x * x + y * y); }
	double MagSqr() { return (x * x + y * y); } // Added to avoid unnecessary sqrt calls (SZ)
	void Normalize() {
		double m = Magnitude();
		x = x / m;
		y = y / m;
	}

	float DotProduct(const Vector2D<T> &v) const { return x * v.x + y * v.y; }

	// operators
	void operator+=(const Vector2D &v) {
		x += v.x;
		y += v.y;
	}
	void operator-=(const Vector2D &v) {
		x -= v.x;
		y -= v.y;
	}
	bool operator==(const Vector2D &v) { return x == v.x && y == v.y; }
	bool operator!=(const Vector2D &v) { return x != v.x || y != v.y; }
	friend Vector2D operator-(const Vector2D &v1, const Vector2D &v2) {
		return Vector2D(v1.x - v2.x, v1.y - v2.y);
	}
};

typedef Vector2D<int> Vector2i;
typedef Vector2D<float> Vector2f;

//------------------------------------------------------------------------
// Purpose: A simple 3D vector class
//------------------------------------------------------------------------
template<typename T>
class Vector3D {
public:
	T x, y, z;

	Vector3D(T X = 0, T Y = 0, T Z = 0) { Set(X, Y, Z); }
	Vector3D(rapidxml::xml_node<char> *node) { Load(node); }

	void Set(T X = 0, T Y = 0, T Z = 0) {
		x = X;
		y = Y;
		z = Z;
	}
	bool Load(rapidxml::xml_node<char> *node) { return LoadXYZ<T>(x, y, z, node); }

	// operators
	void operator+=(const Vector3D &v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator-=(const Vector3D &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	bool operator==(const Vector3D &v) { return x == v.x && y == v.y && z == v.z; }
};

typedef Vector3D<int> Vector3i;
typedef Vector3D<float> Vector3f;

} // End of namespace Crab

#endif // CRAB_VECTORS_H
