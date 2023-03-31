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

#include "crab/Rectangle.h"

namespace Crab {

bool Rect::Load(rapidxml::xml_node<char> *node, const bool &echo, const std::string &x_name, const std::string &y_name,
				const std::string &w_name, const std::string &h_name) {
	return LoadNum(x, x_name.c_str(), node, echo) && LoadNum(y, y_name.c_str(), node, echo) && LoadNum(w, w_name.c_str(), node, echo) && LoadNum(h, h_name.c_str(), node, echo);
}

bool Rect::Collide(Rect box) {
	if (box.x + box.w < x)
		return false; // just checking if their
	if (box.x > x + w)
		return false; // bounding boxes even touch
	if (box.y + box.h < y)
		return false;
	if (box.y > y + h)
		return false;

	return true; // bounding boxes intersect
}

Direction Rect::ResolveX(Rect collider) {
	// Check left edge of collider
	if (x < collider.x && collider.x < x + w)
		return DIRECTION_RIGHT;

	// Check right edge of collider
	if (x < collider.x + collider.w && collider.x + collider.w < x + w)
		return DIRECTION_LEFT;

	return DIRECTION_NONE;
}

Direction Rect::ResolveY(Rect collider) {
	// Check top edge of collider
	if (y < collider.y && collider.y < y + h)
		return DIRECTION_DOWN;

	// Check bottom edge of collider
	if (y < collider.y + collider.h && collider.y + collider.h < y + h)
		return DIRECTION_UP;

	return DIRECTION_NONE;
}

void Rect::Flip(const TextureFlipType &flip, const Vector2i &axis) {
	if (flip == FLIP_NONE)
		return;

	if (flip == FLIP_X || flip == FLIP_XY)
		x = 2 * axis.x - x - w;

	if (flip == FLIP_Y || flip == FLIP_XY)
		y = 2 * axis.y - y - h;
}

void Rect::Draw(const int &XOffset, const int &YOffset, const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a) {
	int X = x + XOffset, Y = y + YOffset;

	DrawLine(X, Y, X + w, Y, r, g, b, a);
	DrawLine(X, Y, X, Y + h, r, g, b, a);
	DrawLine(X + w, Y, X + w, Y + h, r, g, b, a);
	DrawLine(X, Y + h, X + w, Y + h, r, g, b, a);
}

void Rect::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("x", gStrPool.Get(x)));
	child->append_attribute(doc.allocate_attribute("y", gStrPool.Get(y)));
	child->append_attribute(doc.allocate_attribute("w", gStrPool.Get(w)));
	child->append_attribute(doc.allocate_attribute("h", gStrPool.Get(h)));
	root->append_node(child);
}

} // End of namespace Crab
