#pragma once

#include "common_header.h"
#include "vectors.h"
#include "Rectangle.h"

//------------------------------------------------------------------------
// Purpose: The result of a collision
//------------------------------------------------------------------------
struct PolygonCollisionResult
{
	//Are the Polygons currently intersecting?
	bool intersect;

	//The translation to apply to polygon A to push the polygons apart
	//Also known as the minimum translation vector
	Vector2f mtv;

	PolygonCollisionResult() { intersect = false; }
};

//------------------------------------------------------------------------
// Purpose: A simple 2D Polygon class
//------------------------------------------------------------------------
class Polygon2D
{
	void AddPoint(const Vector2f &ref, const std::string &x, const std::string &y, Vector2f &min, Vector2f &max);

public:
	//A list of all points
	std::vector<Vector2f> point;

	//A list of all edges
	std::vector<Vector2f> edge;

	Polygon2D(){}
	Polygon2D(rapidxml::xml_node<char> *node, Rect &bounds){ Load(node, bounds); }

	//Returns the approximate axis aligned bounding box of the polygon
	void Load(rapidxml::xml_node<char> *node, Rect &bounds);

	void SetEdge();

	Vector2f Center() const;
	void Offset(const float &x, const float &y);
	void Offset(const Vector2f &v){ Offset(v.x, v.y); }

	//Check if Polygon2D A is going to collide with Polygon2D B for the given velocity
	//PolyA is this polygon
	PolygonCollisionResult Collide(const Polygon2D &polyB);

	//Code for collision with a rectangle
	PolygonCollisionResult Collide(const Rect &rect);

	//Find if a point is inside this polygon
	bool Contains(const float &X, const float &Y);

	// Calculate the projection of a polygon on an axis and returns it as a [min, max] interval
	void Project(const Vector2f &axis, float &min, float &max) const;

	void Draw(const int &XOffset = 0, const int &YOffset = 0,
		const Uint8 &r = 0, const Uint8 &g = 0, const Uint8 &b = 0.0f, const Uint8 &a = 255);
};
