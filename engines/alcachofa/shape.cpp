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

namespace Alcachofa {

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
	}
	_polygons.push_back({ firstI, newCount });
	return newCount;
}

Polygon Shape::at(uint index) {
	auto range = _polygons[index];
	Polygon p;
	p._points = Span<Point>(_points.data() + range.first, range.second);
	return p;
}

PathFindingShape::PathFindingShape() {}

PathFindingShape::PathFindingShape(ReadStream &stream) {
	auto polygonCount = stream.readUint16LE();
	_polygons.reserve(polygonCount);
	_polygonValues.reserve(polygonCount);
	_points.reserve(polygonCount * kPointsPerPolygon);
	_pointValues.reserve(polygonCount * kPointsPerPolygon);

	for (int i = 0; i < polygonCount; i++) {
		for (int j = 0; j < kPointsPerPolygon; j++)
			_points.push_back(readPoint(stream));
		_polygonValues.push_back(stream.readSByte());
		for (int j = 0; j < kPointsPerPolygon; j++)
			_pointValues.push_back(stream.readSByte());

		addPolygon(kPointsPerPolygon);
	}

	// TODO: Implement the path finding
}

PathFindingPolygon PathFindingShape::at(uint index) {
	auto range = _polygons[index];
	PathFindingPolygon p;
	p._points = Span<Point>(_points.data() + range.first, range.second);
	p._pointValues = Span<int8>(_pointValues.data() + range.first, range.second);
	p._polygonValue = _polygonValues[index];
	return p;
}

FloorColorShape::FloorColorShape() {}

FloorColorShape::FloorColorShape(ReadStream &stream) {
	auto polygonCount = stream.readUint16LE();
	_polygons.reserve(polygonCount);
	_polygonValues.reserve(polygonCount);
	_points.reserve(polygonCount * kPointsPerPolygon);
	_pointColors.reserve(polygonCount * kPointsPerPolygon);
	_pointWeights.reserve(polygonCount * kPointsPerPolygon);

	for (int i = 0; i < polygonCount; i++) {
		for (int j = 0; j < kPointsPerPolygon; j++)
			_points.push_back(readPoint(stream));
		for (int j = 0; j < kPointsPerPolygon; j++)
			_pointWeights.push_back(stream.readSByte());
		for (int j = 0; j < kPointsPerPolygon; j++)
			_pointColors.push_back(stream.readUint32LE());
		_polygonValues.push_back(stream.readSByte());

		addPolygon(kPointsPerPolygon);
	}
}

FloorColorPolygon FloorColorShape::at(uint index) {
	auto range = _polygons[index];
	FloorColorPolygon p;
	p._points = Span<Point>(_points.data() + range.first, range.second);
	p._pointWeights = Span<uint8>(_pointWeights.data() + range.first, range.second);
	p._pointColors = Span<uint32>(_pointColors.data() + range.first, range.second);
	p._polygonValue = _polygonValues[index];
	return p;
}

}
