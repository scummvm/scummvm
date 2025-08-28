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

using namespace Common;
using namespace Math;

namespace Alcachofa {

static Vector2d asVec(const Point &p) {
	return Vector2d((float)p.x, (float)p.y);
}

static int sideOfLine(const Point &a, const Point &b, const Point &q) {
	return (b.x - a.x) * (q.y - a.y) - (b.y - a.y) * (q.x - a.x);
}

static bool segmentsIntersect(const Point &a1, const Point &b1, const Point &a2, const Point &b2) {
	// as there are a number of special cases to consider, this method is a direct translation
	// of the original engine
	// TODO: It is still bad and does sometimes not work correctly. Check this. keep in mind
	// it *could* also be a case of incorrect floor segments being passed into in the first place.

	const auto sideOfLine = [](const Point &a, const Point &b, const Point q) {
		return Alcachofa::sideOfLine(a, b, q) > 0;
	};
	const auto lineIntersects = [&](const Point &a1, const Point &b1, const Point &a2, const Point &b2) {
		return sideOfLine(a1, b1, a2) != sideOfLine(a1, b1, b2);
	};

	if (a2.x > b2.x) {
		if (a1.x > b1.x)
			return lineIntersects(b1, a1, b2, a2) && lineIntersects(b2, a2, b1, a1);
		else
			return lineIntersects(a1, b1, b2, a2) && lineIntersects(b2, a2, a1, b1);
	}
	else {
		if (a1.x > b1.x)
			return lineIntersects(b1, a1, a2, b2) && lineIntersects(a2, b2, b1, a1);
		else
			return lineIntersects(a1, b1, a2, b2) && lineIntersects(a2, b2, a1, b1);
	}
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
			return depthOnEdge * 0.01f;
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

uint PathFindingPolygon::findSharedPoints(
	const PathFindingPolygon &other,
	Common::Span<SharedPoint> sharedPoints) const {
	uint count = 0;
	for (uint outerI = 0; outerI < _points.size(); outerI++) {
		for (uint innerI = 0; innerI < other._points.size(); innerI++) {
			if (_points[outerI] == other._points[innerI]) {
				assert(count < sharedPoints.size());
				sharedPoints[count++] = { outerI, innerI };
			}
		}
	}
	return count;
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
	p._index = index;
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

void Shape::setAsRectangle(const Rect &rect) {
	_polygons.resize(1);
	_polygons[0] = { 0, 4 };
	_points.resize(4);
	_points[0] = { rect.left, rect.top };
	_points[1] = { rect.right, rect.top };
	_points[2] = { rect.right, rect.bottom };
	_points[3] = { rect.left, rect.bottom };
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

	setupLinks();
	initializeFloydWarshall();
	calculateFloydWarshall();
}

PathFindingPolygon PathFindingShape::at(uint index) const {
	auto range = _polygons[index];
	PathFindingPolygon p;
	p._index = index;
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

PathFindingShape::LinkPolygonIndices::LinkPolygonIndices() {
	Common::fill(_points, _points + kPointsPerPolygon, LinkIndex( -1, -1 ));
}

static Pair<int32, int32> orderPoints(const Polygon &polygon, int32 point1, int32 point2) {
	if ((point1 > point2 && point1 + 1 != (int32)polygon._points.size()) ||
		point2 + 1 == (int32)polygon._points.size()) {
		int32 tmp = point1;
		point1 = point2;
		point2 = tmp;
	}
	return { point1, point2 };
}

void PathFindingShape::setupLinks() {
	// just a heuristic, each polygon will be attached to at least one other
	_linkPoints.reserve(polygonCount() * 3);
	_linkIndices.resize(polygonCount());
	_targetQuads.resize(polygonCount() * kPointsPerPolygon);
	Common::fill(_targetQuads.begin(), _targetQuads.end(), -1);
	Pair<uint, uint> sharedPoints[2];
	for (uint outerI = 0; outerI < polygonCount(); outerI++) {
		const auto outer = at(outerI);
		for (uint innerI = outerI + 1; innerI < polygonCount(); innerI++) {
			const auto inner = at(innerI);
			uint sharedPointCount = outer.findSharedPoints(inner, { sharedPoints, 2 });
			if (sharedPointCount > 0)
				setupLinkPoint(outer, inner, sharedPoints[0]);
			if (sharedPointCount > 1) {
				auto outerPoints = orderPoints(outer, sharedPoints[0].first, sharedPoints[1].first);
				auto innerPoints = orderPoints(inner, sharedPoints[0].second, sharedPoints[1].second);
				setupLinkEdge(outer, inner, outerPoints.first, outerPoints.second, innerPoints.first);
				setupLinkPoint(outer, inner, sharedPoints[1]);
			}
		}
	}
}

void PathFindingShape::setupLinkPoint(
	const PathFindingPolygon &outer,
	const PathFindingPolygon &inner,
	PathFindingPolygon::SharedPoint pointI) {
	auto &outerLink = _linkIndices[outer._index]._points[pointI.first];
	auto &innerLink = _linkIndices[inner._index]._points[pointI.second];
	if (outerLink.first < 0) {
		outerLink.first = _linkPoints.size();
		_linkPoints.push_back(outer._points[pointI.first]);
	}
	innerLink.first = outerLink.first;
}

void PathFindingShape::setupLinkEdge(
	const PathFindingPolygon &outer,
	const PathFindingPolygon &inner,
	int32 outerP1, int32 outerP2, int32 innerP) {
	_targetQuads[outer._index * kPointsPerPolygon + outerP1] = inner._index;
	_targetQuads[inner._index * kPointsPerPolygon + innerP] = outer._index;
	auto &outerLink = _linkIndices[outer._index]._points[outerP1];
	auto &innerLink = _linkIndices[inner._index]._points[innerP];
	if (outerLink.second < 0) {
		outerLink.second = _linkPoints.size();
		_linkPoints.push_back((outer._points[outerP1] + outer._points[outerP2]) / 2);
	}
	innerLink.second = outerLink.second;
}

void PathFindingShape::initializeFloydWarshall() {
	_distanceMatrix.resize(_linkPoints.size() * _linkPoints.size());
	_previousTarget.resize(_linkPoints.size() * _linkPoints.size());
	Common::fill(_distanceMatrix.begin(), _distanceMatrix.end(), UINT_MAX);
	Common::fill(_previousTarget.begin(), _previousTarget.end(), -1);

	// every linkpoint is the shortest path to itself
	for (uint i = 0; i < _linkPoints.size(); i++) {
		_distanceMatrix[i * _linkPoints.size() + i] = 0;
		_previousTarget[i * _linkPoints.size() + i] = i;
	}

	// every linkpoint to linkpoint within the same polygon *is* the shortest path
	// between them. Therefore these are our initial paths for Floyd-Warshall
	for (const auto &linkPolygon : _linkIndices) {
		for (uint i = 0; i < 2 * kPointsPerPolygon; i++) {
			LinkIndex linkFrom = linkPolygon._points[i / 2];
			int32 linkFromI = i % 2 ? linkFrom.second : linkFrom.first;
			if (linkFromI < 0)
				continue;
			for (uint j = i + 1; j < 2 * kPointsPerPolygon; j++) {
				LinkIndex linkTo = linkPolygon._points[j / 2];
				int32 linkToI = j % 2 ? linkTo.second : linkTo.first;
				if (linkToI >= 0) {
					const int32 linkFromFullI = linkFromI * _linkPoints.size() + linkToI;
					const int32 linkToFullI = linkToI * _linkPoints.size() + linkFromI;
					_distanceMatrix[linkFromFullI] = _distanceMatrix[linkToFullI] =
						(uint)sqrtf(_linkPoints[linkFromI].sqrDist(_linkPoints[linkToI]) + 0.5f);
					_previousTarget[linkFromFullI] = linkFromI;
					_previousTarget[linkToFullI] = linkToI;
				}
			}
		}
	}
}

void PathFindingShape::calculateFloydWarshall() {
	const auto distance = [&](uint a, uint b) -> uint& {
		return _distanceMatrix[a * _linkPoints.size() + b];
	};
	const auto previousTarget = [&](uint a, uint b) -> int32& {
		return _previousTarget[a * _linkPoints.size() + b];
	};
	for (uint over = 0; over < _linkPoints.size(); over++) {
		for (uint from = 0; from < _linkPoints.size(); from++) {
			for (uint to = 0; to < _linkPoints.size(); to++) {
				if (distance(from, over) != UINT_MAX && distance(over, to) != UINT_MAX &&
					distance(from, over) + distance(over, to) < distance(from, to)) {
					distance(from, to) = distance(from, over) + distance(over, to);
					previousTarget(from, to) = previousTarget(over, to);
				}
			}
		}
	}

	// in the game all floors should be fully connected
	assert(find(_previousTarget.begin(), _previousTarget.end(), -1) == _previousTarget.end());
}

bool PathFindingShape::findPath(const Point &from, const Point &to_, Stack<Point> &path) const {
	Point to = to_; // we might want to correct it
	path.clear();

	int32 fromContaining = polygonContaining(from);
	if (fromContaining < 0)
		return false;
	int32 toContaining = polygonContaining(to);
	if (toContaining < 0) {
		to = getClosestPoint(to);
		toContaining = polygonContaining(to);
		assert(toContaining >= 0);
	}
	//if (canGoStraightThrough(from, to, fromContaining, toContaining)) {
	if (canGoStraightThrough(from, to, fromContaining, toContaining)) {
		path.push(to);
		return true;
	}
	floydWarshallPath(from, to, fromContaining, toContaining, path);
	return true;
}

bool PathFindingShape::canGoStraightThrough(
	const Point &from, const Point &to,
	int32 fromContainingI, int32 toContainingI) const {
	int32 lastContainingI = -1;
	while (fromContainingI != toContainingI) {
		auto toContaining = at(toContainingI);
		bool foundPortal = false;
		for (uint i = 0; i < toContaining._points.size(); i++) {
			uint fullI = toContainingI * kPointsPerPolygon + i;
			if (_targetQuads[fullI] < 0 || _targetQuads[fullI] == lastContainingI)
				continue;

			uint j = i + 1 == toContaining._points.size() ? 0 : i + 1;
			if (segmentsIntersect(from, to, toContaining._points[i], toContaining._points[j])) {
				foundPortal = true;
				lastContainingI = toContainingI;
				toContainingI = _targetQuads[fullI];
				break;
			}
		}
		if (!foundPortal)
			return false;
	}
	return true;
}

void PathFindingShape::floydWarshallPath(
	const Point &from, const Point &to,
	int32 fromContaining, int32 toContaining,
	Stack<Point> &path) const {
	path.push(to);
	// first find the tuple of link points to be used
	uint fromLink = UINT_MAX, toLink = UINT_MAX, bestDistance = UINT_MAX;
	const auto &fromIndices = _linkIndices[fromContaining];
	const auto &toIndices = _linkIndices[toContaining];
	for (uint i = 0; i < 2 * kPointsPerPolygon; i++) {
		const auto &curFromPoint = fromIndices._points[i / 2];
		int32 curFromLink = i % 2 ? curFromPoint.second : curFromPoint.first;
		if (curFromLink < 0)
			continue;
		uint curFromDistance = (uint)sqrtf(from.sqrDist(_linkPoints[curFromLink]) + 0.5f);

		for (uint j = 0; j < 2 * kPointsPerPolygon; j++) {
			const auto &curToPoint = toIndices._points[j / 2];
			int32 curToLink = j % 2 ? curToPoint.second : curToPoint.first;
			if (curToLink < 0)
				continue;
			uint totalDistance =
				curFromDistance +
				_distanceMatrix[curFromLink * _linkPoints.size() + curToLink] +
				(uint)sqrtf(to.sqrDist(_linkPoints[curToLink]) + 0.5f);
			if (totalDistance < bestDistance) {
				bestDistance = totalDistance;
				fromLink = curFromLink;
				toLink = curToLink;
			}
		}
	}
	assert(fromLink != UINT_MAX && toLink != UINT_MAX);

	// then walk the matrix back to reconstruct the path
	while (fromLink != toLink) {
		path.push(_linkPoints[toLink]);
		toLink = _previousTarget[fromLink * _linkPoints.size() + toLink];
		assert(toLink < _linkPoints.size());
	}
	path.push(_linkPoints[fromLink]);
}

Point PathFindingShape::getClosestPoint(const Point &query) const {
	// TODO: Improve this function, it does not seem correct

	assert(!_points.empty());
	Point bestPoint;
	uint bestDistance = UINT_MAX;
	for (auto p : _points) {
		uint curDistance = query.sqrDist(p);
		if (curDistance < bestDistance) {
			bestDistance = curDistance;
			bestPoint = p;
		}
	}

	assert(bestDistance < UINT_MAX);
	return bestPoint;
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
	p._index = index;
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
