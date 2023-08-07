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

#include "graphics/screen.h"
#include "crab/crab.h"
#include "crab/Rectangle.h"

namespace Crab {

bool Rect::load(rapidxml::xml_node<char> *node, const bool &echo, const Common::String &x_name, const Common::String &y_name,
				const Common::String &w_name, const Common::String &h_name) {
	return loadNum(x, x_name, node, echo) && loadNum(y, y_name, node, echo) && loadNum(w, w_name, node, echo) && loadNum(h, h_name, node, echo);
}

bool Rect::collide(Rect box) const {
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

void Rect::extend(Rect box) {
	int left1 = x;
	int right1 = x + w;
	int top1 = y;
	int bottom1 = y + h;

	const int left2 = box.x;
	const int right2 = box.x + box.w;
	const int top2 = box.y;
	const int bottom2 = box.y + box.h;

	left1 = MIN(left1, left2);
	right1 = MAX(right1, right2);
	top1 = MIN(top1, top2);
	bottom1 = MAX(bottom1, bottom2);

	x = left1;
	y = top1;
	w = right1 - left1;
	h = bottom1 - top1;
}

Direction Rect::resolveX(Rect collider) {
	// Check left edge of collider
	if (x < collider.x && collider.x < x + w)
		return DIRECTION_RIGHT;

	// Check right edge of collider
	if (x < collider.x + collider.w && collider.x + collider.w < x + w)
		return DIRECTION_LEFT;

	return DIRECTION_NONE;
}

Direction Rect::resolveY(Rect collider) {
	// Check top edge of collider
	if (y < collider.y && collider.y < y + h)
		return DIRECTION_DOWN;

	// Check bottom edge of collider
	if (y < collider.y + collider.h && collider.y + collider.h < y + h)
		return DIRECTION_UP;

	return DIRECTION_NONE;
}

void Rect::flip(const TextureFlipType &flip, const Vector2i &axis) {
	if (flip == FLIP_NONE)
		return;

	if (flip == FLIP_X || flip == FLIP_XY)
		x = 2 * axis.x - x - w;

	if (flip == FLIP_Y || flip == FLIP_XY)
		y = 2 * axis.y - y - h;
}

void Rect::draw(const int &xOffset, const int &yOffset, const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a) {
	int X = x + xOffset, Y = y + yOffset;

	g_engine->_screen->drawLine(X, Y, X + w, Y, g_engine->_format->ARGBToColor(a, r, g, b));
	g_engine->_screen->drawLine(X, Y, X, Y + h, g_engine->_format->ARGBToColor(a, r, g, b));
	g_engine->_screen->drawLine(X + w, Y, X + w, Y + h, g_engine->_format->ARGBToColor(a, r, g, b));
	g_engine->_screen->drawLine(X, Y + h, X + w, Y + h, g_engine->_format->ARGBToColor(a, r, g, b));
}

void Rect::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("x", g_engine->_stringPool->Get(x)));
	child->append_attribute(doc.allocate_attribute("y", g_engine->_stringPool->Get(y)));
	child->append_attribute(doc.allocate_attribute("w", g_engine->_stringPool->Get(w)));
	child->append_attribute(doc.allocate_attribute("h", g_engine->_stringPool->Get(h)));
	root->append_node(child);
}

} // End of namespace Crab
