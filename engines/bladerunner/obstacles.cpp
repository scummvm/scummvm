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

#include "bladerunner/obstacles.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h" // for debug
#include "bladerunner/set.h"
#include "bladerunner/view.h"

#include "common/debug.h"

#define DISABLE_PATHFINDING 0
#define USE_PATHFINDING_EXPERIMENTAL_FIX_2 0 // Alternate Fix: Allows polygons merged on one point

#define WITHIN_TOLERANCE(a, b) (((a) - 0.009) < (b) && ((a) + 0.009) > (b))

namespace BladeRunner {

Obstacles::Obstacles(BladeRunnerEngine *vm) {
	_vm = vm;
	_polygons       = new Polygon[kPolygonCount];
	_polygonsBackup = new Polygon[kPolygonCount];
	_path           = new Vector2[kVertexCount];
	clear();
}

Obstacles::~Obstacles() {
	clear();

	delete[] _polygons;
	_polygons = nullptr;

	delete[] _polygonsBackup;
	_polygonsBackup = nullptr;

	delete[] _path;
	_path = nullptr;
}

void Obstacles::clear() {
	for (int i = 0; i < kPolygonCount; ++i) {
		_polygons[i].isPresent = false;
		_polygons[i].verticeCount = 0;
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			_polygons[i].vertices[j].x = 0.0f;
			_polygons[i].vertices[j].y = 0.0f;
		}
	}
	_pathSize = 0;
	_backup = false;
	_count = 0;
}

#define IN_RANGE(v, start, end) ((start) <= (v) && (v) <= (end))

/*
 * This function is limited to finding intersections between
 * horizontal and vertical lines!
 *
 * The original implementation is more general but obstacle
 * polygons only consists of horizontal and vertical lines,
 * and this is more numerically stable.
 */
bool Obstacles::lineLineIntersection(LineSegment a, LineSegment b, Vector2 *intersection) {
	assert(a.start.x == a.end.x || a.start.y == a.end.y);
	assert(b.start.x == b.end.x || b.start.y == b.end.y);

	if (a.start.x > a.end.x) SWAP(a.start.x, a.end.x);
	if (a.start.y > a.end.y) SWAP(a.start.y, a.end.y);
	if (b.start.x > b.end.x) SWAP(b.start.x, b.end.x);
	if (b.start.y > b.end.y) SWAP(b.start.y, b.end.y);

	if (a.start.x == a.end.x && b.start.y == b.end.y && IN_RANGE(a.start.x, b.start.x, b.end.x) && IN_RANGE(b.start.y, a.start.y, a.end.y)) {
		// A is vertical, B is horizontal
		*intersection = Vector2(a.start.x, b.start.y);
		return true;
	}

	if (a.start.y == a.end.y && b.start.x == b.end.x && IN_RANGE(a.start.y, b.start.y, b.end.y) && IN_RANGE(b.start.x, a.start.x, a.end.x)) {
		// A is horizontal, B is vertical
		*intersection = Vector2(b.start.x, a.start.y);
		return true;
	}

	return false;
}

bool Obstacles::linePolygonIntersection(LineSegment lineA, VertexType lineAType, Polygon *polyB, Vector2 *intersectionPoint, int *intersectionIndex, int pathLengthSinceLastIntersection) {
	bool hasIntersection = false;
	float nearestIntersectionDistance = 0.0f;

	for (int i = 0; i != polyB->verticeCount; ++i) {
		LineSegment lineB;
		lineB.start = polyB->vertices[i];
		lineB.end   = polyB->vertices[(i+1) % polyB->verticeCount];

		VertexType lineBType = polyB->vertexType[i];

		Vector2 newIntersectionPoint;

		if (lineLineIntersection(lineA, lineB, &newIntersectionPoint)) {
			if ((lineAType == TOP_RIGHT    && lineBType == TOP_LEFT)
			 || (lineAType == BOTTOM_RIGHT && lineBType == TOP_RIGHT)
			 || (lineAType == BOTTOM_LEFT  && lineBType == BOTTOM_RIGHT)
			 || (lineAType == TOP_LEFT     && lineBType == BOTTOM_LEFT)
			) {
				if ( (pathLengthSinceLastIntersection > 2)
					|| ( (!(WITHIN_TOLERANCE(lineB.end.x, intersectionPoint->x) && WITHIN_TOLERANCE(lineB.end.y, intersectionPoint->y)))
					&& (newIntersectionPoint != *intersectionPoint) )) {
					float newIntersectionDistance = getLength(lineA.start.x, lineA.start.y, newIntersectionPoint.x, newIntersectionPoint.y);
					if (!hasIntersection || newIntersectionDistance < nearestIntersectionDistance) {
						hasIntersection = true;
						nearestIntersectionDistance = newIntersectionDistance;
						*intersectionPoint = newIntersectionPoint;
						*intersectionIndex = i;
					}
				}
			}
		}
	}

	return hasIntersection;
}

/*
 * Polygons vertices are defined in clock-wise order
 * starting at the top-most, right-most corner.
 *
 * When merging two polygons, we start at the top-most, right-most vertex.
 * The polygon with this vertex starts is the primary polygon.
 * We follow the edges until we find an intersection with the secondary polygon,
 * in which case we switch primary and secondary and continue following the new edges.
 *
 * Luckily the first two polygons added in RC01 (A, then B) are laid as as below,
 * making an ideal test case.
 *
 * Merge order: (B0,B1) (B1,B2) (B2,J) (J,A2) (A2,A3) (A3,A0) (A0,I) (I,B0)
 *
 *   0,0 ---> x
 *   |
 *   |                   primary
 *   |      B 0 ----- 1
 *   |        |       |
 *   |  A 0 --I-- 1   |
 *   |    |   |   |   |
 *   |    |   3 --J-- 2
 *   |    |       |
 *   |    3 ----- 2
 *   |               secondary
 *   v y
 */

bool Obstacles::mergePolygons(Polygon &polyA, Polygon &polyB) {
	bool flagDidMergePolygons = false;
	Polygon polyMerged;
	polyMerged.rect = merge(polyA.rect, polyB.rect);

	Polygon *polyPrimary, *polySecondary;
	if (polyA.rect.y0 < polyB.rect.y0 || (polyA.rect.y0 == polyB.rect.y0 && polyA.rect.x0 < polyB.rect.x0)) {
		polyPrimary = &polyA;
		polySecondary = &polyB;
	} else {
		polyPrimary = &polyB;
		polySecondary = &polyA;
	}

	Vector2 intersectionPoint;
	LineSegment polyLine;
	bool flagAddVertexToVertexList = true;
	bool flagDidFindIntersection = false;
	int vertIndex = 0;
	int pathLengthSinceLastIntersection = 0; // Part of pathfinding fix 2. It's only updated when enabling that fix, otherwise it is always zero (0).

	Polygon *startingPolygon = polyPrimary;
	int flagDone = false;
	while (!flagDone) {
		VertexType polyPrimaryType;

		polyLine.start  = flagDidFindIntersection ? intersectionPoint : polyPrimary->vertices[vertIndex];
		polyLine.end    = polyPrimary->vertices[(vertIndex + 1) % polyPrimary->verticeCount];

		// TODO(madmoose): How does this work when adding a new intersection point?
		polyPrimaryType = polyPrimary->vertexType[vertIndex];

		if (flagAddVertexToVertexList) {
#if USE_PATHFINDING_EXPERIMENTAL_FIX_2
			assert(polyMerged.verticeCount < kPolygonVertexCount);
#else
			// In some cases polygons will have only one intersection (touching corners) and because of that second SWAP never occurs,
			// algorithm will stop only when the merged polygon is full.
			if (polyMerged.verticeCount >= kPolygonVertexCount) {
				flagDidMergePolygons = false;
				break;
			}
#endif
			polyMerged.vertices[polyMerged.verticeCount] = polyLine.start;
			polyMerged.vertexType[polyMerged.verticeCount] = polyPrimaryType;
			++(polyMerged.verticeCount);
		}

		flagAddVertexToVertexList = true;
		int polySecondaryIntersectionIndex = -1;

		if (linePolygonIntersection(polyLine, polyPrimaryType, polySecondary, &intersectionPoint, &polySecondaryIntersectionIndex, pathLengthSinceLastIntersection)) {
			if (WITHIN_TOLERANCE(intersectionPoint.x, polyLine.start.x) && WITHIN_TOLERANCE(intersectionPoint.y, polyLine.start.y)) {
				flagAddVertexToVertexList = false;
				// TODO(madmoose): How would this work?
				--(polyMerged.verticeCount);
			} else {
				// Obstacles::nop
			}
			vertIndex = polySecondaryIntersectionIndex;
			flagDidFindIntersection = true;

			SWAP(polyPrimary, polySecondary);

			flagDidMergePolygons = true;
#if USE_PATHFINDING_EXPERIMENTAL_FIX_2
			pathLengthSinceLastIntersection = 0;
#endif
		} else {
			vertIndex = (vertIndex + 1) % polyPrimary->verticeCount;
#if USE_PATHFINDING_EXPERIMENTAL_FIX_2
			++pathLengthSinceLastIntersection;
#endif
			flagDidFindIntersection = false;
		}
		if (polyPrimary->vertices[vertIndex] == startingPolygon->vertices[0]) {
			flagDone = true;
		}
	}

	if (flagDidMergePolygons) {
		*startingPolygon = polyMerged;
		startingPolygon->isPresent = true;
		if (startingPolygon == &polyA) {
			polyB.isPresent = false;
		} else {
			polyA.isPresent = false;
		}
	}

	return flagDidMergePolygons;
}

void Obstacles::add(RectFloat rect) {
	int polygonIndex = findEmptyPolygon();
	if (polygonIndex < 0) {
		return;
	}

	rect.expand(12.0f);
	rect.trunc_2_decimals();

	Polygon &poly = _polygons[polygonIndex];

	poly.rect = rect;

	poly.vertices[0] = Vector2(rect.x0, rect.y0);
	poly.vertexType[0] = TOP_LEFT;

	poly.vertices[1] = Vector2(rect.x1, rect.y0);
	poly.vertexType[1] = TOP_RIGHT;

	poly.vertices[2] = Vector2(rect.x1, rect.y1);
	poly.vertexType[2] = BOTTOM_RIGHT;

	poly.vertices[3] = Vector2(rect.x0, rect.y1);
	poly.vertexType[3] = BOTTOM_LEFT;

	poly.isPresent = true;
	poly.verticeCount = 4;

restart:
	for (int i = 0; i < kPolygonCount; ++i) {
		Polygon &polyA = _polygons[i];
		if (!polyA.isPresent) {
			continue;
		}

		if (polyA.verticeCount == 0) {
				continue;
		}

		for (int j = i+1; j < kPolygonCount; ++j) {
			Polygon &polyB = _polygons[j];
			if (!polyB.isPresent) {
				continue;
			}

			if (polyB.verticeCount == 0) {
				continue;
			}

			if (!overlaps(polyA.rect, polyB.rect)) {
				continue;
			}

			if (mergePolygons(polyA, polyB)) {
				goto restart;
			}
		}
	}
}

int Obstacles::findEmptyPolygon() const {
	for (int i = 0; i < kPolygonCount; ++i) {
		if (!_polygons[i].isPresent) {
			return i;
		}
	}
	return -1;
}

float Obstacles::getLength(float x0, float z0, float x1, float z1) {
	if (x0 == x1) {
		return fabs(z1 - z0);
	}
	return fabs(x1 - x0);
}

#if DISABLE_PATHFINDING
bool Obstacles::findNextWaypoint(const Vector3 &from, const Vector3 &to, Vector3 *next) {
	*next = to;

	return true;
}
#else

bool Obstacles::findNextWaypoint(const Vector3 &from, const Vector3 &to, Vector3 *next) {
	static int  recursionLevel = 0;
	static bool polygonVisited[kPolygonCount];

	if (++recursionLevel == 1) {
		clearPath();
		for (int i = 0; i != kPolygonCount; ++i) {
			polygonVisited[i] = false;
		}
	}

	int     polyIndex = -1;
	int     polyNearVertIndex = -1;
	float   polyNearDist = 0.0f;
	Vector2 polyNearPos;
	int     polyFarVertIndex = -1;
	float   polyFarDist = 0.0f;
	Vector2 polyFarPos;

	for (int i = 0; i != kPolygonCount; ++i) {
		Polygon &poly = _polygons[i];
		if (!poly.isPresent || polygonVisited[i]) {
			continue;
		}

		int     nearVertIndex;
		float   nearDist;
		Vector2 nearPos;

		if (!findIntersectionNearest(i, from.xz(), to.xz(), &nearVertIndex, &nearDist, &nearPos)) {
			continue;
		}

		int     farVertIndex;
		float   farDist;
		Vector2 farPos;

		int hasFar = findIntersectionFarthest(i, from.xz(), to.xz(), &farVertIndex, &farDist, &farPos);
		assert(hasFar);

		if (polyIndex == -1 || nearDist < polyNearDist) {
			polyNearDist = nearDist;
			polyNearPos = nearPos;
			polyFarDist = farDist;
			polyFarPos = farPos;
			polyIndex = i;
			polyNearVertIndex = nearVertIndex;
			polyFarVertIndex = farVertIndex;
		}
	}

	if (polyIndex < 0) {
		assert(_pathSize < kVertexCount);
		_path[_pathSize++] = to.xz();
	} else {
		polygonVisited[polyIndex] = true;

		if (polyNearDist == 0.0f && polyFarDist == 0.0f) {
			assert(_pathSize < kVertexCount);
			_path[_pathSize++] = polyNearPos;
		} else {
			Vector2 pathA[kMaxPathSize];
			Vector2 pathB[kMaxPathSize];

			bool pathABlocked;
			bool pathBBlocked;

			int pathASize = buildNegativePath(polyIndex, polyNearVertIndex, polyNearPos, polyFarVertIndex, polyFarPos, pathA, kMaxPathSize, &pathABlocked);
			int pathBSize = buildPositivePath(polyIndex, polyNearVertIndex, polyNearPos, polyFarVertIndex, polyFarPos, pathB, kMaxPathSize, &pathBBlocked);

			float pathATotalDistance = pathTotalDistance(pathA, pathASize, from.xz(), to.xz());
			float pathBTotalDistance = pathTotalDistance(pathB, pathBSize, from.xz(), to.xz());

			bool usePathA;
			if (pathABlocked && !pathBBlocked) {
				usePathA = false;
			} else if (pathBBlocked && !pathABlocked) {
				usePathA = true;
			} else {
				usePathA = pathATotalDistance <= pathBTotalDistance;
			}

			if (usePathA) {
				assert(_pathSize + pathASize < kVertexCount);
				for (int i = 0; i != pathASize; ++i) {
					_path[_pathSize + i] = pathA[i];
				}
				_pathSize += pathASize;
			} else {
				assert(_pathSize + pathBSize < kVertexCount);
				for (int i = 0; i != pathBSize; ++i) {
					_path[_pathSize + i] = pathB[i];
				}
				_pathSize += pathBSize;
			}
		}
		assert(_pathSize > 0);
		Vector3 lastPathPos(_path[_pathSize - 1].x, from.y, _path[_pathSize - 1].y);
		findNextWaypoint(lastPathPos, to, next);
	}

	if (--recursionLevel > 1) {
		return false;
	}

	return findFarthestAvailablePathVertex(_path, _pathSize, from, next);
}
#endif

bool Obstacles::findIntersectionNearest(int polygonIndex, Vector2 from, Vector2 to,
                                        int *outVertexIndex, float *outDistance, Vector2 *out) const
{
	float   minDistance = 0.0f;
	Vector2 minIntersection;
	int     minVertexIndex = -1;

	bool hasDistance = false;

	for (int i = 0; i < _polygons[polygonIndex].verticeCount; ++i) {
		int nextIndex = (i + 1) % _polygons[polygonIndex].verticeCount;
		Vector2 *vertices = _polygons[polygonIndex].vertices;
		Vector2 intersection;
		bool intersects = lineIntersection(from, to, vertices[i], vertices[nextIndex], &intersection);
		if (intersects) {
			float distance2 = distance(from, intersection);
			if (!hasDistance || distance2 < minDistance) {
				minDistance = distance2;
				minIntersection = intersection;
				minVertexIndex = i;
				hasDistance = true;
			}
		}
	}

	*outDistance    = minDistance;
	*outVertexIndex = minVertexIndex;
	*out            = minIntersection;

	return minVertexIndex != -1;
}

bool Obstacles::findIntersectionFarthest(int polygonIndex, Vector2 from, Vector2 to,
                                         int *outVertexIndex, float *outDistance, Vector2 *out) const
{
	float   maxDistance = 0.0f;
	Vector2 maxIntersection;
	int     maxVertexIndex = -1;

	bool hasDistance = false;

	for (int i = 0; i < _polygons[polygonIndex].verticeCount; ++i) {
		int nextIndex = (i + 1) % _polygons[polygonIndex].verticeCount;
		Vector2 *vertices = _polygons[polygonIndex].vertices;
		Vector2 intersection;
		bool intersects = lineIntersection(from, to, vertices[i], vertices[nextIndex], &intersection);
		if (intersects) {
			float distance2 = distance(from, intersection);
			if (!hasDistance || distance2 > maxDistance) {
				maxDistance = distance2;
				maxIntersection = intersection;
				maxVertexIndex = i;
				hasDistance = true;
			}
		}
	}

	*outDistance    = maxDistance;
	*outVertexIndex = maxVertexIndex;
	*out            = maxIntersection;

	return maxVertexIndex != -1;
}

float Obstacles::pathTotalDistance(const Vector2 *path, int pathSize, Vector2 from, Vector2 to) const {
	// Yes, 'to' and 'from' are ignored.
	float totalDistance = 0.0f;
	for (int i = 0; i != pathSize - 1; ++i) {
		totalDistance += distance(path[i], path[i+1]);
	}
	return totalDistance;
}


bool Obstacles::findPolygonVerticeByXZ(int *polygonIndex, int *verticeIndex, int *verticeCount, float x, float z) const {
	*polygonIndex = -1;
	*verticeIndex = -1;
	*verticeCount = -1;

	for (int i = 0; i != kPolygonCount; ++i) {
		if (!_polygons[i].isPresent || _polygons[i].verticeCount == 0) {
			continue;
		}

		for (int j = 0; j != _polygons[i].verticeCount; ++j) {
			if (_polygons[i].vertices[j].x == x && _polygons[i].vertices[j].y == z) {
				*polygonIndex = i;
				*verticeIndex = j;
				*verticeCount = _polygons[i].verticeCount;
				return true;
			}
		}
	}

	return false;
}

bool Obstacles::findPolygonVerticeByXZWithinTolerance(float x, float z, int *polygonIndex, int *verticeIndex, int startSearchFromPolygonIdx) const {
	*polygonIndex = -1;
	*verticeIndex = -1;

//	for (int i = 0; i != kPolygonCount; ++i) {
	for (int countUp = 0, i = startSearchFromPolygonIdx; countUp != kPolygonCount; ++countUp, ++i) {
		i = i  % kPolygonCount;	// we want to circle around to go through all polygons
		if (!_polygons[i].isPresent || _polygons[i].verticeCount == 0) {
			continue;
		}

		for (int j = 0; j != _polygons[i].verticeCount; ++j) {
			if (WITHIN_TOLERANCE(_polygons[i].vertices[j].x, x) && WITHIN_TOLERANCE(_polygons[i].vertices[j].y, z)) {
				*polygonIndex = i;
				*verticeIndex = j;
				return true;
			}
		}
	}

	return false;
}

void Obstacles::clearPath() {
	_pathSize = 0;
}

int Obstacles::buildNegativePath(int polyIndex, int vertStartIndex, Vector2 startPos, int vertEndIndex, Vector2 endPos, Vector2 *path, int pathCapacity, bool *pathBlocked) {
	int pathSize = 0;
	*pathBlocked = false;
	Polygon *poly = &_polygons[polyIndex];

	/* Add start position to path */
	if (_vm->_scene->_set->findWalkbox(startPos.x, startPos.y) == -1) {
		*pathBlocked = true;
	}
	assert(pathSize < pathCapacity);
	path[pathSize++] = startPos;

	int i = vertStartIndex;

	/* Add polygon vertices in negative iteration order */
	while (true) {
		Vector2 v = poly->vertices[i];
		if (_vm->_scene->_set->findWalkbox(v.x, v.y) == -1) {
			*pathBlocked = true;
		}

		assert(pathSize < pathCapacity);
		path[pathSize++] = v;

		i = (i + poly->verticeCount - 1) % poly->verticeCount;
		if (i == vertEndIndex) {
			break;
		}
	}

	/* Add end position to path */
	if (_vm->_scene->_set->findWalkbox(endPos.x, endPos.y) == -1) {
		*pathBlocked = true;
	}
	assert(pathSize < pathCapacity);
	path[pathSize++] = endPos;

	return pathSize;
}

int Obstacles::buildPositivePath(int polyIndex, int vertStartIndex, Vector2 startPos, int vertEndIndex, Vector2 endPos, Vector2 *path, int pathCapacity, bool *pathBlocked) {
	int pathSize = 0;
	*pathBlocked = false;
	Polygon *poly = &_polygons[polyIndex];

	/* Add start position to path */
	if (_vm->_scene->_set->findWalkbox(startPos.x, startPos.y) == -1) {
		*pathBlocked = true;
	}
	assert(pathSize < pathCapacity);
	path[pathSize++] = startPos;

	int i = (vertStartIndex + 1) % poly->verticeCount;

	/* Add polygon vertices in positive iteration order */
	while (true) {
		Vector2 v = poly->vertices[i];
		if (_vm->_scene->_set->findWalkbox(v.x, v.y) == -1) {
			*pathBlocked = true;
		}

		assert(pathSize < pathCapacity);
		path[pathSize++] = v;

		if (i == vertEndIndex) {
			break;
		}

		i = (i + 1) % poly->verticeCount;
	}

	/* Add end position to path */
	if (_vm->_scene->_set->findWalkbox(endPos.x, endPos.y) == -1) {
		*pathBlocked = true;
	}
	assert(pathSize < pathCapacity);
	path[pathSize++] = endPos;

	return pathSize;
}

bool Obstacles::verticesCanIntersect(int lineType0, int lineType1, float x0, float y0, float x1, float y1) const {
	if (lineType0 == TOP_LEFT && lineType1 == TOP_RIGHT) {
		if (x0 > x1 && y0 < y1) return true;
	}
	if (lineType0 == TOP_RIGHT && lineType1 == BOTTOM_RIGHT) {
		if (x0 > x1 && y0 > y1) return true;
	}
	if (lineType0 == BOTTOM_RIGHT && lineType1 == BOTTOM_LEFT) {
		if (x0 < x1 && y0 > y1) return true;
	}
	if (lineType0 == BOTTOM_LEFT && lineType1 == TOP_LEFT) {
		if (x0 < x1 && y0 < y1) return true;
	}
	if (lineType0 == TOP_RIGHT && lineType1 == TOP_LEFT) {
		if (x0 > x1 || y0 < y1) return true;
	}
	if (lineType0 == BOTTOM_RIGHT && lineType1 == TOP_RIGHT) {
		if (x0 > x1 || y0 > y1) return true;
	}
	if (lineType0 == BOTTOM_LEFT && lineType1 == BOTTOM_RIGHT) {
		if (x0 < x1 || y0 > y1) return true;
	}
	if (lineType0 == TOP_LEFT && lineType1 == BOTTOM_LEFT) {
		if (x0 < x1 || y0 < y1) return true;
	}
	return false;
}

bool Obstacles::findFarthestAvailablePathVertex(Vector2 *path, int pathSize, Vector3 start, Vector3 *next) const {
	if (pathSize == 0) {
		*next = start;
		return false;
	}

	int vertexTypeStart = -1;
	int vertexTypeStartPrev = -1;
	int polygonIndexStart = -1;
	int vertexIndexStart = -1;
	bool startOnPolygon = findPolygonVerticeByXZWithinTolerance(start.x, start.z, &polygonIndexStart, &vertexIndexStart, 0);
	if (startOnPolygon) {
		int vertexIndexStartPrev = (vertexIndexStart - 1 + _polygons[polygonIndexStart].verticeCount) % _polygons[polygonIndexStart].verticeCount;

		vertexTypeStart     = _polygons[polygonIndexStart].vertexType[vertexIndexStart];
		vertexTypeStartPrev = _polygons[polygonIndexStart].vertexType[vertexIndexStartPrev];
	}

	signed int farthestPathIndex = -1;
	for (int pathVertexIdx = 0; pathVertexIdx < pathSize; ++pathVertexIdx) {
		bool foundVertexNeighbor = false;
		int polygonIndexPath = -1;
		int vertexIndexPath = -1;
		bool pathVertexOnPolygon = findPolygonVerticeByXZWithinTolerance(path[pathVertexIdx].x, path[pathVertexIdx].y, &polygonIndexPath, &vertexIndexPath, 0) == 1;

		//start and current path vertices are on same polygon and are next to each other
		if (pathVertexOnPolygon && polygonIndexStart == polygonIndexPath) {
			int vertexIndexStartPrev = (vertexIndexStart - 1 + _polygons[polygonIndexPath].verticeCount) % _polygons[polygonIndexPath].verticeCount;
			int vertexIndexStartNext = (vertexIndexStart + 1                                           ) % _polygons[polygonIndexPath].verticeCount;

			if (vertexIndexPath == vertexIndexStartNext || vertexIndexPath == vertexIndexStartPrev || vertexIndexPath == vertexIndexStart) {
				foundVertexNeighbor = true;
			}
		}

		// neighboring vertices are always available
		if (foundVertexNeighbor) {
			farthestPathIndex = pathVertexIdx;
			continue;
		}

		bool pathVertexAvailable = true;
		for (int currentPolygonIdx = 0; currentPolygonIdx < kPolygonCount && pathVertexAvailable; ++currentPolygonIdx) {
			Polygon *polygon = &_polygons[currentPolygonIdx];

			if (!polygon->isPresent || polygon->verticeCount == 0) {
				continue;
			}

			for (int polygonVertexIdx = 0; polygonVertexIdx < polygon->verticeCount && pathVertexAvailable; ++polygonVertexIdx) {
				int polygonVertexNextIdx = (polygonVertexIdx + 1) % polygon->verticeCount;

				// check intersection between start -> path and polygon edge
				Vector2 intersection;
				if (!lineIntersection(Vector2(start.x, start.z), path[pathVertexIdx], polygon->vertices[polygonVertexIdx], polygon->vertices[polygonVertexNextIdx], &intersection)) {
					continue;
				}

				// intersection has to be at end of one of these points (either on this polygon or on the path or at start)
				if (!(
					(WITHIN_TOLERANCE(intersection.x, start.x)                                   && WITHIN_TOLERANCE(intersection.y, start.z)                                  )
				 || (WITHIN_TOLERANCE(intersection.x, path[pathVertexIdx].x)                     && WITHIN_TOLERANCE(intersection.y, path[pathVertexIdx].y)                    )
				 || (WITHIN_TOLERANCE(intersection.x, polygon->vertices[polygonVertexIdx].x)     && WITHIN_TOLERANCE(intersection.y, polygon->vertices[polygonVertexIdx].y)    )
				 || (WITHIN_TOLERANCE(intersection.x, polygon->vertices[polygonVertexNextIdx].x) && WITHIN_TOLERANCE(intersection.y, polygon->vertices[polygonVertexNextIdx].y))
				)) {
					pathVertexAvailable = false;
					break;
				}

				int polygonIndexIntersection = -1;
				int vertexIndexIntersection = -1;
				if (findPolygonVerticeByXZWithinTolerance(intersection.x, intersection.y, &polygonIndexIntersection, &vertexIndexIntersection, currentPolygonIdx)) {
					// Intersection has to be vertex only on current polygon
					// Part of pathfinding fix 2 (dealing with merge on only one edge point)
					//			but also speeds up process:
					// 				we start (a cyclical) searching in Polygons array
					//				beginning from the current polygon index
					assert(polygonIndexIntersection == currentPolygonIdx);

					if (verticesCanIntersect(vertexTypeStartPrev, vertexTypeStart, start.x, start.z, path[pathVertexIdx].x, path[pathVertexIdx].y)) {
						pathVertexAvailable = false;
						break;
					}

					if ((currentPolygonIdx == polygonIndexPath  && vertexIndexIntersection == vertexIndexPath)
					|| (currentPolygonIdx == polygonIndexStart && vertexIndexIntersection == vertexIndexStart)
					) {
						continue;
					}

					int vertexIndexIntersectionprev = (vertexIndexIntersection - 1 + _polygons[polygonIndexIntersection].verticeCount ) % _polygons[polygonIndexIntersection].verticeCount;
					if (verticesCanIntersect(_polygons[polygonIndexIntersection].vertexType[vertexIndexIntersectionprev], _polygons[polygonIndexIntersection].vertexType[vertexIndexIntersection], intersection.x, intersection.y, path[pathVertexIdx].x, path[pathVertexIdx].y)) {
						pathVertexAvailable = false;
						break;
					}
				} else {
					bool startIntersectionWithinTolerance = false;
					if (WITHIN_TOLERANCE(intersection.x, start.x)
					 && WITHIN_TOLERANCE(intersection.y, start.z)
					) {
						startIntersectionWithinTolerance = true;
					}

					if (currentPolygonIdx == polygonIndexStart || startIntersectionWithinTolerance) {
						if (polygonIndexStart >= 0 || !startIntersectionWithinTolerance) {
							pathVertexAvailable = false;
							break;
						}

						int polygonVertexType =  polygon->vertexType[polygonVertexIdx];
						if ((polygonVertexType == TOP_LEFT     && intersection.y < path[pathVertexIdx].y)
						|| (polygonVertexType == TOP_RIGHT    && intersection.x > path[pathVertexIdx].x)
						|| (polygonVertexType == BOTTOM_RIGHT && intersection.y > path[pathVertexIdx].y)
						|| (polygonVertexType == BOTTOM_LEFT  && intersection.x < path[pathVertexIdx].x)
						) {
							pathVertexAvailable = false;
							break;
						}
					}
				}
			}
		}

		if (pathVertexAvailable) {
			farthestPathIndex = pathVertexIdx;
		}
	}

	if (farthestPathIndex == -1) {
		*next = start;
		return false;
	}

	next->x = path[farthestPathIndex].x;
	next->z = path[farthestPathIndex].y;

	bool walkboxFound;
	float walkboxAltitude = _vm->_scene->_set->getAltitudeAtXZ(next->x, next->z, &walkboxFound);

	if (walkboxFound) {
		next->y = walkboxAltitude;
		return true;
	} else {
		next->y = start.y;
		return false;
	}
}

void Obstacles::backup() {
	for (int i = 0; i != kPolygonCount; ++i) {
		_polygonsBackup[i].isPresent = false;
	}

	int count = 0;
	for (int i = 0; i != kPolygonCount; ++i) {
		if (_polygons[i].isPresent) {
			_polygonsBackup[count] = _polygons[i];
			++count;
		}
	}

	for (int i = 0; i != kPolygonCount; ++i) {
		_polygons[i] = _polygonsBackup[count];
	}

	_count = count;
	_backup = true;
}

void Obstacles::restore() {
	for (int i = 0; i != kPolygonCount; ++i) {
		_polygons[i].isPresent = false;
	}
	for (int i = 0; i != kPolygonCount; ++i) {
		_polygons[i] = _polygonsBackup[i];
	}
}

void Obstacles::save(SaveFileWriteStream &f) {
	f.writeBool(_backup);
	f.writeInt(_count);
	for (int i = 0; i < _count; ++i) {
		Polygon &p = _polygonsBackup[i];
		f.writeBool(p.isPresent);
		f.writeInt(p.verticeCount);
		f.writeFloat(p.rect.x0);
		f.writeFloat(p.rect.y0);
		f.writeFloat(p.rect.x1);
		f.writeFloat(p.rect.y1);
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			f.writeVector2(p.vertices[j]);
		}
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			f.writeInt(p.vertexType[j]);
		}
	}
	for (int i = 0; i < kVertexCount; ++i) {
		f.writeVector2(_path[i]);
	}
	f.writeInt(_pathSize);
}

void Obstacles::load(SaveFileReadStream &f) {
	for (int i = 0; i < kPolygonCount; ++i) {
		_polygons[i].isPresent = false;
		_polygons[i].verticeCount = 0;
		_polygonsBackup[i].isPresent = false;
		_polygonsBackup[i].verticeCount = 0;
	}

	_backup = f.readBool();
	_count = f.readInt();
	for (int i = 0; i < _count; ++i) {
		Polygon &p = _polygonsBackup[i];
		p.isPresent = f.readBool();
		p.verticeCount = f.readInt();
		p.rect.x0 = f.readFloat();
		p.rect.y0 = f.readFloat();
		p.rect.x1 = f.readFloat();
		p.rect.y1 = f.readFloat();
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			p.vertices[j] = f.readVector2();
		}
		for (int j = 0; j < kPolygonVertexCount; ++j) {
			p.vertexType[j] = (VertexType)f.readInt();
		}
	}

	for (int i = 0; i < kPolygonCount; ++i) {
		_polygons[i] = _polygonsBackup[i];
	}

	for (int i = 0; i < kVertexCount; ++i) {
		_path[i] = f.readVector2();
	}
	_pathSize = f.readInt();
}

void Obstacles::draw() {
	float y = _vm->_playerActor->getY();

	for (int i = 0; i != kPolygonCount; ++i) {
		if (!_polygons[i].isPresent) {
			continue;
		}

		Vector3 p0 = _vm->_view->calculateScreenPosition(Vector3(
			_polygons[i].vertices[_polygons[i].verticeCount - 1].x,
			y,
			_polygons[i].vertices[_polygons[i].verticeCount - 1].y
		));

		for (int j = 0; j != _polygons[i].verticeCount; ++j) {
			Vector3 p1 = _vm->_view->calculateScreenPosition(Vector3(
				_polygons[i].vertices[j].x,
				y,
				_polygons[i].vertices[j].y
			));

			_vm->_surfaceFront.drawLine(p0.x, p0.y, p1.x, p1.y, _vm->_surfaceFront.format.RGBToColor(255, 255, 255));

			p0 = p1;
		}
	}

	// draw actor's box
	{
		Vector3 playerPos = _vm->_playerActor->getXYZ();
		Vector3 p0 = _vm->_view->calculateScreenPosition(playerPos + Vector3(-12.0f, 0.0f, -12.0f));
		Vector3 p1 = _vm->_view->calculateScreenPosition(playerPos + Vector3( 12.0f, 0.0f, -12.0f));
		Vector3 p2 = _vm->_view->calculateScreenPosition(playerPos + Vector3( 12.0f, 0.0f,  12.0f));
		Vector3 p3 = _vm->_view->calculateScreenPosition(playerPos + Vector3(-12.0f, 0.0f,  12.0f));

		_vm->_surfaceFront.drawLine(p0.x, p0.y, p1.x, p1.y, _vm->_surfaceFront.format.RGBToColor(255, 0, 0));
		_vm->_surfaceFront.drawLine(p1.x, p1.y, p2.x, p2.y, _vm->_surfaceFront.format.RGBToColor(255, 0, 0));
		_vm->_surfaceFront.drawLine(p2.x, p2.y, p3.x, p3.y, _vm->_surfaceFront.format.RGBToColor(255, 0, 0));
		_vm->_surfaceFront.drawLine(p3.x, p3.y, p0.x, p0.y, _vm->_surfaceFront.format.RGBToColor(255, 0, 0));
	}

	// draw path along polygons
	for (int i = 1; i < _pathSize; ++i) {
		Vector3 p0 = _vm->_view->calculateScreenPosition(Vector3(_path[i - 1].x, y, _path[i - 1].y));
		Vector3 p1 = _vm->_view->calculateScreenPosition(Vector3(_path[i].x, y, _path[i].y));
		_vm->_surfaceFront.drawLine(p0.x, p0.y, p1.x, p1.y, _vm->_surfaceFront.format.RGBToColor(255, 0, 0));
	}

	// draw "next" vertex
	{
		//TODO
	}


}

} // End of namespace BladeRunner
