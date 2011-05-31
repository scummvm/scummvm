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
 */

#ifndef M4_RAILS_H
#define M4_RAILS_H

#include "m4/graphics.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

// TODO: This needs cleaning up

namespace M4 {

#define MAXRAILNODES 32
#define PATH_END	 0xffff

struct RailNode {
	uint8 nodeID;
	int32 x, y;
	RailNode *shortPath;
	int32 pathWeight;
};

struct NoWalkRect {
	int32 x1, y1, x2, y2;
	int32 alternateWalkToNode;
	int32 walkAroundNode1;
	int32 walkAroundNode2;
	int32 walkAroundNode3;
	int32 walkAroundNode4;
};

struct PathNode {
	PathNode *next;
	int8 nodeID;
};

struct IsWalkableData {
	M4Surface *codes;
	bool result;
};

class Rails {
public:
	Rails();
	~Rails();

	void setCodeSurface(M4Surface *surface) { _walkCodes = surface; }
	void clearRails();
	int32 addRailNode(int32 x, int32 y, bool restoreEdges);
	uint8 getDepth(const Common::Point &pt);

private:
	Common::Array<RailNode *> _nodes;
	Common::Array<int16> _edges;
	Common::List<NoWalkRect *> _noWalkRects;
	M4Surface *_walkCodes;

	bool lineCrossesRect(int32 x1, int32 y1, int32 x2, int32 y2, Common::Rect rect);
	bool linePassesThroughRect(int32 x1, int32 y1, int32 x2, int32 y2);
	void createEdge(int32 node1, int32 node2);
	bool removeRailNode(int32 nodeID, bool restoreEdges);
	int16 getEdgeLength(int32 node1, int32 node2);

	void restoreNodeEdges(int32 nodeID);
	void restoreEdgeList();
	void disposePath(RailNode *pathStart);
	bool getShortestPath(int32 origID, int32 destID, RailNode **shortPath);
	bool isLineWalkable(int x0, int y0, int x1, int y1);
};

long SqrtF16(long n);

} // End of namespace M4

#endif
