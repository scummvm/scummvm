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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef PRISONER_PATH_H
#define PRISONER_PATH_H

#include "prisoner/prisoner.h"
#include "prisoner/objectstorage.h"

namespace Prisoner {

const int16 kMaxPathNodes = 50;
const int16 kMaxPathNodeConnections = 30;
const int16 kMaxPathEdges = 450;
const int16 kMaxPathPolygons = 50;

struct PathNodeConnection {
	byte used;
	int16 nodeIndex;
	int16 edgeIndex;
	uint16 distance;
	byte enabled;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
};

struct PathNode {
	byte used;
	byte unk1;
	int16 x, y, scale;
	uint16 connectionsCount;
	ObjectStorage<PathNodeConnection, kMaxPathNodeConnections> connections;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
};

struct PathEdge {
	byte used;
	int16 enabled;
	int16 unk2;
	int16 polyCount; // number of polygons this edge is connected to
	int16 nodeIndex1, nodeIndex2;
	uint16 distance;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	bool hasNode(int16 nodeIndex) const {
		return nodeIndex1 == nodeIndex || nodeIndex2 == nodeIndex;
	}
};

struct PathPolygon {
	byte used;
	byte enabled;
	int16 nodeIndex1;
	int16 nodeIndex2;
	int16 nodeIndex3;
	int16 edgeIndex1;
	int16 edgeIndex2;
	int16 edgeIndex3;
	bool isEmpty() const { return used == 0; }
	void clear() { used = 0; }
	bool hasEdge(int16 edgeIndex) const {
		return edgeIndex1 == edgeIndex || edgeIndex2 == edgeIndex || edgeIndex3 == edgeIndex;
	}
	bool hasNode(int16 nodeIndex) const {
		return nodeIndex1 == nodeIndex || nodeIndex2 == nodeIndex || nodeIndex3 == nodeIndex;
	}
};

struct PathHelper {
	int16 edgeIndex;
	int16 nodeIndex;
	int16 x, y;
	uint16 distance0;
	uint16 distance1;
	uint16 distance2;
};

struct ComparePathHelperByDistance0 {
	bool operator()(const BackgroundObject *left, const BackgroundObject *right) const {
		return left->y < right->y;
	}
};

struct PathNodeHelper {
	int16 outerIndex;
	byte unk2;
	uint16 distance;
	int16 nodeIndex;
};

/* PathSystem */

class PathSystem {
public:
	PathSystem(PrisonerEngine *vm);
	~PathSystem();
protected:
	PrisonerEngine *_vm;
	bool _pathSystemBuilt;
	ObjectStorage<PathNode, kMaxPathNodes> _pathNodes;
	int16 _pathNodesCount;
	ObjectStorage<PathEdge, kMaxPathEdges> _pathEdges;
	int16 _pathEdgesCount;
	ObjectStorage<PathPolygon, kMaxPathPolygons> _pathPolygons;
	int16 _pathPolygonsCount;
	PathHelper _pathHelpers[450];
	int16 _pathHelpersCount;
	PathNodeHelper _pathNodeHelpers[kMaxPathNodes];
	int16 _pathNodeHelpersCount;
	PathResult _pathResults[452];
	int16 _pathResultsCount;
	PathResult *_pathCurrResult;
public:
	void addPathNode(int16 nodeIndex, int16 x, int16 y, int16 scale);
	void addPathEdge(int16 edgeIndex, int16 nodeIndex1, int16 nodeIndex2, int16 enabled);
	bool addPathPolygon(int16 polyIndex, int16 nodeIndex1, int16 nodeIndex2, int16 nodeIndex3, int16 flag);
	uint16 calcPathNodesDistance(int16 nodeIndex1, int16 nodeIndex2);
	uint16 calcDistance(int16 x1, int16 y1, int16 x2, int16 y2);
	uint16 adjustToClosestPointOnLine(int16 &x, int16 &y, int16 nodeIndex1, int16 nodeIndex2);
	int16 findPathPolygonAtPos(int16 x, int16 y);
	int16 findPathNodeAtPos(int16 x, int16 y, bool fuzzy);
	void findClosestPointOnEdge(int16 x, int16 y, bool flag1, bool flag2, int16 nodeIndex);
	bool path252C1(int16 x1, int16 y1, int16 &x2, int16 &y2, int16 x3, int16 y3);
	int16 findPathConnectionEdge(int16 nodeIndex1, int16 nodeIndex2);
	bool path25964(int16 nodeIndex1, int16 nodeIndex2, int16 value, int16 edgeIndex1, int16 edgeIndex2);
	bool path24D5B(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3, int16 x4, int16 y4);
	int16 findPathPolygonByEdges(int16 edgeIndex1, int16 edgeIndex2);
	int16 findPathPolygonByNodes(int16 nodeIndex1, int16 nodeIndex2);
	int16 findPathPolygonByNodeAndEdge(int16 nodeIndex, int16 edgeIndex);
	void path25669(int16 nodeIndex1, uint16 distance, int16 nodeIndex2, int16 nodeIndex3);
	void clearPathNodeHelpers();
 	void path2579A(int16 nodeIndex);
 	bool insertPathEdge(int16 nodeIndex1, int16 nodeIndex2);
 	void cleanupPathResult();
 	void deletePathResultItem(int16 resultIndex);
	int16 calcPathEdgeScale(int16 y, int16 edgeIndex);
 	int16 calcPathPolygonScale(int16 y, int16 polygonIndex);
	bool path260BA(int16 sourceNodeIndex, int16 edgeIndex, int16 sourcePolyIndex, int16 destX, int16 destY);
	void removePathNode(int16 nodeIndex);
	void removePathEdgeByPathNodeIndex(int16 nodeIndex);
	void removePathEdge(int16 edgeIndex);
	void unlinkPathNodeConnection(int16 nodeIndex1, int16 nodeIndex2);
	bool findPath(int16 sourceX, int16 sourceY, int16 destX, int16 destY);
	void clearPathWalker();
	void buildPathSystem();
	void clearPathSystem();
	void setPathEdgeOrPolygonEnabled(int16 type, int16 index, int16 enabled);
	void setPathEdgeEnabled(int16 edgeIndex, int16 enabled);
	void togglePathSystem(int16 flag);

	PathNode *getNode(int16 nodeIndex) { return &_pathNodes[nodeIndex]; }
	PathEdge *getEdge(int16 edgeIndex) { return &_pathEdges[edgeIndex]; }
	PathPolygon *getPolygon(int16 polyIndex) { return &_pathPolygons[polyIndex]; }
	PathResult *getPathResults() { return _pathResults; }
	int16 getPathResultsCount() { return _pathResultsCount; }
	bool getPathSystemBuilt() { return _pathSystemBuilt; }
	void setPathSystemBuilt(bool value) { _pathSystemBuilt = value; }

};

} // End of namespace Prisoner

#endif /* PRISONER_H */
