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

#include "shape.h"
#include "stream-helper.h"

#include "math/line2d.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

static Vector2d asVec(const Point &p) {
	return Vector2d((float)p.x, (float)p.y);
}

static int sideOfLine(const Point &a, const Point &b, const Point &q) {
	return (b.x - a.x) * (q.y - a.y) - (b.y - a.y) * (q.x - a.x);
}

bool Polygon::contains(const Point &query) const {
	switch (_points.size()) {
	case 0: return false;
	case 1: return query == _points[0];
	default:
		// we assume that the polygon is convex
		for (uint i = 1; i < _points.size(); i++) {
			if (sideOfLine(_points[i - 1], _points[i], query) < 0)
				return false;
		}
		return sideOfLine(_points[_points.size() - 1], _points[0], query) >= 0;
	}
}

EdgeDistances Polygon::edgeDistances(uint startPointI, const Point &query) const {
	assert(startPointI < _points.size());
	uint endPointI = startPointI + 1 == _points.size() ? 0 : startPointI + 1;
	Vector2d
		a = asVec(_points[startPointI]),
		b = asVec(_points[endPointI]),
		q = asVec(query);
	float edgeLength = a.getDistanceTo(b);
	Vector2d edgeDir = (b - a) / edgeLength;
	Vector2d edgeNormal(-edgeDir.getY(), edgeDir.getX());
	EdgeDistances distances;
	distances._edgeLength = edgeLength;
	distances._onEdge = edgeDir.dotProduct(q - a);
	distances._toEdge = abs(edgeNormal.dotProduct(q) - edgeNormal.dotProduct(a));
	return distances;
}

static float depthAtForLine(const Point &a, const Point &b, const Point &q, int8 depthA, int8 depthB) {
	return (sqrtf(a.sqrDist(q)) / a.sqrDist(b) * depthB + depthA) * 0.01f;
}

static float depthAtForConvex(const PathFindingPolygon &p, const Point &q) {
	float sumDepths = 0, sumDistances = 0;
	for (uint i = 0; i < p._points.size(); i++) {
		uint j = i + 1 == p._points.size() ? 0 : i + 1;
		auto distances = p.edgeDistances(i, q);
		float depthOnEdge = p._pointDepths[i] + distances._onEdge * (p._pointDepths[j] - p._pointDepths[i]) / distances._edgeLength;
		if (distances._toEdge < epsilon) // q is directly on the edge
			return depthOnEdge;
		sumDepths += 1 / distances._toEdge * depthOnEdge;
		sumDistances += 1 / distances._toEdge;
	}
	return sumDepths / sumDistances * 0.01f;
}

float PathFindingPolygon::depthAt(const Point &query) const {
	switch (_points.size()) {
	case 0:
	case 1: return 1.0f;
	case 2: return depthAtForLine(_points[0], _points[1], query, _pointDepths[0], _pointDepths[1]);
	default: return depthAtForConvex(*this, query);
	}
}

static Color colorAtForLine(const Point &a, const Point &b, const Point &q, Color colorA, Color colorB) {
	// I highly suspect RGB calculation being very bugged, so for now I just ignore and only calc alpha
	float phase = sqrtf(q.sqrDist(a)) / a.sqrDist(b);
	colorA.a += phase * colorB.a;
	return colorA;
}

static Color colorAtForConvex(const FloorColorPolygon &p, const Point &query) {
	// This is a quite literal translation of the original engine
	// There may very well be a better way than this...
	float weights[FloorColorShape::kPointsPerPolygon];
	memset(weights, 0, sizeof(weights));

	for (uint i = 0; i < p._points.size(); i++) {
		EdgeDistances distances = p.edgeDistances(i, query);
		float edgeWeight = distances._toEdge * distances._onEdge / distances._edgeLength;
		if (distances._edgeLength > 1) {
			weights[i] += edgeWeight;
			weights[i + 1 == p._points.size() ? 0 : i + 1] += edgeWeight;
		}
	}
	float weightSum = 0;
	for (uint i = 0; i < p._points.size(); i++)
		weightSum += weights[i];
	for (uint i = 0; i < p._points.size(); i++) {
		if (weights[i] < epsilon)
			return p._pointColors[i];
		weights[i] = weightSum / weights[i];
	}

	weightSum = 0;
	for (uint i = 0; i < p._points.size(); i++)
		weightSum += weights[i];
	for (uint i = 0; i < p._points.size(); i++)
		weights[i] /= weightSum;

	float r = 0, g = 0, b = 0, a = 0.5f;
	for (uint i = 0; i < p._points.size(); i++) {
		r += p._pointColors[i].r * weights[i];
		g += p._pointColors[i].g * weights[i];
		b += p._pointColors[i].b * weights[i];
		a += p._pointColors[i].a * weights[i];
	}
	return {
		(byte)MIN(255, MAX(0, (int)r)),
		(byte)MIN(255, MAX(0, (int)g)),
		(byte)MIN(255, MAX(0, (int)b)),
		(byte)MIN(255, MAX(0, (int)a)),
	};
}

Color FloorColorPolygon::colorAt(const Point &query) const {
	switch (_points.size()) {
	case 0: return kWhite;
	case 1: return { 255, 255, 255, _pointColors[0].a };
	case 2: return colorAtForLine(_points[0], _points[1], query, _pointColors[0], _pointColors[1]);
	default: return colorAtForConvex(*this, query);
	}
}

Shape::Shape() {}

Shape::Shape(ReadStream &stream) {
	auto complexity = stream.readByte();
	uint8 pointsPerPolygon;
	if (complexity < 0 || complexity > 3)
		error("Invalid shape complexity %d", complexity);
	else if (complexity == 3)
		pointsPerPolygon = 0; // read in per polygon
	else
		pointsPerPolygon = 1 << complexity;

	int polygonCount = stream.readUint16LE();
	_polygons.reserve(polygonCount);
	_points.reserve(MIN(3, (int)pointsPerPolygon) * polygonCount);
	for (int i = 0; i < polygonCount; i++) {
		auto pointCount = pointsPerPolygon == 0
			? stream.readByte()
			: pointsPerPolygon;
		for (int j = 0; j < pointCount; j++)
			_points.push_back(readPoint(stream));
		addPolygon(pointCount);
	}
}

uint Shape::addPolygon(uint maxCount) {
	// Common functionality of shapes is that polygons are reduced
	// so that the first point is not duplicated
	uint firstI = empty() ? 0 : _polygons.back().first + _polygons.back().second;
	uint newCount = maxCount;
	if (maxCount > 1) {
		for (newCount = 1; newCount < maxCount; newCount++) {
			if (_points[firstI + newCount] == _points[firstI])
				break;
		}
		_points.resize(firstI + newCount);
	}
	_polygons.push_back({ firstI, newCount });
	return newCount;
}

Polygon Shape::at(uint index) const {
	auto range = _polygons[index];
	Polygon p;
	p._points = Span<const Point>(_points.data() + range.first, range.second);
	return p;
}

int32 Shape::polygonContaining(const Point &query) const {
	for (uint i = 0; i < _polygons.size(); i++) {
		if (at(i).contains(query))
			return (int32)i;
	}
	return -1;
}

bool Shape::contains(const Point &query) const {
	return polygonContaining(query) >= 0;
}

PathFindingShape::PathFindingShape() {}

PathFindingShape::PathFindingShape(ReadStream &stream) {
	auto polygonCount = stream.readUint16LE();
	_polygons.reserve(polygonCount);
	_polygonOrders.reserve(polygonCount);
	_points.reserve(polygonCount * kPointsPerPolygon);
	_pointDepths.reserve(polygonCount * kPointsPerPolygon);

	for (int i = 0; i < polygonCount; i++) {
		for (int j = 0; j < kPointsPerPolygon; j++)
			_points.push_back(readPoint(stream));
		_polygonOrders.push_back(stream.readSByte());
		for (int j = 0; j < kPointsPerPolygon; j++)
			_pointDepths.push_back(stream.readByte());

		uint pointCount = addPolygon(kPointsPerPolygon);
		assert(pointCount <= kPointsPerPolygon);
		_pointDepths.resize(_points.size());
	}

	// TODO: Implement the path finding
}

PathFindingPolygon PathFindingShape::at(uint index) const {
	auto range = _polygons[index];
	PathFindingPolygon p;
	p._points = Span<const Point>(_points.data() + range.first, range.second);
	p._pointDepths = Span<const uint8>(_pointDepths.data() + range.first, range.second);
	p._order = _polygonOrders[index];
	return p;
}

int8 PathFindingShape::orderAt(const Point &query) const {
	int32 polygon = polygonContaining(query);
	return polygon < 0 ? 49 : _polygonOrders[polygon];
}

float PathFindingShape::depthAt(const Point &query) const {
	int32 polygon = polygonContaining(query);
	return polygon < 0 ? 1.0f : at(polygon).depthAt(query);
}

FloorColorShape::FloorColorShape() {}

FloorColorShape::FloorColorShape(ReadStream &stream) {
	auto polygonCount = stream.readUint16LE();
	_polygons.reserve(polygonCount);
	_points.reserve(polygonCount * kPointsPerPolygon);
	_pointColors.reserve(polygonCount * kPointsPerPolygon);

	for (int i = 0; i < polygonCount; i++) {
		for (int j = 0; j < kPointsPerPolygon; j++)
			_points.push_back(readPoint(stream));
		Color color; // RGB and A components are stored separately
		for (int j = 0; j < kPointsPerPolygon; j++)
			color.a = stream.readByte();
		for (int j = 0; j < kPointsPerPolygon; j++) {
			color.r = stream.readByte();
			color.g = stream.readByte();
			color.b = stream.readByte();
			stream.readByte(); // second alpha value is ignored
		}
		_pointColors.push_back(color);
		stream.readByte(); // unused byte per polygon

		uint pointCount = addPolygon(kPointsPerPolygon);
		assert(pointCount <= kPointsPerPolygon);
		_pointColors.resize(_points.size());
	}
}

FloorColorPolygon FloorColorShape::at(uint index) const {
	auto range = _polygons[index];
	FloorColorPolygon p;
	p._points = Span<const Point>(_points.data() + range.first, range.second);
	p._pointColors = Span<const Color>(_pointColors.data() + range.first, range.second);
	return p;
}

OptionalColor FloorColorShape::colorAt(const Common::Point &query) const {
	int32 polygon = polygonContaining(query);
	return polygon < 0
		? OptionalColor(false, kClear)
		: OptionalColor(true, at(polygon).colorAt(query));
}

}
