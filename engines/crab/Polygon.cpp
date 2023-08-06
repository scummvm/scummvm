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

#include "crab/crab.h"
#include "crab/Polygon.h"

namespace Crab {

// Calculate the distance between [minA, maxA] and [minB, maxB]
// The distance will be negative if the intervals overlap
float IntervalDistance(float minA, float maxA, float minB, float maxB) {
	if (minA < minB)
		return minB - maxA;
	return minA - maxB;
}

void Polygon2D::addPoint(const Vector2f &ref, const Common::String &x, const Common::String &y, Vector2f &min, Vector2f &max) {
	Vector2f p;
	p.x = ref.x + stringToNumber<float>(x);
	p.y = ref.y + stringToNumber<float>(y);

	if (p.x < min.x)
		min.x = p.x;
	if (p.x > max.x)
		max.x = p.x;

	if (p.y < min.y)
		min.y = p.y;
	if (p.y > max.y)
		max.y = p.y;

	_point.push_back(p);
}

void Polygon2D::load(rapidxml::xml_node<char> *node, Rect &bounds) {
	Vector2f ref;
	ref.load(node);

	// Converting a polygon to an axis aligned bounding box is easy - just record the minimum and maximum values of x and y
	// for the vertices of the polygon, then minimum = top left corner, max - min = dimensions
	Vector2f min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	_point.clear();
	rapidxml::xml_node<char> *polynode = node->first_node("polygon");
	if (polynode != nullptr) {
		Common::String points, x, y;
		loadStr(points, "points", polynode);

		bool comma = false;
		for (auto i = points.begin(); i != points.end(); ++i) {
			if (*i == ',')
				comma = true;
			else if (*i == ' ') {
				addPoint(ref, x, y, min, max);
				comma = false;
				x.clear();
				y.clear();
			} else if (comma)
				y += *i;
			else
				x += *i;
		}

		addPoint(ref, x, y, min, max);

		bounds.x = min.x;
		bounds.y = min.y;
		bounds.w = max.x - min.x;
		bounds.h = max.y - min.y;
	}

	setEdge();
}

void Polygon2D::setEdge() {
	_edge.clear();
	Vector2f p1, p2, res;
	for (unsigned int i = 0; i < _point.size(); i++) {
		p1 = _point[i];
		if (i + 1 >= _point.size())
			p2 = _point[0];
		else
			p2 = _point[i + 1];

		res.x = p2.x - p1.x;
		res.y = p2.y - p1.y;
		_edge.push_back(res);
	}
}

Vector2f Polygon2D::center() const {
	Vector2f total;
	for (unsigned int i = 0; i < _point.size(); i++) {
		total.x += _point[i].x;
		total.y += _point[i].y;
	}

	Vector2f ret;
	if (_point.size() > 0) {
		ret.x = total.x / _point.size();
		ret.y = total.y / _point.size();
	}
	return ret;
}

void Polygon2D::offset(const float &x, const float &y) {
	for (auto i = _point.begin(); i != _point.end(); ++i) {
		i->x += x;
		i->y += y;
	}
}

void Polygon2D::project(const Vector2f &axis, float &min, float &max) const {
	// To project a point on an axis use the dot product
	float d = axis.dotProduct(_point[0]);
	min = d;
	max = d;

	for (auto i = _point.begin(); i != _point.end(); ++i) {
		d = i->dotProduct(axis);

		if (d < min)
			min = d;
		else if (d > max)
			max = d;
	}
}

PolygonCollisionResult Polygon2D::collide(const Rect &rect) const {
	Polygon2D polyB;
	Vector2f p;
	p.x = rect.x;
	p.y = rect.y;
	polyB._point.push_back(p);
	p.x = rect.x + rect.w;
	p.y = rect.y;
	polyB._point.push_back(p);
	p.x = rect.x + rect.w;
	p.y = rect.y + rect.h;
	polyB._point.push_back(p);
	p.x = rect.x;
	p.y = rect.y + rect.h;
	polyB._point.push_back(p);
	polyB.setEdge();

	return collide(polyB);
}

PolygonCollisionResult Polygon2D::collide(const Polygon2D &polyB) const {
	PolygonCollisionResult result;
	result._intersect = true;

	int edgeCountA = _edge.size();
	int edgeCountB = polyB._edge.size();
	float minIntervalDistance = std::numeric_limits<float>::max();
	Vector2f translationAxis;
	Vector2f e;

	// Loop through all the edges of both polygons
	for (int edgeIndex = 0; edgeIndex < edgeCountA + edgeCountB; edgeIndex++) {
		if (edgeIndex < edgeCountA)
			e = _edge[edgeIndex];
		else
			e = polyB._edge[edgeIndex - edgeCountA];

		// ===== 1. Find if the Polygon2Ds are currently intersecting =====

		// Find the axis perpendicular to the current edge
		Vector2f axis(-e.y, e.x);
		axis.normalize();

		// Find the projection of the Polygon2D on the current axis
		float minA = 0;
		float minB = 0;
		float maxA = 0;
		float maxB = 0;
		project(axis, minA, maxA);
		polyB.project(axis, minB, maxB);

		// Check if the Polygon2D projections are currently intersecting
		float intervalDistance = IntervalDistance(minA, maxA, minB, maxB);
		if (intervalDistance > 0) {
			// If the Polygon2Ds are not intersecting and won't intersect, exit the loop
			result._intersect = false;
			break;
		}

		// Check if the current interval distance is the minimum one. If so store
		// the interval distance and the current distance.
		// This will be used to calculate the minimum translation vector
		intervalDistance = abs(intervalDistance);
		if (intervalDistance < minIntervalDistance) {
			minIntervalDistance = intervalDistance;
			translationAxis = axis;

			Vector2f d, ca, cb;
			ca = center();
			cb = polyB.center();

			d.x = ca.x - cb.x;
			d.y = ca.y - cb.y;

			if (d.dotProduct(translationAxis) < 0) {
				translationAxis.x = -translationAxis.x;
				translationAxis.y = -translationAxis.y;
			}
		}
	}

	// The minimum translation vector can be used to push the Polygon2Ds apart.
	// First moves the Polygon2Ds by their velocity
	// then move polyA by MinimumTranslationVector.
	if (result._intersect) {
		result._mtv.x = translationAxis.x * minIntervalDistance;
		result._mtv.y = translationAxis.y * minIntervalDistance;
	}

	return result;
}

bool Polygon2D::contains(const float &x, const float &y) {
	bool result = false;

	for (unsigned int i = 0, j = _point.size() - 1; i < _point.size(); j = i++) {
		if (((_point[i].y > y) != (_point[j].y > y)) &&
			(x < (_point[j].x - _point[i].x) * (y - _point[i].y) / (_point[j].y - _point[i].y) + _point[i].x))
			result = !result;
	}

	return result;
}

void Polygon2D::draw(const int &xOffset, const int &yOffset, const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a) {
	Vector2f p1, p2;
	for (unsigned int i = 0; i < _point.size(); i++) {
		p1 = _point[i];
		if (i + 1 >= _point.size())
			p2 = _point[0];
		else
			p2 = _point[i + 1];

		g_engine->_screen->drawLine(p1.x + xOffset, p1.y + yOffset, p2.x + xOffset, p2.y + yOffset, g_engine->_format->ARGBToColor(a, r, g, b));
	}
}

} // End of namespace Crab
