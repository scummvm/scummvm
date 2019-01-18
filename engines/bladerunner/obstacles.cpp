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

#define DISABLE_PATHFINDING 1

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
	for (int i = 0; i < kPolygonCount; i++) {
		_polygons[i].isPresent = false;
		_polygons[i].verticeCount = 0;
		for (int j = 0; j < kPolygonVertexCount; j++) {
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

bool Obstacles::linePolygonIntersection(LineSegment lineA, VertexType lineAType, Polygon *polyB, Vector2 *intersectionPoint, int *intersectionIndex) {
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
				if (!WITHIN_TOLERANCE(lineB.end.x, intersectionPoint->x)
				 || !WITHIN_TOLERANCE(lineB.end.y, intersectionPoint->y)) {
					if (newIntersectionPoint != *intersectionPoint) {
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

	Polygon *startingPolygon = polyPrimary;
	int flagDone = false;
	while (!flagDone) {
		VertexType polyPrimaryType;

		polyLine.start  = flagDidFindIntersection ? intersectionPoint : polyPrimary->vertices[vertIndex];
		polyLine.end    = polyPrimary->vertices[(vertIndex + 1) % polyPrimary->verticeCount];

		// TODO(madmoose): How does this work when adding a new intersection point?
		polyPrimaryType = polyPrimary->vertexType[vertIndex];

		if (flagAddVertexToVertexList) {
			assert(polyMerged.verticeCount < kPolygonVertexCount);
			polyMerged.vertices[polyMerged.verticeCount] = polyLine.start;
			polyMerged.vertexType[polyMerged.verticeCount] = polyPrimaryType;
			polyMerged.verticeCount++;
		}

		flagAddVertexToVertexList = true;
		int polySecondaryIntersectionIndex = -1;

		if (linePolygonIntersection(polyLine, polyPrimaryType, polySecondary, &intersectionPoint, &polySecondaryIntersectionIndex)) {
			if (WITHIN_TOLERANCE(intersectionPoint.x, polyLine.start.x) && WITHIN_TOLERANCE(intersectionPoint.y, polyLine.start.y)) {
				flagAddVertexToVertexList = false;
				polyMerged.verticeCount--; // TODO(madmoose): How would this work?
			} else {
				// Obstacles::nop
			}
			vertIndex = polySecondaryIntersectionIndex;
			flagDidFindIntersection = true;

			SWAP(polyPrimary, polySecondary);

			flagDidMergePolygons = true;
		} else {
			vertIndex = (vertIndex + 1) % polyPrimary->verticeCount;
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

		for (int j = i+1; j < kPolygonCount; ++j) {
			Polygon &polyB = _polygons[j];
			if (!polyB.isPresent) {
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
	for (int i = 0; i < kPolygonCount; i++) {
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
	int     polyNearVertIndex;
	float   polyNearDist = 0.0f;
	Vector2 polyNearPos;
	int     polyFarVertIndex;
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

	// TODO(tmf)
	// postProcessPath(_path, _pathSize, &next);

	*next = Vector3(_path[0].x, from.y, _path[0].y);
	return true;
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

		for (int j = 0; j != kPolygonVertexCount; ++j) {
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

bool Obstacles::findPolygonVerticeByXZWithinTolerance(float x, float z, int *polygonIndex, int *verticeIndex) const {
	*polygonIndex = -1;
	*verticeIndex = -1;

	for (int i = 0; i != kPolygonCount; ++i) {
		if (!_polygons[i].isPresent || _polygons[i].verticeCount == 0) {
			continue;
		}

		for (int j = 0; j != kPolygonVertexCount; ++j) {
			if (WITHIN_TOLERANCE(_polygons[i].vertices[j].x, x)) {
				if (WITHIN_TOLERANCE(_polygons[i].vertices[j].y, z)) {
					*polygonIndex = i;
					*verticeIndex = j;
					return true;
				}
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

#define DEC_WRAP(x) (((x) + poly->verticeCount - 1) % poly->verticeCount)

	/* Add polygon vertices in negative iteration order */
	for (int i = vertStartIndex; i != vertEndIndex; i = DEC_WRAP(i)) {
		Vector2 v = poly->vertices[i];
		if (_vm->_scene->_set->findWalkbox(v.x, v.y) == -1) {
			*pathBlocked = true;
		}

		assert(pathSize < pathCapacity);
		path[pathSize++] = v;
	}

#undef DEC_WRAP

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

#define INC_WRAP(x) (((x) + 1) % poly->verticeCount)

	/* Add polygon vertices in positive iteration order */
	for (int i = INC_WRAP(vertStartIndex); i != vertEndIndex; i = INC_WRAP(i)) {
		Vector2 v = poly->vertices[i];
		if (_vm->_scene->_set->findWalkbox(v.x, v.y) == -1) {
			*pathBlocked = true;
		}

		assert(pathSize < pathCapacity);
		path[pathSize++] = v;
	}

#undef INC_WRAP

	/* Add end position to path */
	if (_vm->_scene->_set->findWalkbox(endPos.x, endPos.y) == -1) {
		*pathBlocked = true;
	}
	assert(pathSize < pathCapacity);
	path[pathSize++] = endPos;

	return pathSize;
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
	float floor = _vm->_playerActor->getY();
	for (int i = 0; i != kPolygonCount; ++i) {
		if (!_polygons[i].isPresent) {
			continue;
		}

		Vector3 p0 = _vm->_view->calculateScreenPosition(Vector3(
			_polygons[i].vertices[_polygons[i].verticeCount - 1].x,
			floor,
			_polygons[i].vertices[_polygons[i].verticeCount - 1].y
		));

		for (int j = 0; j != _polygons[i].verticeCount; ++j) {
			Vector3 p1 = _vm->_view->calculateScreenPosition(Vector3(
				_polygons[i].vertices[j].x,
				floor,
				_polygons[i].vertices[j].y
			));

			_vm->_surfaceFront.drawLine(p0.x, p0.y, p1.x, p1.y, 0x7FE0);

			p0 = p1;
		}
	}
}

} // End of namespace BladeRunner
