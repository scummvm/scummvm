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

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resource.h"

namespace Prisoner {

PathSystem::PathSystem(PrisonerEngine *vm) : _vm(vm) {
}

PathSystem::~PathSystem() {
}

void PathSystem::addPathNode(int16 nodeIndex, int16 x, int16 y, int16 scale) {
	PathNode *pathNode = &_pathNodes[nodeIndex];
	pathNode->used = 1;
	pathNode->unk1 = 0;
	pathNode->x = x;
	pathNode->y = y;
	pathNode->scale = scale;
	pathNode->connectionsCount = 0;
	pathNode->connections.clear();
}

void PathSystem::addPathEdge(int16 edgeIndex, int16 nodeIndex1, int16 nodeIndex2, int16 enabled) {

	debug(8, "addPathEdge(%d, %d, %d, %d)", edgeIndex, nodeIndex1, nodeIndex2, enabled);

	PathEdge *pathEdge = &_pathEdges[edgeIndex];
	PathNode *pathNode1 = &_pathNodes[nodeIndex1];
	PathNode *pathNode2 = &_pathNodes[nodeIndex2];
	PathNodeConnection *pathNodeConnection;
	int16 connectionIndex;

	pathEdge->used = 1;
	pathEdge->enabled = enabled;
	pathEdge->unk2 = 0;
	pathEdge->polyCount = 0;
	pathEdge->nodeIndex1 = nodeIndex1;
	pathEdge->nodeIndex2 = nodeIndex2;
	pathEdge->distance = calcPathNodesDistance(nodeIndex1, nodeIndex2);
	_pathEdgesCount++;

	connectionIndex = pathNode1->connections.getFreeSlot();
	pathNodeConnection = &pathNode1->connections[connectionIndex];
	pathNodeConnection->nodeIndex = nodeIndex2;
	pathNodeConnection->edgeIndex = edgeIndex;
	pathNodeConnection->used = 1;
	pathNodeConnection->distance = pathEdge->distance;
	pathNodeConnection->enabled = enabled;
	pathNode1->connectionsCount++;

	connectionIndex = pathNode2->connections.getFreeSlot();
	pathNodeConnection = &pathNode2->connections[connectionIndex];
	pathNodeConnection->nodeIndex = nodeIndex1;
	pathNodeConnection->edgeIndex = edgeIndex;
	pathNodeConnection->used = 1;
	pathNodeConnection->distance = pathEdge->distance;
	pathNodeConnection->enabled = enabled;
	pathNode2->connectionsCount++;

}

bool PathSystem::addPathPolygon(int16 polyIndex, int16 nodeIndex1, int16 nodeIndex2, int16 nodeIndex3, int16 flag) {

	PathPolygon *pathPolygon = &_pathPolygons[polyIndex];

	int16 edgeIndex1 = findPathConnectionEdge(nodeIndex1, nodeIndex2);
	if (edgeIndex1 == -1)
		return false;

	int16 edgeIndex2 = findPathConnectionEdge(nodeIndex2, nodeIndex3);
	if (edgeIndex2 == -1)
		return false;

	int16 edgeIndex3 = findPathConnectionEdge(nodeIndex3, nodeIndex1);
	if (edgeIndex3 == -1)
		return false;

	pathPolygon->used = 1;
	pathPolygon->enabled = flag;
	pathPolygon->nodeIndex1 = nodeIndex1;
	pathPolygon->nodeIndex2 = nodeIndex2;
	pathPolygon->nodeIndex3 = nodeIndex3;
	pathPolygon->edgeIndex1 = edgeIndex1;
	pathPolygon->edgeIndex2 = edgeIndex2;
	pathPolygon->edgeIndex3 = edgeIndex3;

	if (flag) {
		_pathEdges[edgeIndex1].polyCount++;
		_pathEdges[edgeIndex2].polyCount++;
		_pathEdges[edgeIndex3].polyCount++;
	}

	_pathPolygonsCount++;

	return true;

}

uint16 PathSystem::calcDistance(int16 x1, int16 y1, int16 x2, int16 y2) {
	uint16 deltaX = ABS(x1 - x2);
	uint16 deltaY = ABS(y1 - y2);
	uint16 result;
	if (deltaX < deltaY)
		result = deltaX / 2 + deltaY;
	else if (deltaX > deltaY)
		result = deltaX + deltaY / 2;
	else
		result = deltaX * 3 / 2;
	return result;
}

uint16 PathSystem::adjustToClosestPointOnLine(int16 &x, int16 &y, int16 nodeIndex1, int16 nodeIndex2) {

	/*
		Calculates the closest point from x/y to the line between nodeIndex1 and nodeIndex2
		and the distance between these points.
	*/

	int16 px1 = _pathNodes[nodeIndex1].x;
	int16 py1 = _pathNodes[nodeIndex1].y;
	int16 px2 = _pathNodes[nodeIndex2].x;
	int16 py2 = _pathNodes[nodeIndex2].y;
	bool xflag = false, yflag = false;
	int16 xv = 0, yv = 0, rx, ry;
	int16 xd = px2, yd = py2;

	while (1) {

		rx = px1 + xd;
		ry = py1 + yd;

		if (rx & 1) {
			if (xflag) {
				xflag = false;
				rx++;
			} else {
				xflag = true;
			}
		}

		if (ry & 1) {
			if (yflag) {
				yflag = false;
				ry++;
			} else {
				yflag = true;
			}
		}

		rx /= 2;
		ry /= 2;

		int16 d = (x - rx) * (px2 - rx) + (y - ry) * (py2 - ry);

		if (d == 0 || d == xv)
			break;

		xv = yv;
		yv = d;

		if (d > 0) {
			px1 = rx;
			py1 = ry;
		} else {
			xd = rx;
			yd = ry;
		}

	}

	uint16 distance = calcDistance(rx, ry, x, y);
	x = rx;
	y = ry;

	return distance;

}

uint16 PathSystem::calcPathNodesDistance(int16 nodeIndex1, int16 nodeIndex2) {
	return calcDistance(_pathNodes[nodeIndex1].x, _pathNodes[nodeIndex1].y,
		_pathNodes[nodeIndex2].x, _pathNodes[nodeIndex2].y);
}

int16 PathSystem::findPathPolygonAtPos(int16 x, int16 y) {
	int16 result = -1;
	for (int16 i = 0; i < _pathPolygons.count(); i++) {
		PathPolygon *pathPolygon = &_pathPolygons[i];
		if (pathPolygon->used == 1) {
			// Test if the given point is inside the current polygon
			int distance1, xdelta1, ydelta1;
			int distance2, xdelta2, ydelta2;
			int distance3, xdelta3, ydelta3;
			xdelta1 = _pathNodes[pathPolygon->nodeIndex1].x - x;
			ydelta1 = _pathNodes[pathPolygon->nodeIndex1].y - y;
			xdelta2 = _pathNodes[pathPolygon->nodeIndex2].x - x;
			ydelta2 = _pathNodes[pathPolygon->nodeIndex2].y - y;
			xdelta3 = _pathNodes[pathPolygon->nodeIndex3].x - x;
			ydelta3 = _pathNodes[pathPolygon->nodeIndex3].y - y;
			distance1 = ydelta2 * xdelta1 - xdelta2 * ydelta1;
			distance2 = ydelta3 * xdelta2 - xdelta3 * ydelta2;
			distance3 = ydelta1 * xdelta3 - xdelta1 * ydelta3;
			if ((distance1 < 0 && distance2 < 0 && distance3 < 0) ||
				(distance1 > 0 && distance2 > 0 && distance3 > 0)) {
				result = i;
				break;
			}
		}
	}
	return result;
}

int16 PathSystem::findPathNodeAtPos(int16 x, int16 y, bool fuzzy) {
	int16 result = -1;
	for (int16 nodeIndex = 0; nodeIndex < _pathNodes.count(); nodeIndex++) {
		PathNode *node = &_pathNodes[nodeIndex];
		if (node->used == 1) {
			if (fuzzy) {
				// Not used in POI, remove this and parameter if neccessary
			} else {
				if (node->x == x && node->y == y) {
					result = nodeIndex;
					break;
				}
			}
		}
	}
	return result;
}

int comparePathHelperByDistance0(const void * a, const void * b) {
	if (((const PathHelper*)a)->distance0 > ((const PathHelper*)b)->distance0)
		return 1;
	else if (((const PathHelper*)a)->distance0 < ((const PathHelper*)b)->distance0)
		return -1;
	else
		return 0;
}

void PathSystem::findClosestPointOnEdge(int16 x, int16 y, bool flag1, bool flag2, int16 nodeIndex) {

	PathHelper *pathHelper = &_pathHelpers[_pathHelpersCount];

	for (int16 edgeIndex = 0; edgeIndex < kMaxPathEdges; edgeIndex++) {
		PathEdge *edge = &_pathEdges[edgeIndex];

		if (edge->used == 1 && (edge->enabled == 1 || flag1 || flag2) && edge->unk2 == 0 &&
			edge->nodeIndex1 != nodeIndex && edge->nodeIndex2 != nodeIndex) {

			PathNode *node1 = &_pathNodes[edge->nodeIndex1];
			PathNode *node2 = &_pathNodes[edge->nodeIndex2];

			pathHelper->x = x;
			pathHelper->y = y;
			pathHelper->distance0 = adjustToClosestPointOnLine(pathHelper->x, pathHelper->y, edge->nodeIndex1, edge->nodeIndex2);
			pathHelper->distance1 = calcDistance(pathHelper->x, pathHelper->y, node1->x, node1->y);
			pathHelper->distance2 = calcDistance(pathHelper->x, pathHelper->y, node2->x, node2->y);

			if (pathHelper->distance1 > 0 && pathHelper->distance2 > 0 && (edge->enabled == 1 || flag2)) {
				pathHelper->edgeIndex = edgeIndex;
				pathHelper->nodeIndex = -1;
				path252C1(node1->x, node1->y, pathHelper->x, pathHelper->y, node2->x, node2->y);
				_pathHelpersCount++;
				pathHelper = &_pathHelpers[_pathHelpersCount];
			} else if (flag1) {
				pathHelper->edgeIndex = -1;
				if (pathHelper->distance1 == 0) {
					pathHelper->nodeIndex = edge->nodeIndex1;
					pathHelper->distance0 = calcDistance(x, y, node1->x, node1->y);
				} else {
					pathHelper->nodeIndex = edge->nodeIndex2;
					pathHelper->distance0 = calcDistance(x, y, node2->x, node2->y);
				}
				_pathHelpersCount++;
				pathHelper = &_pathHelpers[_pathHelpersCount];
			} else if (!flag2) {
				pathHelper->x = node1->x;
				pathHelper->y = node1->y;
				pathHelper->edgeIndex = -1;
				pathHelper->nodeIndex = edge->nodeIndex1;
				pathHelper->distance1 = 0;
				pathHelper->distance2 = edge->distance;
				pathHelper->distance0 = calcDistance(x, y, node1->x, node1->y);
				_pathHelpersCount++;
				pathHelper = &_pathHelpers[_pathHelpersCount];
				pathHelper->x = node2->x;
				pathHelper->y = node2->y;
				pathHelper->edgeIndex = -1;
				pathHelper->nodeIndex = edge->nodeIndex2;
				pathHelper->distance1 = edge->distance;
				pathHelper->distance2 = 0;
				pathHelper->distance0 = calcDistance(x, y, node2->x, node2->y);
				_pathHelpersCount++;
				pathHelper = &_pathHelpers[_pathHelpersCount];
			}

		}

	}

	qsort(_pathHelpers, _pathHelpersCount, sizeof(PathHelper), comparePathHelperByDistance0);

}

bool PathSystem::path252C1(int16 x1, int16 y1, int16 &x2, int16 &y2, int16 x3, int16 y3) {

	int16 deltaX = x3 - x1;
	int16 deltaY = y3 - y1;

	if (deltaX == 0 || deltaY == 0 || (x1 == x2 && y1 == y2) || (x3 == x2 && y3 == y2))
		return false;

	int16 deltaX2 = ABS(deltaX);
	int16 deltaY2 = ABS(deltaY);

	if (deltaX2 >= deltaY2) {
		y2 = y1 + (x2 - x1) * deltaY / deltaX;
	}

	if (deltaX2 <= deltaY2) {
		x2 = x1 + (y2 - y1) * deltaX / deltaY;
	}

	return true;
}

int16 PathSystem::findPathConnectionEdge(int16 nodeIndex1, int16 nodeIndex2) {
	int16 result = -1;
	PathNode *node = &_pathNodes[nodeIndex1];
	for (int16 i = 0; i < kMaxPathNodeConnections; i++) {
		if (node->connections[i].used == 1 && node->connections[i].nodeIndex == nodeIndex2) {
			result = node->connections[i].edgeIndex;
			break;
		}
	}
	return result;
}

bool PathSystem::path25964(int16 nodeIndex1, int16 nodeIndex2, int16 value, int16 edgeIndex1, int16 edgeIndex2) {

	int counter = 0;

	if (_pathEdgesCount >= _pathEdges.count() || nodeIndex1 == nodeIndex2 || nodeIndex1 == -1 || nodeIndex2 == -1)
		return false;

	if (findPathConnectionEdge(nodeIndex1, nodeIndex2) != -1)
		return true;

	for (int16 edgeIndex = 0; edgeIndex < _pathEdges.count(); edgeIndex++) {
		PathEdge *edge = &_pathEdges[edgeIndex];

		if (edge->used == 1 && edge->unk2 == 0 && edgeIndex != edgeIndex1 && edgeIndex != edgeIndex2) {

			if (edge->nodeIndex1 != nodeIndex1 && edge->nodeIndex2 != nodeIndex1 &&
				edge->nodeIndex1 != nodeIndex2 && edge->nodeIndex2 != nodeIndex2) {

				PathNode *node1 = &_pathNodes[nodeIndex1];
				PathNode *node2 = &_pathNodes[nodeIndex2];
				PathNode *node3 = &_pathNodes[edge->nodeIndex1];
				PathNode *node4 = &_pathNodes[edge->nodeIndex2];

				if (edge->polyCount < 2) {
					if (path24D5B(node1->x, node1->y, node2->x, node2->y, node3->x, node3->y, node4->x, node4->y))
						return false;
				} else if (edge->polyCount == 2) {
					if (path24D5B(node1->x, node1->y, node2->x, node2->y, node3->x, node3->y, node4->x, node4->y))
						counter++;
				}

			} else if ((nodeIndex1 == edge->nodeIndex1 && nodeIndex2 == edge->nodeIndex2) ||
				(nodeIndex2 == edge->nodeIndex1 && nodeIndex1 == edge->nodeIndex2))
				return true;

		}

	}

	if (counter >= value)
		return insertPathEdge(nodeIndex1, nodeIndex2);
	else
		return false;

}

bool PathSystem::path24D5B(int16 x1, int16 y1, int16 x2, int16 y2, int16 x3, int16 y3, int16 x4, int16 y4) {
	bool result = false;
	if ((y3 - y4) * (x1 - x2) != (y1 - y2) * (x3 - x4))	{
		int distance1 = (y3 - y4) * (x1 - x2) - (y1 - y2) * (x3 - x4);
		int distance2 = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4);
		int distance3 = (y1 - y3) * -(x1 - x2) + (x1 - x3) * (y1 - y2);
		if (distance1 < 0) {
			distance1 = -distance1;
			distance2 = -distance2;
			distance3 = -distance3;
		}
		result = (distance2 >= 0 && distance3 >= 0 && distance1 >= distance2 && distance1 >= distance3);
	}
	return result;
}

int16 PathSystem::findPathPolygonByEdges(int16 edgeIndex1, int16 edgeIndex2) {
	if (edgeIndex1 != -1 && edgeIndex2 != -1) {
		for (int16 i = 0; i < _pathPolygons.count(); i++) {
			PathPolygon *polygon = &_pathPolygons[i];
			if (polygon->used == 1 && polygon->enabled == 1 &&
				((edgeIndex1 == polygon->edgeIndex1 && edgeIndex2 == polygon->edgeIndex2) ||
				(edgeIndex1 == polygon->edgeIndex1 && edgeIndex2 == polygon->edgeIndex3) ||
				(edgeIndex1 == polygon->edgeIndex2 && edgeIndex2 == polygon->edgeIndex1) ||
				(edgeIndex1 == polygon->edgeIndex2 && edgeIndex2 == polygon->edgeIndex3) ||
				(edgeIndex1 == polygon->edgeIndex3 && edgeIndex2 == polygon->edgeIndex1) ||
				(edgeIndex1 == polygon->edgeIndex3 && edgeIndex2 == polygon->edgeIndex2)))
				return i;
		}
	}
	return -1;
}

int16 PathSystem::findPathPolygonByNodes(int16 nodeIndex1, int16 nodeIndex2) {
	if (nodeIndex1 != -1 && nodeIndex2 != -1) {
		for (int16 i = 0; i < _pathPolygons.count(); i++) {
			PathPolygon *polygon = &_pathPolygons[i];
			if (polygon->used == 1 && polygon->enabled == 1 &&
				((nodeIndex1 == polygon->nodeIndex1 && nodeIndex2 == polygon->nodeIndex2) ||
				(nodeIndex1 == polygon->nodeIndex1 && nodeIndex2 == polygon->nodeIndex3) ||
				(nodeIndex2 == polygon->nodeIndex1 && nodeIndex1 == polygon->nodeIndex2) ||
				(nodeIndex2 == polygon->nodeIndex1 && nodeIndex1 == polygon->nodeIndex3) ||
				(nodeIndex1 == polygon->nodeIndex2 && nodeIndex2 == polygon->nodeIndex3) ||
				(nodeIndex2 == polygon->nodeIndex2 && nodeIndex1 == polygon->nodeIndex3)))
			return i;
		}
	}
	return -1;
}

int16 PathSystem::findPathPolygonByNodeAndEdge(int16 nodeIndex, int16 edgeIndex) {
	if (nodeIndex != -1 && edgeIndex != -1) {
		if (nodeIndex == _pathEdges[edgeIndex].nodeIndex1 ||
			nodeIndex == _pathEdges[edgeIndex].nodeIndex2)
			return edgeIndex;
		for (int16 i = 0; i < _pathPolygons.count(); i++) {
			PathPolygon *polygon = &_pathPolygons[i];
			if (polygon->used == 1 && polygon->enabled == 1 &&
				((nodeIndex == polygon->nodeIndex1 && edgeIndex == polygon->edgeIndex1) ||
				(nodeIndex == polygon->nodeIndex1 && edgeIndex == polygon->edgeIndex2) ||
				(nodeIndex == polygon->nodeIndex1 && edgeIndex == polygon->edgeIndex3) ||
				(nodeIndex == polygon->nodeIndex2 && edgeIndex == polygon->edgeIndex1) ||
				(nodeIndex == polygon->nodeIndex2 && edgeIndex == polygon->edgeIndex2) ||
				(nodeIndex == polygon->nodeIndex2 && edgeIndex == polygon->edgeIndex3) ||
				(nodeIndex == polygon->nodeIndex3 && edgeIndex == polygon->edgeIndex1) ||
				(nodeIndex == polygon->nodeIndex3 && edgeIndex == polygon->edgeIndex2) ||
				(nodeIndex == polygon->nodeIndex3 && edgeIndex == polygon->edgeIndex3)))
			return i;
		}
	}
	return -1;
}

void PathSystem::path25669(int16 nodeIndex1, uint16 distance, int16 nodeIndex2, int16 nodeIndex3) {

	int16 outerIndex = 0;

	clearPathNodeHelpers();

	_pathNodeHelpers[nodeIndex1].unk2 = 1;
	_pathNodeHelpers[nodeIndex1].outerIndex = 0;
	_pathNodeHelpers[nodeIndex1].distance = distance;
	_pathNodeHelpersCount = 1;

	while (_pathNodeHelpersCount > 0) {
		for (int16 nodeIndex = 0; nodeIndex < kMaxPathNodes; nodeIndex++) {
			PathNode *node = &_pathNodes[nodeIndex];
			PathNodeHelper *nodeHelper = &_pathNodeHelpers[nodeIndex];
			if (nodeHelper->unk2 == 1 && nodeHelper->outerIndex == outerIndex) {
				nodeHelper->unk2 = 0;
				_pathNodeHelpersCount--;
				for (int16 connIndex = 0; connIndex < kMaxPathNodeConnections; connIndex++) {
					PathNodeConnection *nodeConnection = &node->connections[connIndex];
					if (nodeConnection->used == 1 && nodeConnection->enabled == 1 &&
						nodeConnection->nodeIndex != nodeHelper->nodeIndex &&
						nodeConnection->nodeIndex != nodeIndex2 &&
						nodeConnection->nodeIndex != nodeIndex3) {
						PathNodeHelper *nodeHelper2 = &_pathNodeHelpers[nodeConnection->nodeIndex];
						if (nodeConnection->distance + nodeHelper->distance < nodeHelper2->distance) {
							if (nodeHelper2->unk2 == 0) {
								nodeHelper2->unk2 = 1;
								_pathNodeHelpersCount++;
							}
							nodeHelper2->outerIndex = outerIndex + 1;
							nodeHelper2->distance = nodeHelper->distance + nodeConnection->distance;
							nodeHelper2->nodeIndex = nodeIndex;
						}
					}
				}
			}
		}
		outerIndex++;
	}

}

void PathSystem::clearPathNodeHelpers() {
	for (int16 nodeIndex = 0; nodeIndex < kMaxPathNodes; nodeIndex++) {
		PathNodeHelper *nodeHelper = &_pathNodeHelpers[nodeIndex];
		nodeHelper->unk2 = 0;
		nodeHelper->outerIndex = -1;
		nodeHelper->distance = 0xFFFF;
		nodeHelper->nodeIndex = -1;
	}
	_pathNodeHelpersCount = 0;
}

void PathSystem::path2579A(int16 nodeIndex) {

	if (_pathNodeHelpers[nodeIndex].nodeIndex != -1)
		path2579A(_pathNodeHelpers[nodeIndex].nodeIndex);

	_pathCurrResult->x = _pathNodes[nodeIndex].x;
	_pathCurrResult->y = _pathNodes[nodeIndex].y;
	_pathCurrResult->scale = _pathNodes[nodeIndex].scale;
	_pathCurrResult->nodeIndex = nodeIndex;
	_pathCurrResult->edgeIndex = findPathConnectionEdge(nodeIndex, _pathNodeHelpers[nodeIndex].nodeIndex);
	if (_pathEdges[_pathCurrResult->edgeIndex].unk2 == 1)
		_pathCurrResult->edgeIndex = -1;
	_pathCurrResult->polyIndex = -1;
	_pathResultsCount++;
	_pathCurrResult = &_pathResults[_pathResultsCount];

}

bool PathSystem::insertPathEdge(int16 nodeIndex1, int16 nodeIndex2) {

	int16 edgeIndex, connIndex1, connIndex2;

	if (findPathConnectionEdge(nodeIndex1, nodeIndex2) != -1)
		return true;

	edgeIndex = _pathEdges.getFreeSlot();
	connIndex1 = _pathNodes[nodeIndex1].connections.getFreeSlot();
	connIndex2 = _pathNodes[nodeIndex2].connections.getFreeSlot();
	if (edgeIndex == -1 || connIndex1 == -1 || connIndex2 == -1)
		return false;

	PathEdge *edge = &_pathEdges[edgeIndex];
	edge->enabled = 1;
	edge->unk2 = 1;
	edge->polyCount = 0;
	edge->nodeIndex1 = nodeIndex1;
	edge->nodeIndex2 = nodeIndex2;
	edge->distance = calcPathNodesDistance(nodeIndex1, nodeIndex2);
	edge->used = 1;
	_pathEdgesCount++;

	PathNodeConnection *nodeConnection1 = &_pathNodes[nodeIndex1].connections[connIndex1];
	nodeConnection1->nodeIndex = nodeIndex2;
	nodeConnection1->edgeIndex = edgeIndex;
	nodeConnection1->distance = edge->distance;
	nodeConnection1->enabled = 1;
	nodeConnection1->used = 1;
	_pathNodes[nodeIndex1].connectionsCount++;

	PathNodeConnection *nodeConnection2 = &_pathNodes[nodeIndex2].connections[connIndex2];
	nodeConnection2->nodeIndex = nodeIndex1;
	nodeConnection2->edgeIndex = edgeIndex;
	nodeConnection2->distance = edge->distance;
	nodeConnection2->enabled = 1;
	nodeConnection2->used = 1;
	_pathNodes[nodeIndex2].connectionsCount++;

	return true;

}

void PathSystem::cleanupPathResult() {

	int16 resultIndex = 0;

	if (_pathResultsCount <= 0)
		return;

	while (resultIndex < _pathResultsCount - 2) {
		PathResult *currResult = &_pathResults[resultIndex];
		if (findPathConnectionEdge(currResult->nodeIndex, _pathResults[resultIndex + 2].nodeIndex) != -1) {
			deletePathResultItem(resultIndex + 1);
			resultIndex = 0;
		} else {
			currResult->direction = _vm->calcDirection(currResult->x, currResult->y,
				_pathResults[resultIndex + 1].x, _pathResults[resultIndex + 1].y);
			resultIndex++;
		}
	}

	if (_pathResultsCount == 2 &&
		_pathResults[0].x == _pathResults[1].x &&
		_pathResults[0].y == _pathResults[1].y)
		_pathResultsCount = 1;

	if (_pathResultsCount > 1) {
		PathResult *currResult = &_pathResults[_pathResultsCount - 1];
		PathResult *prevResult = &_pathResults[_pathResultsCount - 2];
		prevResult->direction = _vm->calcDirection(prevResult->x, prevResult->y, currResult->x, currResult->y);
	}

	_pathResults[_pathResultsCount - 1].direction = -1;

}

void PathSystem::deletePathResultItem(int16 resultIndex) {
	memcpy(&_pathResults[resultIndex], &_pathResults[resultIndex + 1], (_pathResultsCount - resultIndex - 1) * sizeof(PathResult));
	_pathResultsCount--;
}

int16 PathSystem::calcPathEdgeScale(int16 y, int16 edgeIndex) {
	PathEdge *edge = &_pathEdges[edgeIndex];
	PathNode *node1 = &_pathNodes[edge->nodeIndex1];
	PathNode *node2 = &_pathNodes[edge->nodeIndex2];
	int16 result;
	if (node1->scale == node2->scale) {
		result = node1->scale;
	} else if (node1->y > node2->y) {
		result = node1->scale - ((node1->y - y) * (node1->scale - node2->scale) / (node1->y - node2->y));
	} else {
		result = node2->scale - ((node2->y - y) * (node2->scale - node1->scale) / (node2->y - node1->y));
	}
	return result;
}

int16 PathSystem::calcPathPolygonScale(int16 y, int16 polygonIndex) {

	PathPolygon *polygon = &_pathPolygons[polygonIndex];
	int16 y1 = _pathNodes[polygon->nodeIndex1].y;
	int16 y2 = _pathNodes[polygon->nodeIndex2].y;
	int16 y3 = _pathNodes[polygon->nodeIndex3].y;
	int16 scale1 = _pathNodes[polygon->nodeIndex1].scale;
	int16 scale2 = _pathNodes[polygon->nodeIndex2].scale;
	int16 scale3 = _pathNodes[polygon->nodeIndex3].scale;

	if (scale1 == scale2 && scale2 == scale3)
		return scale1;

	if (y1 > y2) {
		SWAP(y1, y2);
		SWAP(scale1, scale2);
	}

	if (y1 > y3) {
		SWAP(y1, y3);
		SWAP(scale1, scale3);
	}

	if (y3 < y2) {
		y3 = y2;
		scale3 = scale2;
	}

	return scale3 - (y3 - y) * (scale3 - scale1) / (y3 - y1);

}

bool PathSystem::path260BA(int16 sourceNodeIndex, int16 edgeIndex, int16 sourcePolyIndex, int16 destX, int16 destY) {

	bool doLoopFlag = false;
	int16 destPolyIndex, destNodeIndex, destEdgeIndex, nodeIndex2 = -1;
	PathPolygon *sourcePolygon, *destPolygon = NULL;
	PathNode *pathNode = NULL;

	sourcePolygon = &_pathPolygons[sourcePolyIndex];

	destPolyIndex = findPathPolygonAtPos(destX, destY);
	if (destPolyIndex != -1 && _pathPolygons[destPolyIndex].enabled == 0)
		destPolyIndex = -1;

	if (destPolyIndex != -1)
		destPolygon = &_pathPolygons[destPolyIndex];

	if (sourcePolyIndex == destPolyIndex && destPolyIndex != -1) {
		// source and dest points are in the same polygon
		PathNode *node = &_pathNodes[sourceNodeIndex];
		PathResult *pathResult;
		pathResult = &_pathResults[_pathResultsCount];
		pathResult->direction = _vm->calcDirection(node->x, node->y, destX, destY);
		pathResult->edgeIndex = -1;
		pathResult->nodeIndex = -1;
		pathResult->polyIndex = sourcePolyIndex;
		pathResult->x = node->x;
		pathResult->y = node->y;
		pathResult->scale = calcPathPolygonScale(node->y, destPolyIndex);
		_pathResultsCount++;
		if (node->x != destX || node->y != destY) {
			pathResult = &_pathResults[_pathResultsCount];
			pathResult->direction = -1;
			pathResult->edgeIndex = -1;
			pathResult->nodeIndex = -1;
			pathResult->polyIndex = sourcePolyIndex;
			pathResult->x = destX;
			pathResult->y = destY;
			pathResult->scale = calcPathPolygonScale(destY, sourcePolyIndex);
			_pathResultsCount++;
		}
		return true;
	}

	_pathHelpersCount = 0;

	if (destPolyIndex != -1) {

		destNodeIndex = findPathNodeAtPos(destX, destY, false);
		if (destNodeIndex != -1 && _pathNodes[destNodeIndex].connectionsCount == 0) {
			nodeIndex2 = destNodeIndex;
			destNodeIndex = -1;
		}

		if (destNodeIndex == -1) {
			_pathHelpers[0].edgeIndex = -1;
			_pathHelpers[0].nodeIndex = -1;
			_pathHelpers[0].x = destX;
			_pathHelpers[0].y = destY;
			_pathHelpers[0].distance0 = 0;
			_pathHelpersCount++;
		}

		if (sourcePolyIndex != -1 && edgeIndex != -1 && destPolygon->hasEdge(edgeIndex) &&
			sourcePolygon->hasEdge(edgeIndex)) {
			edgeIndex = -1;
		}

	}

	//-> loc_2631B
	findClosestPointOnEdge(destX, destY, false, false, nodeIndex2);

	if (_pathHelpersCount == 0)
		return false;

	int16 oldDestPolyIndex = destPolyIndex;

	for (int16 pathHelperIndex = 0; pathHelperIndex < _pathHelpersCount; pathHelperIndex++) {
		PathHelper *pathHelper = &_pathHelpers[pathHelperIndex];
		bool destNodeIsTemporary = false;
		PathEdge *pathEdge = NULL;

		destNodeIndex = pathHelper->nodeIndex;
		destEdgeIndex = pathHelper->edgeIndex;
		if (destEdgeIndex != -1)
			pathEdge = &_pathEdges[destEdgeIndex];

		//loc_2639C
		if (destNodeIndex == -1 && destEdgeIndex == -1)
			destPolyIndex = oldDestPolyIndex;
		else
			destPolyIndex = -1;

		//loc_263C2
		if (destPolyIndex != -1)
			destPolygon = &_pathPolygons[destPolyIndex];

		//loc_263DA
		if (destNodeIndex == -1) {
			destNodeIndex = _pathNodes.getFreeSlot();
			pathNode = &_pathNodes[destNodeIndex];
			destNodeIsTemporary = true;
			pathNode->used = 1;
			pathNode->unk1 = 1;
			pathNode->x = pathHelper->x;
			pathNode->y = pathHelper->y;

			if (destEdgeIndex != -1) {
				pathNode->scale = calcPathEdgeScale(pathHelper->y, destEdgeIndex);
			} else if (destPolyIndex != -1) {
				pathNode->scale = calcPathPolygonScale(pathHelper->y, destPolyIndex);
			}

			pathNode->connectionsCount = 0;
			pathNode->connections.clear();
			_pathNodesCount++;

			if (destPolyIndex != -1) {
				insertPathEdge(destNodeIndex, destPolygon->nodeIndex1);
				insertPathEdge(destNodeIndex, destPolygon->nodeIndex2);
				insertPathEdge(destNodeIndex, destPolygon->nodeIndex3);
			} else {
				if (destEdgeIndex == -1)
					return false;
				if (pathEdge->polyCount > 0) {
					for (int16 polyIndex = 0; polyIndex < _pathPolygons.count(); polyIndex++) {
						PathPolygon *poly = &_pathPolygons[polyIndex];
						if (poly->used == 1 && poly->enabled == 1 && poly->hasEdge(destEdgeIndex)) {
							insertPathEdge(destNodeIndex, poly->nodeIndex1);
							insertPathEdge(destNodeIndex, poly->nodeIndex2);
							insertPathEdge(destNodeIndex, poly->nodeIndex3);
						}
					}
				} else {
					//loc_26561
					insertPathEdge(destNodeIndex, pathEdge->nodeIndex1);
					insertPathEdge(destNodeIndex, pathEdge->nodeIndex2);
				}
			}

		} else {
			//loc_26585
			pathNode = &_pathNodes[destNodeIndex];
		}

		//loc_26592
		_pathResultsCount = 0;

		if (sourceNodeIndex != destNodeIndex) {
			if (destPolyIndex != -1) {
				if (sourcePolyIndex != -1) {
					if (destEdgeIndex != -1 && destPolygon->hasEdge(destEdgeIndex) &&
						sourcePolygon->hasEdge(destEdgeIndex)) {
						destEdgeIndex = -1;
					}
					doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
				} else {
					//loc_2663C
					if (edgeIndex != -1) {
						if (destPolygon->hasEdge(destEdgeIndex)) {
							insertPathEdge(sourceNodeIndex, destNodeIndex);
							doLoopFlag = true;
						} else {
							doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
						}
					} else {
						//loc_2668C
						if (destPolygon->hasNode(sourceNodeIndex)) {
							insertPathEdge(sourceNodeIndex, destNodeIndex);
							doLoopFlag = true;
						} else {
							doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
						}
					}
				}
			} else {
				//loc_266BB
				if (sourcePolyIndex != -1) {
					if (destEdgeIndex != -1) {
						if (sourcePolygon->hasEdge(destEdgeIndex)) {
							insertPathEdge(sourceNodeIndex, destNodeIndex);
							doLoopFlag = true;
						} else {
							doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
						}
					} else {
						//loc_2671E
						if (sourcePolygon->hasNode(destNodeIndex)) {
							insertPathEdge(sourceNodeIndex, destNodeIndex);
							doLoopFlag = true;
						} else {
							doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
						}
					}
				} else {
					//loc_26753
					if ((edgeIndex == destEdgeIndex && edgeIndex != -1) ||
						(findPathPolygonByEdges(edgeIndex, destEdgeIndex) != -1) ||
						(findPathPolygonByNodes(sourceNodeIndex, destNodeIndex) != -1)) {
						insertPathEdge(sourceNodeIndex, destNodeIndex);
						doLoopFlag = true;
					} else {
						//loc_267AC
						if ((destEdgeIndex != -1 && pathEdge->hasNode(sourceNodeIndex)) ||
							(findPathPolygonByNodeAndEdge(sourceNodeIndex, destEdgeIndex) != -1) ||
							(findPathPolygonByNodeAndEdge(destNodeIndex, edgeIndex) != -1)) {
							insertPathEdge(sourceNodeIndex, destNodeIndex);
							doLoopFlag = true;
						} else {
							doLoopFlag = path25964(sourceNodeIndex, destNodeIndex, 1, edgeIndex, destEdgeIndex);
						}
					}
				}
			}

			// loop:
			if (!doLoopFlag) {
				for (int16 polyIndex = 0; polyIndex < _pathPolygons.count() && _pathEdgesCount < kMaxPathEdges; polyIndex++) {
					PathPolygon *poly = &_pathPolygons[polyIndex];
					if (poly->used == 1 && poly->enabled == 1) {
						if (polyIndex != sourcePolyIndex) {
							path25964(poly->nodeIndex1, sourceNodeIndex, 1, edgeIndex, -1);
							path25964(poly->nodeIndex2, sourceNodeIndex, 1, edgeIndex, -1);
							path25964(poly->nodeIndex3, sourceNodeIndex, 1, edgeIndex, -1);
						}
						if (polyIndex != destPolyIndex) {
							path25964(poly->nodeIndex1, destNodeIndex, 1, destEdgeIndex, -1);
							path25964(poly->nodeIndex2, destNodeIndex, 1, destEdgeIndex, -1);
							path25964(poly->nodeIndex3, destNodeIndex, 1, destEdgeIndex, -1);
						}
					}
				}
			}

			path25669(sourceNodeIndex, 0, -1, -1);

			if (_pathNodeHelpers[destNodeIndex].nodeIndex != -1) {
				_pathCurrResult = &_pathResults[_pathResultsCount];
				path2579A(destNodeIndex);
				cleanupPathResult();
			}

		} else {
			//loc_26963
			_pathResults[0].direction = -1;
			_pathResults[0].edgeIndex = edgeIndex;
			_pathResults[0].nodeIndex = destNodeIndex;
			_pathResults[0].polyIndex = sourcePolyIndex;
			_pathResults[0].x = pathNode->x;
			_pathResults[0].y = pathNode->y;
			if (sourcePolyIndex != -1) {
				pathNode->scale = calcPathPolygonScale(pathNode->y, sourcePolyIndex);
			} else {
				pathNode->scale = _pathNodes[destNodeIndex].scale;
			}
			_pathResultsCount++;
		}

		//loc_269CF
		if (destNodeIsTemporary)
			removePathNode(destNodeIndex);

		if (_pathResultsCount > 0) {
			if (destNodeIsTemporary) {
				if (_pathResults[0].nodeIndex == destNodeIndex) {
					_pathResults[0].nodeIndex = -1;
					_pathResults[0].edgeIndex = destEdgeIndex;
				}
				if (_pathResults[_pathResultsCount - 1].nodeIndex == destNodeIndex) {
					_pathResults[_pathResultsCount - 1].nodeIndex = -1;
					_pathResults[_pathResultsCount - 1].edgeIndex = destEdgeIndex;
				}
			}
			return true;
		}

	}

	return false;

}

void PathSystem::removePathNode(int16 nodeIndex) {
	if (nodeIndex != -1) {
		_pathNodesCount--;
		if (_pathNodes[nodeIndex].connectionsCount > 0)
			removePathEdgeByPathNodeIndex(nodeIndex);
		_pathNodes[nodeIndex].used = 0;
	}
}

void PathSystem::removePathEdgeByPathNodeIndex(int16 nodeIndex) {
	for (int16 edgeIndex = 0; edgeIndex < _pathEdges.count(); edgeIndex++) {
		PathEdge *edge = &_pathEdges[edgeIndex];
		if (edge->used == 1 && edge->hasNode(nodeIndex))
			removePathEdge(edgeIndex);
	}
}

void PathSystem::removePathEdge(int16 edgeIndex) {
	if (edgeIndex != -1) {
		_pathEdgesCount--;
		unlinkPathNodeConnection(_pathEdges[edgeIndex].nodeIndex1, _pathEdges[edgeIndex].nodeIndex2);
		_pathEdges[edgeIndex].used = 0;
	}
}

void PathSystem::unlinkPathNodeConnection(int16 nodeIndex1, int16 nodeIndex2) {
	for (int16 i = 0; i < kMaxPathNodeConnections; i++) {
		PathNodeConnection *conn1 = &_pathNodes[nodeIndex1].connections[i];
		PathNodeConnection *conn2 = &_pathNodes[nodeIndex2].connections[i];
		if (conn1->used == 1 && conn1->nodeIndex == nodeIndex2) {
			conn1->used = 0;
			_pathNodes[nodeIndex1].connectionsCount--;
		}
		if (conn2->used == 1 && conn2->nodeIndex == nodeIndex1) {
			conn2->used = 0;
			_pathNodes[nodeIndex2].connectionsCount--;
		}
	}
}

bool PathSystem::findPath(int16 sourceX, int16 sourceY, int16 destX, int16 destY) {

	debug(8, "PathSystem::findPath(%d, %d, %d, %d)", sourceX, sourceY, destX, destY);

	bool result, sourceNodeIsTemporary = false;
	PathPolygon *pathPolygon = NULL;
	int16 edgeIndex = -1;

	_pathResultsCount = 0;

	if (_pathEdgesCount == 0)
		return false;

	int16 sourcePolyIndex = findPathPolygonAtPos(sourceX, sourceY);
	if (sourcePolyIndex != -1 && !_pathPolygons[sourcePolyIndex].enabled)
		sourcePolyIndex = -1;

	if (sourcePolyIndex != -1)
		pathPolygon = &_pathPolygons[sourcePolyIndex];

	int16 sourceNodeIndex = findPathNodeAtPos(sourceX, sourceY, false);
	if (sourceNodeIndex != -1 && _pathNodes[sourceNodeIndex].connectionsCount == 0)
		return true;

	//loc_26AF3
	if (sourceNodeIndex == -1) {
		if (sourcePolyIndex == -1) {
			_pathHelpersCount = 0;
			findClosestPointOnEdge(sourceX, sourceY, true, false, -1);
			if (_pathHelpersCount == 0)
				return false;
			edgeIndex = _pathHelpers[0].edgeIndex;
			if (_pathHelpers[0].nodeIndex == -1) {
				sourceX = _pathHelpers[0].x;
				sourceY = _pathHelpers[0].y;
			} else {
				sourceNodeIndex = _pathHelpers[0].nodeIndex;
				sourceX = _pathNodes[sourceNodeIndex].x;
				sourceY = _pathNodes[sourceNodeIndex].y;
			}
		}

		//loc_26B80
		if (sourceNodeIndex == -1) {
			sourceNodeIndex = _pathNodes.getFreeSlot();
			PathNode *pathNode = &_pathNodes[sourceNodeIndex];
			sourceNodeIsTemporary = true;
			pathNode->unk1 = 1;
			pathNode->used = 1;
			pathNode->x = sourceX;
			pathNode->y = sourceY;
			if (edgeIndex != -1) {
				pathNode->scale = calcPathEdgeScale(sourceY, edgeIndex);
			} else if (sourcePolyIndex != -1) {
				pathNode->scale = calcPathPolygonScale(sourceY, sourcePolyIndex);
			}
			pathNode->connectionsCount = 0;
			pathNode->connections.clear();
			_pathNodesCount++;
			if (sourcePolyIndex != -1) {
				insertPathEdge(sourceNodeIndex, pathPolygon->nodeIndex1);
				insertPathEdge(sourceNodeIndex, pathPolygon->nodeIndex2);
				insertPathEdge(sourceNodeIndex, pathPolygon->nodeIndex3);
			} else {
				if (edgeIndex == -1)
					return false;
				if (_pathEdges[edgeIndex].polyCount > 0) {
					for (int16 polyIndex = 0; polyIndex < _pathPolygons.count(); polyIndex++) {
						PathPolygon *poly = &_pathPolygons[polyIndex];
						if (poly->used == 1 && poly->enabled == 1 && poly->hasEdge(edgeIndex)) {
							insertPathEdge(sourceNodeIndex, poly->nodeIndex1);
							insertPathEdge(sourceNodeIndex, poly->nodeIndex2);
							insertPathEdge(sourceNodeIndex, poly->nodeIndex3);
						}
					}
				} else {
					insertPathEdge(sourceNodeIndex, _pathEdges[edgeIndex].nodeIndex1);
					insertPathEdge(sourceNodeIndex, _pathEdges[edgeIndex].nodeIndex2);
				}
			}

		}
	}

	//loc_26CD7
	if (sourceNodeIndex == -1 && edgeIndex == -1 && sourcePolyIndex == -1)
		return false;

	result = path260BA(sourceNodeIndex, edgeIndex, sourcePolyIndex, destX, destY);

	if (sourceNodeIsTemporary) {
		if (_pathResultsCount > 0) {
			if (_pathResults[0].nodeIndex == sourceNodeIndex) {
				_pathResults[0].nodeIndex = -1;
				_pathResults[0].edgeIndex = edgeIndex;
			}
			if (_pathResults[_pathResultsCount - 1].nodeIndex == sourceNodeIndex) {
				_pathResults[_pathResultsCount - 1].nodeIndex = -1;
				_pathResults[_pathResultsCount - 1].edgeIndex = edgeIndex;
			}
		}
		removePathNode(sourceNodeIndex);
	}

	return result;

}

void PathSystem::clearPathWalker() {
	_pathNodes.clear();
	_pathNodesCount = 0;
	_pathEdges.clear();
	_pathEdgesCount = 0;
	_pathPolygons.clear();
	_pathPolygonsCount = 0;
	_pathResultsCount = 0;
}

void PathSystem::buildPathSystem() {
	if (_pathEdgesCount > 0) {
		for (int16 outerPolyIndex = 0; outerPolyIndex < kMaxPathPolygons && _pathEdgesCount < kMaxPathEdges; outerPolyIndex++) {
			PathPolygon *outerPoly = &_pathPolygons[outerPolyIndex];
			if (outerPoly->used == 1 && outerPoly->enabled == 1) {
				for (int16 innerPolyIndex = outerPolyIndex + 1; innerPolyIndex < kMaxPathPolygons && _pathEdgesCount < kMaxPathEdges; innerPolyIndex++) {
					PathPolygon *innerPoly = &_pathPolygons[innerPolyIndex];
					if (innerPoly->used == 1 && innerPoly->enabled == 1) {
						path25964(outerPoly->nodeIndex1, innerPoly->nodeIndex1, 1, -1, -1);
						path25964(outerPoly->nodeIndex1, innerPoly->nodeIndex2, 1, -1, -1);
						path25964(outerPoly->nodeIndex1, innerPoly->nodeIndex3, 1, -1, -1);
						path25964(outerPoly->nodeIndex2, innerPoly->nodeIndex1, 1, -1, -1);
						path25964(outerPoly->nodeIndex2, innerPoly->nodeIndex2, 1, -1, -1);
						path25964(outerPoly->nodeIndex2, innerPoly->nodeIndex3, 1, -1, -1);
						path25964(outerPoly->nodeIndex3, innerPoly->nodeIndex1, 1, -1, -1);
						path25964(outerPoly->nodeIndex3, innerPoly->nodeIndex2, 1, -1, -1);
						path25964(outerPoly->nodeIndex3, innerPoly->nodeIndex3, 1, -1, -1);
					}
				}
			}
		}
	}
}

void PathSystem::clearPathSystem() {

	if (_pathNodesCount > 0) {
		for (int16 nodeIndex = 0; nodeIndex < kMaxPathNodes; nodeIndex++) {
			if (_pathNodes[nodeIndex].used == 1 && _pathNodes[nodeIndex].unk1 == 1) {
				removePathNode(nodeIndex);
			}
		}
	}

	if (_pathEdgesCount > 0) {
		for (int16 edgeIndex = 0; edgeIndex < kMaxPathEdges; edgeIndex++) {
			if (_pathEdges[edgeIndex].used == 1 && _pathEdges[edgeIndex].unk2 == 1) {
				removePathEdge(edgeIndex);
			}
		}
	}

}

void PathSystem::setPathEdgeOrPolygonEnabled(int16 type, int16 index, int16 enabled) {
	if (type == 0) {
		if (_pathEdges[index].enabled != enabled) {
			if (_pathSystemBuilt)
				clearPathSystem();
			setPathEdgeEnabled(index, enabled);
			if (_pathSystemBuilt)
				buildPathSystem();
		}
	} else if (type== 1) {
		if (_pathPolygons[index].enabled != enabled) {
			if (_pathSystemBuilt)
				clearPathSystem();
			// TODO: setPathPolygonEnabled(index, enabled);
			if (_pathSystemBuilt)
				buildPathSystem();
		}
	}
}

void PathSystem::setPathEdgeEnabled(int16 edgeIndex, int16 enabled) {

	PathNode *node;
	PathEdge *edge = &_pathEdges[edgeIndex];
	edge->enabled = enabled;

	node = &_pathNodes[edge->nodeIndex1];
	for (int16 connIndex = 0; connIndex < kMaxPathNodeConnections; connIndex++) {
		PathNodeConnection *nodeConnection = &_pathNodes[edge->nodeIndex1].connections[connIndex];
		if (nodeConnection->used == 1 && nodeConnection->nodeIndex == edge->nodeIndex2) {
			nodeConnection->enabled = enabled;
			break;
		}
	}

	for (int16 connIndex = 0; connIndex < kMaxPathNodeConnections; connIndex++) {
		PathNodeConnection *nodeConnection = &_pathNodes[edge->nodeIndex2].connections[connIndex];
		if (nodeConnection->used == 1 && nodeConnection->nodeIndex == edge->nodeIndex1) {
			nodeConnection->enabled = enabled;
			break;
		}
	}

}

void PathSystem::togglePathSystem(int16 flag) {
	if (flag == 1) {
		if (_pathSystemBuilt) {
			clearPathSystem();
			_pathSystemBuilt = false;
		}
	} else if (flag == 0) {
		if (!_pathSystemBuilt) {
			buildPathSystem();
			_pathSystemBuilt = true;
		}
	}
}

} // End of namespace Prisoner
