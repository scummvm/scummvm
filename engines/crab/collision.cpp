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

#include "crab/collision.h"

// Find if a line and a rectangle intersect
bool CollideLineRect(int p0_x, int p0_y, int p1_x, int p1_y, const Rect &rect) {
	int q0_x = rect.x;
	int q0_y = rect.y;
	int q1_x = rect.x + rect.w;
	int q1_y = rect.y;

	if (CollideLine<int>(p0_x, p0_y, p1_x, p1_y, q0_x, q0_y, q1_x, q1_y))
		return true;

	q0_x = rect.x;
	q0_y = rect.y;
	q1_x = rect.x;
	q1_y = rect.y + rect.h;

	if (CollideLine<int>(p0_x, p0_y, p1_x, p1_y, q0_x, q0_y, q1_x, q1_y))
		return true;

	q0_x = rect.x + rect.w;
	q0_y = rect.y;
	q1_x = rect.x + rect.w;
	q1_y = rect.y + rect.h;

	if (CollideLine<int>(p0_x, p0_y, p1_x, p1_y, q0_x, q0_y, q1_x, q1_y))
		return true;

	q0_x = rect.x;
	q0_y = rect.y + rect.h;
	q1_x = rect.x + rect.w;
	q1_y = rect.y + rect.h;

	if (CollideLine<int>(p0_x, p0_y, p1_x, p1_y, q0_x, q0_y, q1_x, q1_y))
		return true;

	return false;
}

// Find if there is a clear line of sight between two rectangles
bool LineOfSight(const Rect &a, const Rect &b, const Rect &obstacle) {
	int p0_x = a.x;
	int p0_y = a.y;
	int p1_x = b.x;
	int p1_y = b.y;

	if (CollideLineRect(p0_x, p0_y, p1_x, p1_y, obstacle))
		return false;

	p0_x = a.x + a.w;
	p0_y = a.y;
	p1_x = b.x + b.w;
	p1_y = b.y;

	if (CollideLineRect(p0_x, p0_y, p1_x, p1_y, obstacle))
		return false;

	p0_x = a.x;
	p0_y = a.y + a.h;
	p1_x = b.x;
	p1_y = b.y + b.h;

	if (CollideLineRect(p0_x, p0_y, p1_x, p1_y, obstacle))
		return false;

	p0_x = a.x + a.w;
	p0_y = a.y + a.h;
	p1_x = b.x + b.w;
	p1_y = b.y + b.h;

	if (CollideLineRect(p0_x, p0_y, p1_x, p1_y, obstacle))
		return false;

	return true;
}

// float IntervalDistance(float minA, float maxA, float minB, float maxB)
//{
//	if (minA < minB)
//		return minB - maxA;
//
//	return minA - maxB;
// }
//
//// Calculate the projection of a polygon on an axis and returns it as a [min, max] interval
// void ProjectPolygon(const Vector2i &axis, const Polygon2D &poly, float &min, float &max)
//{
//	// To project a point on an axis use the dot product
//	float d = axis.DotProduct(poly.point[0]);
//	min = d;
//	max = d;
//
//	for(auto i = poly.point.begin(); i != poly.point.end(); ++i)
//	{
//		d = i->DotProduct(axis);
//
//		if (d < min) min = d;
//		else if (d > max) max = d;
//	}
// }
//
// PolygonCollisionResult Collide(const Polygon2D &polyA, const Polygon2D &polyB, const Vector2i &velocity)
//{
//	PolygonCollisionResult result;
//
//	int edgeCountA = polyA.edge.size();
//	int edgeCountB = polyB.edge.size();
//	float minIntervalDistance = std::numeric_limits<float>::max();
//	Vector2i translationAxis;
//	Vector2i edge;
//
//	// Loop through all the edges of both polygons
//	for(int edgeIndex = 0; edgeIndex < edgeCountA + edgeCountB; edgeIndex++)
//	{
//		if (edgeIndex < edgeCountA)
//			edge = polyA.edge[edgeIndex];
//		else
//			edge = polyB.edge[edgeIndex - edgeCountA];
//
//		// ===== 1. Find if the Polygon2Ds are currently intersecting =====
//
//		// Find the axis perpendicular to the current edge
//		Vector2i axis(-edge.y, edge.x);
//		axis.Normalize();
//
//		// Find the projection of the Polygon2D on the current axis
//		float minA = 0; float minB = 0; float maxA = 0; float maxB = 0;
//		ProjectPolygon(axis, polyA, minA, maxA);
//		ProjectPolygon(axis, polyB, minB, maxB);
//
//		// Check if the Polygon2D projections are currently intersecting
//		if (IntervalDistance(minA, maxA, minB, maxB) > 0)
//			result.intersect = false;
//
//		// ===== 2. Now find if the Polygon2Ds *will* intersect =====
//
//		// Project the velocity on the current axis
//		float velocityProjection = axis.DotProduct(velocity);
//
//		// Get the projection of Polygon2D A during the movement
//		if (velocityProjection < 0)
//			minA += velocityProjection;
//		else
//			maxA += velocityProjection;
//
//		// Do the same test as above for the new projection
//		float intervalDistance = IntervalDistance(minA, maxA, minB, maxB);
//		if (intervalDistance > 0)
//			result.will_intersect = false;
//
//		// If the Polygon2Ds are not intersecting and won't intersect, exit the loop
//		if (!result.intersect && !result.will_intersect)
//			break;
//
//		// Check if the current interval distance is the minimum one. If so store
//		// the interval distance and the current distance.
//		// This will be used to calculate the minimum translation vector
//		intervalDistance = abs(intervalDistance);
//		if (intervalDistance < minIntervalDistance)
//		{
//			minIntervalDistance = intervalDistance;
//			translationAxis = axis;
//
//			Vector2i d, ca, cb;
//			ca = polyA.Center();
//			cb = polyB.Center();
//
//			d.x = ca.x - cb.x;
//			d.y = ca.y - cb.y;
//
//			if (d.DotProduct(translationAxis) < 0)
//			{
//				translationAxis.x = -translationAxis.x;
//				translationAxis.y = -translationAxis.y;
//			}
//		}
//	}
//
//	// The minimum translation vector can be used to push the Polygon2Ds apart.
//	// First moves the Polygon2Ds by their velocity
//	// then move polyA by MinimumTranslationVector.
//	if (result.will_intersect)
//	{
//		result.mtv.x = translationAxis.x * minIntervalDistance;
//		result.mtv.y = translationAxis.y * minIntervalDistance;
//	}
//
//	return result;
// }
