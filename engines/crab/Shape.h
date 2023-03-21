#pragma once

#include "vectors.h"
#include "Polygon.h"

//The kind of shape
enum ShapeType { SHAPE_RECT, SHAPE_POLYGON, SHAPE_ELLIPSE };

struct CollisionData
{
	//Store the type of shape
	ShapeType type;

	//Do the two shapes intersect?
	bool intersect;

	//If Shape is Polygon, the .x and .y of this rectangle contain the minimum translation vector
	//If Shape is Rectangle, this contains the colliding rectangle
	Rect data;

	CollisionData() { type = SHAPE_RECT; intersect = false; }
};

class Shape
{
public:

	//The type of shape
	ShapeType type;

	//This stores both the ellipse and rectangle data
	Rect rect;

	//This stores the polygon data
	Polygon2D poly;

	Shape(){ type = SHAPE_RECT; }
	~Shape(){}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	CollisionData Collide(Rect box);

	bool Contains(const Vector2i &pos);

	void Draw(const int &XOffset = 0, const int &YOffset = 0,
		const Uint8 &r = 0, const Uint8 &g = 0, const Uint8 &b = 0.0f, const Uint8 &a = 255);
};
