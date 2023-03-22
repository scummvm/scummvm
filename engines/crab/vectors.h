#pragma once
#include "common_header.h"

#include "GameParam.h"
#include "loaders.h"

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