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

#include "macs2/pathfinding.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "graphics/pixelformat.h"

namespace Macs2 {

void Pathfinding::createMap(int width, int height) {
	_map.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

void Pathfinding::clearWalkOverrides() {
	_walkOverrides.clear();
}

void Pathfinding::clearAreaOverrides() {
	memset(_areaOverrides, 0, sizeof(_areaOverrides));
}

uint16 Pathfinding::walkabilityAt(int16 y, int16 x) const {
	if (x < 0 || x >= _map.w || y < 0 || y >= _map.h) {
		return 0;
	}
	uint16 value = _map.getPixel(x, y);
	if (value >= 0xC8 && value <= 0xEF) {
		uint16 overrideResult;
		if (getWalkOverride(value, overrideResult)) {
			return overrideResult;
		}
		return 0xFF;
	}
	return value;
}

uint16 Pathfinding::walkabilityAt(const Common::Point &p) const {
	return walkabilityAt((int16)p.y, (int16)p.x);
}

uint16 Pathfinding::areaAt(uint16 x, uint16 y) const {
	if (x >= (uint16)_map.w || y >= (uint16)_map.h) {
		return 0;
	}
	uint16 result = _map.getPixel(x, y);
	if (result >= AREA_OVERRIDE_MIN && result < 250) {
		const uint16 overrideValue = areaOverrideAt(result);
		if (overrideValue >= AREA_OVERRIDE_MIN) {
			result = overrideValue;
		}
	}
	return result;
}

bool Pathfinding::getWalkOverride(uint16 index, uint16 &result) const {
	for (const PathfindingAreaOverride &current : _walkOverrides) {
		if (current._index == index && current._active) {
			result = current._overrideValue;
			return true;
		}
	}
	return false;
}

void Pathfinding::setWalkOverride(uint16 index, uint16 overrideValue) {
	removeWalkOverride(index);
	PathfindingAreaOverride override;
	override._active = true;
	override._index = index;
	override._overrideValue = overrideValue;
	_walkOverrides.push_back(override);
}

uint16 Pathfinding::areaOverrideAt(uint16 index) const {
	if (index < AREA_OVERRIDE_MIN || index > AREA_OVERRIDE_MAX) {
		return 0;
	}
	return _areaOverrides[index - AREA_OVERRIDE_MIN];
}

void Pathfinding::removeWalkOverride(uint16 index) {
	for (uint i = 0; i < _walkOverrides.size(); i++) {
		PathfindingAreaOverride &current = _walkOverrides[i];
		if (current._index == index) {
			_walkOverrides.remove_at(i);
			return;
		}
	}
}

void Pathfinding::snapToWalkable(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX) const {
	if (_map.w == 0 || _map.h == 0) {
		return;
	}

	int16 savedX = *pTargetX;
	int16 savedY = *pTargetY;
	const int16 maxY = (int16)(_map.h - 1);
	const int16 maxX = (int16)(_map.w - 1);

	// Phase 1: Scan downward with depth constraint
	// Condition: walkability >= 200 OR (targetY - walkability) < savedY
	while (true) {
		uint16 w = walkabilityAt(*pTargetY, savedX);
		if (isWalkabilityWalkable(w) && (*pTargetY - (int16)w >= savedY)) {
			break;
		}
		if (*pTargetY >= maxY) {
			break;
		}
		*pTargetY = *pTargetY + 1;
	}

	// Phase 2: Continue scanning to bottom for best depth match
	int16 scanY = *pTargetY;
	while (scanY <= maxY) {
		uint16 w = walkabilityAt(scanY, *pTargetX);
		if (scanY - (int16)w == savedY) {
			*pTargetY = scanY;
		}
		if (scanY == maxY) {
			break;
		}
		scanY++;
	}

	// Phase 3: If at screen bottom and still non-walkable, scan upward
	if (*pTargetY == maxY) {
		uint16 w = walkabilityAt(*pTargetY, *pTargetX);
		if (isWalkabilityBlocking(w)) {
			while (isWalkabilityBlocking(w) && *pTargetY > 0) {
				*pTargetY = *pTargetY - 1;
				w = walkabilityAt(*pTargetY, *pTargetX);
			}
		}
	}

	// Phase 4: If still non-walkable, scan X toward character
	uint16 w = walkabilityAt(*pTargetY, *pTargetX);
	if (isWalkabilityBlocking(w)) {
		*pTargetY = savedY;
		if (charX < *pTargetX) {
			while (true) {
				uint16 w2 = walkabilityAt(*pTargetY, *pTargetX);
				if (isWalkabilityWalkable(w2)) {
					break;
				}
				if (*pTargetX <= 0) {
					break;
				}
				*pTargetX = *pTargetX - 1;
			}
		} else {
			while (true) {
				uint16 w2 = walkabilityAt(*pTargetY, *pTargetX);
				if (isWalkabilityWalkable(w2)) {
					break;
				}
				if (*pTargetX >= maxX) {
					break;
				}
				*pTargetX = *pTargetX + 1;
			}
		}
		// Phase 5: If all failed, fall back to character position
		uint16 w2 = walkabilityAt(*pTargetY, *pTargetX);
		if (isWalkabilityBlocking(w2)) {
			*pTargetX = charX;
			*pTargetY = charY;
		}
	}

	// Phase 6: Gradient-based wall push
	int16 pushX = 0;
	int16 pushY = 0;
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY, *pTargetX + 1))) {
		pushX--;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY, *pTargetX - 1))) {
		pushX++;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY + 1, *pTargetX))) {
		pushY--;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY - 1, *pTargetX))) {
		pushY++;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY, *pTargetX + 2))) {
		pushX--;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY, *pTargetX - 2))) {
		pushX++;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY + 2, *pTargetX))) {
		pushY--;
	}
	if (isWalkabilityBlocking(walkabilityAt(*pTargetY - 2, *pTargetX))) {
		pushY++;
	}

	while (pushX != 0 || pushY != 0) {
		if (pushX < 0) {
			if (isWalkabilityWalkable(walkabilityAt(*pTargetY, *pTargetX - 1))) {
				*pTargetX = *pTargetX - 1;
			}
			pushX++;
		}
		if (pushX > 0) {
			if (isWalkabilityWalkable(walkabilityAt(*pTargetY, *pTargetX + 1))) {
				*pTargetX = *pTargetX + 1;
			}
			pushX--;
		}
		if (pushY < 0) {
			if (isWalkabilityWalkable(walkabilityAt(*pTargetY - 1, *pTargetX))) {
				*pTargetY = *pTargetY - 1;
			}
			pushY++;
		}
		if (pushY > 0) {
			if (isWalkabilityWalkable(walkabilityAt(*pTargetY + 1, *pTargetX))) {
				*pTargetY = *pTargetY + 1;
			}
			pushY--;
		}
	}
}

bool Pathfinding::isLineWalkable(int16 y1, int16 x1, int16 y2, int16 x2) const {
	uint16 error = 0;
	int16 curX = x2;
	int16 curY = y2;
	uint16 absDx = (uint16)ABS((int)(x2 - x1));
	uint16 absDy = (uint16)ABS((int)(y2 - y1));
	bool result = true;

	do {
		bool steppedX;
		if (error >= absDx) {
			if (y1 < y2) {
				curY--;
			}
			if (y2 < y1) {
				curY++;
			}
			error -= absDx;
			steppedX = false;
		} else {
			if (x1 < x2) {
				curX--;
			}
			if (x2 < x1) {
				curX++;
			}
			error += absDy;
			steppedX = true;
		}

		if (absDx > absDy && steppedX) {
			if (isWalkabilityBlocking(walkabilityAt(curY, curX))) {
				result = false;
			}
		}
		if (absDx <= absDy && !steppedX) {
			if (isWalkabilityBlocking(walkabilityAt(curY, curX))) {
				result = false;
			}
		}
	} while (curX != x1 || curY != y1);

	return result;
}

int Pathfinding::euclideanDistance(const Common::Point &a, const Common::Point &b) const {
	int32 dx = ABS((int)(b.x - a.x));
	int32 dy = ABS((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int i = 0;
	while (i < 0x500 && (int32)i * i < distSq) {
		i++;
	}
	return i;
}

int Pathfinding::walkableDistance(int nodeA, int nodeB) const {
	const Common::Point &a = _points[nodeA - 1]._position;
	const Common::Point &b = _points[nodeB - 1]._position;
	if (!isLineWalkable(a.y, a.x, b.y, b.x)) {
		return 0x500;
	}
	int32 dx = ABS((int)(b.x - a.x));
	int32 dy = ABS((int)(b.y - a.y));
	int32 distSq = dx * dx + dy * dy;
	int result = 0x280;
	int step = 0x280;
	do {
		step = step >> 1;
		if ((int32)result * result >= distSq) {
			result -= step;
		} else {
			result += step;
		}
	} while (step > 1);
	return result;
}

int Pathfinding::computeMinCostToReachable(int nodeIndex, int prevNode, const bool *reachable, int nodeCount, const Common::Point &finalDest) {
	_visitedCount++;
	_visitedStack[_visitedCount] = nodeIndex;

	int result;
	const Common::Point &nodePos = _points[nodeIndex - 1]._position;

	if (reachable[nodeIndex]) {
		if (!isLineWalkable(nodePos.y, nodePos.x, finalDest.y, finalDest.x)) {
			result = 0x500;
		} else {
			int32 dx = ABS((int)(finalDest.x - nodePos.x));
			int32 dy = ABS((int)(finalDest.y - nodePos.y));
			int32 distSq = dx * dx + dy * dy;
			int dist = 640;
			int step = 320;
			do {
				step = step >> 1;
				if ((int32)dist * dist >= distSq) {
					dist -= step;
				} else {
					dist += step;
				}
			} while (step > 1);
			result = dist;
		}
		_visitedCount--;
		return result;
	}

	int bestCost = 0x7777;
	int bestAdj = 0;
	const PathfindingPoint &pt = _points[nodeIndex - 1];
	int adjCount = (int)pt._adjacentPoints.size();

	if (adjCount > 0) {
		for (int i = 0; i < adjCount; i++) {
			const int adj = pt._adjacentPoints[i];
			if (adj == prevNode) {
				continue;
			}

			bool alreadyVisited = false;
			for (int j = 1; j < _visitedCount; j++) {
				if (_visitedStack[j] == adj) {
					alreadyVisited = true;
					break;
				}
			}
			if (alreadyVisited) {
				continue;
			}

			const int cost = computeMinCostToReachable(adj, nodeIndex, reachable, nodeCount, finalDest);
			if (cost < bestCost) {
				bestAdj = adj;
				bestCost = cost;
			}
		}
	}

	if (bestCost < 0x7777) {
		result = bestCost + walkableDistance(bestAdj, nodeIndex);
	} else {
		result = 0x7777;
	}

	_visitedCount--;
	return result;
}

bool Pathfinding::canNodeConnectSourceToTarget(uint16 nodeIndex, const Common::Point &charPos, const Common::Point &target, const bool *reachable, int nodeCount) const {
	const Common::Point &nodePos = _points[nodeIndex - 1]._position;
	if (!isLineWalkable(nodePos.y, nodePos.x, target.y, target.x))
		return false;

	bool visited[kPathNodeSlots + 1] = {};
	floodFillConnectedNodes(nodeIndex, visited, nodeCount);

	bool anySeesTarget = false;
	bool anySeenFromSource = false;
	for (int i = 1; i <= nodeCount; i++) {
		if (!visited[i])
			continue;
		const Common::Point &p = _points[i - 1]._position;
		if (isLineWalkable(p.y, p.x, target.y, target.x))
			anySeesTarget = true;
		if (isLineWalkable(charPos.y, charPos.x, p.y, p.x))
			anySeenFromSource = true;
	}
	return anySeesTarget && anySeenFromSource;
}

void Pathfinding::floodFillConnectedNodes(int nodeIndex, bool *visited, int nodeCount) const {
	if (nodeIndex < 1 || nodeIndex > nodeCount)
		return;
	if (visited[nodeIndex])
		return;
	visited[nodeIndex] = true;
	const PathfindingPoint &pt = _points[nodeIndex - 1];
	for (uint i = 0; i < pt._adjacentPoints.size(); i++) {
		floodFillConnectedNodes(pt._adjacentPoints[i], visited, nodeCount);
	}
}

PathRoute Pathfinding::calculateRoute(const Common::Point &from, const Common::Point &to) {
	// Binary calculatePath (1008:1966). Params: charY, charX, finalDestY, finalDestX.
	PathRoute route;
	route.firstWaypoint = to;
	const int count = nodeCount();

	bool reachable[kPathNodeSlots + 1] = {};
	for (int i = 1; i <= count; i++) {
		const Common::Point &nodePos = _points[i - 1]._position;
		reachable[i] = isLineWalkable(to.y, to.x, nodePos.y, nodePos.x);
	}

	int bestCost = 0x7777;
	int bestNode = 0;
	for (int i = 1; i <= count; i++) {
		const Common::Point &nodePos = _points[i - 1]._position;
		int costToDest = euclideanDistance(nodePos, to);
		int costToChar = euclideanDistance(nodePos, from);
		if (costToDest + costToChar < bestCost) {
			// Binary calls canNodeConnectSourceToTarget(destY, destX, charY, charX, i)
			// due to calculatePath being invoked with swapped source/dest params.
			if (canNodeConnectSourceToTarget(i, to, from, reachable, count)) {
				costToDest = euclideanDistance(nodePos, to);
				costToChar = euclideanDistance(nodePos, from);
				bestCost = costToDest + costToChar;
				bestNode = i;
			}
		}
	}

	if (bestNode == 0) {
		route.startIndex = 1;
		return route;
	}

	route.nodes.push_back(bestNode);
	int currentNode = bestNode;
	while (!reachable[currentNode]) {
		const PathfindingPoint &curPt = _points[currentNode - 1];
		int localBestCost = 0x7777;
		int nextNode = currentNode;
		for (uint a = 0; a < curPt._adjacentPoints.size(); a++) {
			const int adjIdx = curPt._adjacentPoints[a];
			const int cost = computeMinCostToReachable(adjIdx, 0x7fff, reachable, count, to);
			const int edgeCost = walkableDistance(adjIdx, currentNode);
			if (cost + edgeCost < localBestCost) {
				nextNode = adjIdx;
				localBestCost = cost + edgeCost;
			}
		}
		currentNode = nextNode;
		route.nodes.push_back(currentNode);
		if (route.nodes.size() > kPathNodeSlots)
			break;
	}

	for (uint i = 0; i + 1 < route.nodes.size(); i++) {
		const Common::Point &p1 = _points[route.nodes[i + 1] - 1]._position;
		const Common::Point &p2 = _points[route.nodes[i] - 1]._position;
		if (!isLineWalkable(p1.y, p1.x, p2.y, p2.x)) {
			route.nodes.clear();
			return route;
		}
	}

	route.startIndex = 0;
	while (route.startIndex + 1 < (int16)route.nodes.size()) {
		const Common::Point &nextNodePos = _points[route.nodes[route.startIndex + 1] - 1]._position;
		if (!isLineWalkable(nextNodePos.y, nextNodePos.x, from.y, from.x))
			break;
		route.startIndex++;
	}

	route.firstWaypoint = _points[route.nodes[route.startIndex] - 1]._position;
	route.found = true;
	return route;
}

} // namespace Macs2
