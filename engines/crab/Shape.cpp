#include "stdafx.h"
#include "Shape.h"

void Shape::Load(rapidxml::xml_node<char> *node, const bool &echo)
{
	if (NodeValid("polygon", node, echo))
	{
		type = SHAPE_POLYGON;
		poly.Load(node, rect);
	}
	else
	{
		rect.Load(node, echo, "x", "y", "width", "height");
		if (NodeValid("ellipse", node, echo))
			type = SHAPE_ELLIPSE;
		else
			type = SHAPE_RECT;
	}
}

CollisionData Shape::Collide(Rect box)
{
	CollisionData res;
	res.intersect = rect.Collide(box);

	if (res.intersect)
	{
		res.type = type;
		if (type == SHAPE_POLYGON)
		{
			PolygonCollisionResult pcr = poly.Collide(box);
			res.intersect = pcr.intersect;
			res.data.x = pcr.mtv.x;
			res.data.y = pcr.mtv.y;
			return res;
		}
		else
			res.data = rect;
	}

	return res;
}

bool Shape::Contains(const Vector2i &pos)
{
	if (rect.Contains(pos))
	{
		if (type == SHAPE_POLYGON)
			return poly.Contains(pos.x, pos.y);
		else
			return true;
	}

	return false;
}

void Shape::Draw(const int &XOffset, const int &YOffset, const Uint8 &r, const Uint8 &g, const Uint8 &b, const Uint8 &a)
{
	if (type == SHAPE_POLYGON)
		poly.Draw(XOffset, YOffset, r, g, b, a);
	else
		rect.Draw(XOffset, YOffset, r, g, b, a);
}