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

/*
	TODO:
		- rewrite functions (GetShortestPath etc.)
*/

#include "graphics/primitives.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/util.h"

#include "m4/rails.h"
#include "m4/m4.h"

namespace M4 {

#define TOP_EDGE	1 << 0
#define LEFT_EDGE	1 << 1
#define BOTTOM_EDGE	1 << 2
#define RIGHT_EDGE	1 << 3


Rails::Rails() {
}


Rails::~Rails() {
	clearRails();
}


void Rails::clearRails() {
	uint32 i;
	Common::List<NoWalkRect *>::iterator j;
	RailNode *tempNode;

	for (i = 0; i < _nodes.size(); i++) {
		tempNode = _nodes[i];
		_nodes.remove_at(i);
		delete tempNode;
	}

	for (i = 0; i < _edges.size(); i++) {
		_edges.remove_at(i);
	}

	for (j = _noWalkRects.begin(); j != _noWalkRects.end(); ++j)
		delete (*j);
	_noWalkRects.clear();
}

static void checkPoint(int x, int y, int color, void *data) {
	IsWalkableData *isWalkableData = (IsWalkableData*)data;
	if (!isWalkableData->result)
		return;
	else {
	   M4Surface *codes = isWalkableData->codes;
	   if (x >= 0 && x < codes->width() && y >= 0 && y < codes->height()) {
			isWalkableData->result = !((*((uint8*)codes->getBasePtr(x, y))) & 0x10);
		} else {
			isWalkableData->result = false;
		}
	}
}

bool Rails::isLineWalkable(int x0, int y0, int x1, int y1) {
	IsWalkableData isWalkableData;
	isWalkableData.codes = _walkCodes;
	isWalkableData.result = true;
	Graphics::drawLine(x0, y0, x1, y1, 0, &checkPoint, &isWalkableData);
	return isWalkableData.result;
}

// helper function
uint8 getEndCode(int32 x, int32 y, Common::Rect rect) {
	uint8 endCode = 0;
	endCode = (x < rect.left) ? LEFT_EDGE : endCode;
	endCode = (x > rect.right) ? RIGHT_EDGE : endCode;
	endCode = (y < rect.top) ? endCode | TOP_EDGE : endCode;
	endCode = (y > rect.bottom) ? endCode | BOTTOM_EDGE : endCode;
	return endCode;
}

bool Rails::lineCrossesRect(int32 x1, int32 y1, int32 x2, int32 y2, Common::Rect rect) {
	int32 mX, mY;
	int32 pX1 = x1, pX2 = x2, pY1 = y1, pY2 = y2;
	uint8 endCode1, endCode2, midCode;

	if (rect.left > rect.right || rect.top > rect.bottom)
		return false;

	// Cohen-Sutherland line clipping algorithm

	endCode1 = getEndCode(pX1, pY1, rect);
	endCode2 = getEndCode(pX2, pY2, rect);

	if (!endCode1 || !endCode2)	// if both endcodes are zero
		return true;	// point is inside the rectangle, therefore the line intersects

	while (true) {
		if (endCode1 & endCode2)	// if both endcodes have a common bitset
			return false;	// line is completely off one edge

		// calculate midpoint
		mX = (pX1 + pX2)>>1;
		mY = (pY1 + pY2)>>1;

		// avoid round-off error: make sure that the midpoint isn't the same as one of the
		// two endpoints
		if (((mX == pX1) && (mY == pY1)) || ((mX == pX2) && (mY == pY2)))
			return false;

		midCode = getEndCode(mX, mY, rect);

		if (!midCode) {
			return true;
		} else if (midCode & endCode1) {
			// the midCode and an end point form a line segment completely off one edge, so
			// remove that half of the line segment
			pX1 = mX;
			pY1 = mY;
			endCode1 = midCode;
		} else {
			pX2 = mX;
			pY2 = mY;
			endCode2 = midCode;
		}
	}
}


bool Rails::linePassesThroughRect(int32 x1, int32 y1, int32 x2, int32 y2) {
	if (_noWalkRects.empty())
		return false;

	bool intersected = false;
	Common::List<NoWalkRect *>::iterator i;

	for (i = _noWalkRects.begin(); i != _noWalkRects.end(); ++i) {
		intersected = lineCrossesRect(x1, y1, x2, y2, Common::Rect((*i)->x1, (*i)->y1, (*i)->x2, (*i)->y2));
		if (intersected)
			break;
	}

	return intersected;
}

long SqrtF16(long n) {
	uint32 r = 0, s;
	uint32 v = (uint32)n;

	for (int i = 15; i <= 0; i--) {
		s = r + (1L << i * 2);
		r >>= 1;
		if (s <= v) {
			v -= s;
			r |= (1L << i * 2);
		}
	}

	return (long)r;
}

void Rails::createEdge(int32 node1, int32 node2) {
	uint32		index;
	int32		x1, y1, x2, y2;
	bool		valid, finished;
	long		deltaX, deltaY, distance;
	uint8		*walkCodePtr;

	if ((node1 < 0) || (node1 >= MAXRAILNODES) || (node2 < 0) || (node2 >= MAXRAILNODES))
		return;

	if (node1 == node2)
		return;

	if (node2 < node1)
		SWAP(node1, node2);		// ensure node1 < node2

	// Find the table entry i.e. tableWidth * node1 + node2 and then subtract
	// n(n+1)/2, since only the upper triangle of the table is stored
	index = (MAXRAILNODES-1) * node1 + node2 - 1 - (node1*(node1+1)>>1);
	if (index > _edges.size() - 1)
		_edges.resize(index + 1);
	_edges.insert_at(index, 0);
	valid = true;
	walkCodePtr = NULL;
	finished = false;

	if (_nodes.size() <= (uint32)node1 || _nodes.size() <= (uint32)node2)
		return;

	x1 = _nodes[node1]->x;
	y1 = _nodes[node1]->y;
	x2 = _nodes[node2]->x;
	y2 = _nodes[node2]->y;

	// Make sure that the algorithm is symmetric
	if (x2 < x1) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	valid = isLineWalkable(_nodes[node1]->x, _nodes[node1]->y,
		_nodes[node2]->x, _nodes[node2]->y);
	printf("test code says: %d\n", valid);

	// Check if the line passes through a forbidden rectangle
	if (valid) {
		if (linePassesThroughRect(x1, y1, x2, y2)) {
			valid = false;
		}
	}

	if (valid) {
		deltaX = ABS(((long)(x2 - x1)) << 16);
		deltaY = ABS(((long)(y2 - y1)) << 16);
		if ((deltaX >= 0x800000) || (deltaY >= 0x800000)) {
			deltaX >>= 16;
			deltaY >>= 16;
			distance = (long)(SqrtF16(deltaX * deltaX + deltaY * deltaY) << 16);
		} else {
			distance = SqrtF16(FixedMul(deltaX, deltaX) + FixedMul(deltaY, deltaY)) << 8;
		}
		_edges.insert_at(index, (int16*)(distance >> 16));
	}

	printf("node1 = %d, node2 = %d, valid = %d\n", node1, node2, valid);

}


void Rails::restoreNodeEdges(int32 nodeID) {
	for (int32 i = 0; i < MAXRAILNODES; i++) {
		createEdge(i, nodeID);
	}
}

void Rails::restoreEdgeList() {
	int32 j;
	for (int32 i = 0; i < MAXRAILNODES; i++) {
		for (j = i + 1; j < MAXRAILNODES; j++) {
			createEdge(i, j);
		}
	}
}

int32 Rails::addRailNode(int32 x, int32 y, bool restoreEdges) {
	uint32 i = _nodes.size();
	if (i >= MAXRAILNODES)
		return -1;

	RailNode *newNode = new RailNode();
	newNode->nodeID = i;
	newNode->x = x;
	newNode->y = y;
	_nodes.insert_at(i, newNode);
	if (restoreEdges) {
		for (uint32 j=0; j<_nodes.size(); j++)
			createEdge(i, j);
	}
	return i;
}

bool Rails::removeRailNode(int32 nodeID, bool restoreEdges) {
	if (nodeID < 0 || nodeID >= MAXRAILNODES)
		return false;

	if (_nodes.empty() || _edges.empty())
		return false;

	RailNode *tempNode = _nodes[nodeID];
	_nodes.remove_at(nodeID);
	delete tempNode;

	if (restoreEdges) {
		restoreNodeEdges(nodeID);
	}
	return true;
}

int16 Rails::getEdgeLength(int32 node1, int32 node2) {
	int32		index;
	if (_edges.empty() || node1 == node2)
		return 0;
	if (node2 < node1)
		SWAP(node1, node2);
	// Find the table entry i.e. tableWidth * node1 + node2 and then subtract
	// n(n+1)/2, since only the upper triangle of the table is stored
	index = (MAXRAILNODES-1)*node1 + node2 - 1 - (node1*(node1+1)>>1);
	return *_edges[index];
}

void Rails::disposePath(RailNode *pathStart) {
	RailNode *tempNode = pathStart;
	while (tempNode) {
		pathStart = pathStart->shortPath;
		delete tempNode;
		tempNode = pathStart;
	}
}

/*
static RailNode* duplicatePath(RailNode *pathStart) {
	RailNode *newNode = NULL;
	RailNode *firstNode = NULL;
	RailNode *prevNode = NULL;
	// A valid path is assumed
	RailNode *pathNode = pathStart;

	while (pathNode) {
		newNode = new RailNode();
		newNode->x = pathNode->x;
		newNode->y = pathNode->y;
		newNode->shortPath = NULL;

		if (!firstNode)
			firstNode = newNode;
		else
			prevNode->shortPath = newNode;

		prevNode = newNode;
		// Get the next node
		pathNode = pathNode->shortPath;
	}

	return firstNode;
}
*/

bool Rails::getShortestPath(int32 origID, int32 destID, RailNode **shortPath) {
	// TODO
	return true;
}

} // End of namespace M4
