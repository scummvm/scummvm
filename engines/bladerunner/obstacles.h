/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_OBSTACLES_H
#define BLADERUNNER_OBSTACLES_H

#include "bladerunner/rect_float.h"
#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class Obstacles {
	static const int kVertexCount        = 150;
	static const int kPolygonCount       =  50;
	static const int kPolygonVertexCount = 160;

	enum VertexType {
		BOTTOM_LEFT,
		TOP_LEFT,
		TOP_RIGHT,
		BOTTOM_RIGHT
	};

	struct LineSegment {
		Vector2 start;
		Vector2 end;
	};

	struct Polygon {
		bool       isPresent;
		int        verticeCount;
		RectFloat  rect;
		Vector2    vertices[kPolygonVertexCount];
		VertexType vertexType[kPolygonVertexCount];

		Polygon() : isPresent(false), verticeCount(0)
		{}
	};

	BladeRunnerEngine *_vm;

	Polygon *_polygons;
	Polygon *_polygonsBackup;
	Vector2 *_vertices;
	int      _verticeCount;
	int      _count;
	bool     _backup;

	static bool lineLineIntersection(LineSegment a, LineSegment b, Vector2 *intersectionPoint);
	static bool linePolygonIntersection(LineSegment lineA, VertexType lineAType, Polygon *polyB, Vector2 *intersectionPoint, int *intersectionIndex);

	bool mergePolygons(Polygon &polyA, Polygon &PolyB);

public:
	Obstacles(BladeRunnerEngine *vm);
	~Obstacles();

	void clear();
	void add(RectFloat rect);
	void add(float x0, float z0, float x1, float z1) { add(RectFloat(x0, z0, x1, z1)); }
	int findEmptyPolygon() const;
	static float getLength(float x0, float z0, float x1, float z1);
	bool find(const Vector3 &from, const Vector3 &to, Vector3 *next) const;

	bool findIntersectionNearest(int polygonIndex, Vector2 from, Vector2 to,
	                             int *outVertexIndex, float *outDistance, Vector2 *out) const;
	bool findIntersectionFarthest(int polygonIndex, Vector2 from, Vector2 to,
	                              int *outVertexIndex, float *outDistance, Vector2 *out) const;

	bool findPolygonVerticeByXZ(int *polygonIndex, int *verticeIndex, int *verticeCount, float x, float z) const;
	bool findPolygonVerticeByXZWithinTolerance(float x, float z, int *polygonIndex, int *verticeIndex) const;

	void clearVertices();
	void copyVerticesReverse();
	void copyVertices();

	void backup();
	void restore();
	void reset();

	void draw();
	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
