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

#include "Polygon.h"

// Calculate the distance between [minA, maxA] and [minB, maxB]
// The distance will be negative if the intervals overlap
float IntervalDistance(float minA, float maxA, float minB, float maxB) {
	if (minA < minB)
		return minB - maxA;
	return minA - maxB;
}

void Polygon2D::AddPoint(const Vector2f &ref, const std::string &x, const std::string &y, Vector2f &min, Vector2f &max) {
	Vector2f p;
	p.x = ref.x + StringToNumber<float>(x);
	p.y = ref.y + StringToNumber<float>(y);

	if (p.x < min.x)
		min.x = p.x;
	if (p.x > max.x)
		max.x = p.x;

	if (p.y < min.y)
		min.y = p.y;
	if (p.y > max.y)
		max.y = p.y;

	point.push_back(p);
}

void Polygon2D::Load(rapidxml::xml_node<char> *node, Rect &bounds) {
	Vector2f ref;
	ref.Load(node);

	// Converting a polygon to an axis aligned bounding box is easy - just record the minimum and maximum values of x and y
	// for the vertices of the polygon, then minimum = top left corner, max - min = dimensions
	Vector2f min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector2f max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

	point.clear();
	rapidxml::xml_node<char> *polynode = node->first_node("polygon");
	if (polynode != NULL) {
		std::string points, x, y;
		LoadStr(points, "points", polynode);

		bool comma = false;
		for (auto i = points.begin(); i != points.end(); ++i) {
			if (*i == ',')
				comma = true;
			else if (*i == ' ') {
				AddPoint(ref, x, y, min, max);
				comma = false;
				x.clear();
				y.clear();
			} else if (comma)
				y.push_back(*i);
			else
				x.push_back(*i);
		}

		AddPoint(ref, x, y, min, max);

		bounds.x = min.x;
		bounds.y = min.y;
		bounds.w = max.x - min.x;
		bounds.h = max.y - min.y;
	}

	SetEdge();
}

void Polygon2D::SetEdge() {
	edge.clear();
	Vector2f p1, p2, res;
	for (int i = 0; i < point.size(); i++) {
		p1 = point[i];
		if (i + 1 >= point.size())
			p2 = point[0];
		else
			p2 = point[i + 1];

		res.x = p2.x - p1.x;
		res.y = p2.y - p1.y;
		edge.push_back(res);
	}
}

Vector2f Polygon2D::Center() const {
	Vector2f total;
	for (int i = 0; i < point.size(); i++) {
		total.x += point[i].x;
		total.y += point[i].y;
	}

	Vector2f ret;
	if (point.size() > 0) {
		ret.x = total.x / point.size();
		ret.y = total.y / point.size();
	}
	return ret;
}

void Polygon2D::Offset(const float &x, const float &y) {
	for (auto i = point.begin(); i != point.end(); ++i) {
		i->x += x;
		i->y += y;
	}
}

void Polygon2D::Project(const Vector2f &axis, float &min, float &max) const {
	// To project a point on an axis use the dot product
	float d = axis.DotProduct(point[0]);
	min = d;
	max = d;

	for (auto i = point.begin(); i != point.end(); ++i) {
		d = i->DotProduct(axis);

		if (d < min)
			min = d;
		else if (d > max)
			max = d;
	}
}

PolygonCollisionResult Polygon2D::Collide(const Rect &rect) {
	Polygon2D polyB;
	Vector2f p;
	p.x = rect.x;
	p.y = rect.y;
	polyB.point.push_back(p);
	p.x = rect.x + rect.w;
	p.y = rect.y;
	polyB.point.push_back(p);
	p.x = rect.x + rect.w;
	p.y = rect.y + rect.h;
	polyB.point.push_back(p);
	p.x = rect.x;
	p.y = rect.y + rect.h;
	polyB.point.push_back(p);
	polyB.SetEdge();

	return Collide(polyB);
}

PolygonCollisionResult Polygon2D::Collide(const Polygon2D &polyB) {
	PolygonCollisionResult result;
	result.intersect = true;

	int edgeCountA = edge.size();
	int edgeCountB = polyB.edge.size();
	float minIntervalDistance = std::numeric_limits<float>::max();
	Vector2f translationAxis;
	Vector2f e;

	// Loop through all the edges of both polygons
	for (int edgeIndex = 0; edgeIndex < edgeCountA + edgeCountB; edgeIndex++) {
		if (edgeIndex < edgeCountA)
			e = edge[edgeIndex];
		else
			e = polyB.edge[edgeIndex - edgeCountA];

		// ===== 1. Find if the Polygon2Ds are currently intersecting =====

		// Find the axis perpendicular to the current edge
		Vector2f axis(-e.y, e.x);
		axis.Normalize();

		// Find the projection of the Polygon2D on the current axis
		float minA = 0;
		float minB = 0;
		float maxA = 0;
		float maxB = 0;
		Project(axis, minA, maxA);
		polyB.Project(axis, minB, maxB);

		// Check if the Polygon2D projections are currently intersecting
		float intervalDistance = IntervalDistance(minA, maxA, minB, maxB);
		if (intervalDistance > 0) {
			// If the Polygon2Ds are not intersecting and won't intersect, exit the loop
			result.intersect = false;
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
			ca = Center();
			cb = polyB.Center();

			d.x = ca.x - cb.x;
			d.y = ca.y - cb.y;

			if (d.DotProduct(translationAxis) < 0) {
				translationAxis.x = -translationAxis.x;
				translationAxis.y = -translationAxis.y;
			}
		}
	}

	// The minimum translation vector can be used to push the Polygon2Ds apart.
	// First moves the Polygon2Ds by their velocity
	// then move polyA by MinimumTranslationVector.
	if (result.intersect) {
		result.mtv.x = translationAxis.x * minIntervalDistance;
		result.mtv.y = translationAxis.y * minIntervalDistance;
	}

	return result;
}

bool Polygon2D::Contains(const float &X, const float &Y) {
	bool result = false;

	for (int i = 0, j = point.size() - 1; i < point.size(); j = i++) {
		if (((point[i].y > Y) != (point[j].y > Y)) &&
			(X < (point[j].x - point[i].x) * (Y - point[i].y) / (point[j].y - point[i].y) + point[i].x))
			result = !result;
	}

	return result;
}

void Polygon2D::Draw(const int &XOffset, const int &YOffset, const Uint8 &r, const Uint8 &g, const Uint8 &b, const Uint8 &a) {
	Vector2f p1, p2;
	for (int i = 0; i < point.size(); i++) {
		p1 = point[i];
		if (i + 1 >= point.size())
			p2 = point[0];
		else
			p2 = point[i + 1];

		DrawLine(p1.x + XOffset, p1.y + YOffset, p2.x + XOffset, p2.y + YOffset, r, g, b, a);
	}
}
