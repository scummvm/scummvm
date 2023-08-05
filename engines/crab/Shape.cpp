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

#include "crab/Shape.h"

namespace Crab {

void Shape::load(rapidxml::xml_node<char> *node, const bool &echo) {
	if (nodeValid("polygon", node, echo)) {
		_type = SHAPE_POLYGON;
		_poly.load(node, _rect);
	} else {
		_rect.load(node, echo, "x", "y", "width", "height");
		if (nodeValid("ellipse", node, echo))
			_type = SHAPE_ELLIPSE;
		else
			_type = SHAPE_RECT;
	}
}

CollisionData Shape::collide(Rect box) const {
	CollisionData res;
	res._intersect = _rect.collide(box);

	if (res._intersect) {
		res._type = _type;
		if (_type == SHAPE_POLYGON) {
			PolygonCollisionResult pcr = _poly.collide(box);
			res._intersect = pcr._intersect;
			res._data.x = pcr._mtv.x;
			res._data.y = pcr._mtv.y;
			return res;
		} else
			res._data = _rect;
	}

	return res;
}

bool Shape::contains(const Vector2i &pos) {
	if (_rect.contains(pos)) {
		if (_type == SHAPE_POLYGON)
			return _poly.contains(pos.x, pos.y);
		else
			return true;
	}

	return false;
}

void Shape::draw(const int &xOffset, const int &yOffset, const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a) {
	if (_type == SHAPE_POLYGON)
		_poly.draw(xOffset, yOffset, r, g, b, a);
	else
		_rect.draw(xOffset, yOffset, r, g, b, a);
}

} // End of namespace Crab
