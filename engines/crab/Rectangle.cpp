#include "stdafx.h"
#include "Rectangle.h"

bool Rect::Load(rapidxml::xml_node<char> *node, const bool &echo, const std::string &x_name, const std::string &y_name,
	const std::string &w_name, const std::string &h_name)
{
	return LoadNum(x, x_name, node, echo)
		&& LoadNum(y, y_name, node, echo)
		&& LoadNum(w, w_name, node, echo)
		&& LoadNum(h, h_name, node, echo);
}

bool Rect::Collide(Rect box)
{
	if (box.x + box.w < x) return false;	//just checking if their
	if (box.x > x + w)     return false;	//bounding boxes even touch
	if (box.y + box.h < y) return false;
	if (box.y > y + h)	  return false;

	return true; //bounding boxes intersect
}

Direction Rect::ResolveX(Rect collider)
{
	//Check left edge of collider
	if (x < collider.x && collider.x < x + w)
		return DIRECTION_RIGHT;

	//Check right edge of collider
	if (x < collider.x + collider.w && collider.x + collider.w < x + w)
		return DIRECTION_LEFT;

	return DIRECTION_NONE;
}

Direction Rect::ResolveY(Rect collider)
{
	//Check top edge of collider
	if (y < collider.y && collider.y < y + h)
		return DIRECTION_DOWN;

	//Check bottom edge of collider
	if (y < collider.y + collider.h && collider.y + collider.h < y + h)
		return DIRECTION_UP;

	return DIRECTION_NONE;
}

void Rect::Flip(const TextureFlipType &flip, const Vector2i &axis)
{
	if (flip == FLIP_NONE)
		return;

	if (flip == FLIP_X || flip == FLIP_XY)
		x = 2 * axis.x - x - w;

	if (flip == FLIP_Y || flip == FLIP_XY)
		y = 2 * axis.y - y - h;
}

void Rect::Draw(const int &XOffset, const int &YOffset, const Uint8 &r, const Uint8 &g, const Uint8 &b, const Uint8 &a)
{
	int X = x + XOffset, Y = y + YOffset;

	DrawLine(X, Y, X + w, Y, r, g, b, a);
	DrawLine(X, Y, X, Y + h, r, g, b, a);
	DrawLine(X + w, Y, X + w, Y + h, r, g, b, a);
	DrawLine(X, Y + h, X + w, Y + h, r, g, b, a);
}

void Rect::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* name)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("x", gStrPool.Get(x)));
	child->append_attribute(doc.allocate_attribute("y", gStrPool.Get(y)));
	child->append_attribute(doc.allocate_attribute("w", gStrPool.Get(w)));
	child->append_attribute(doc.allocate_attribute("h", gStrPool.Get(h)));
	root->append_node(child);
}