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

#ifndef MACS2_PATHFINDING_H
#define MACS2_PATHFINDING_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "macs2/macs2_constants.h"

namespace Macs2 {

// Area override table at scene+0x4EA8 (indexed by pathfinding value 0xC8..0xEF)
// Set by opcode 0x4D, read by getAreaAtPoint (1008:101d)
static constexpr uint16 AREA_OVERRIDE_MIN = 200;
static constexpr uint16 AREA_OVERRIDE_MAX = 239;
static constexpr uint16 AREA_OVERRIDE_COUNT = AREA_OVERRIDE_MAX - AREA_OVERRIDE_MIN + 1;

struct PathfindingPoint {
	uint8 _index = 0;
	Common::Point _position;
	Common::Array<uint8> _adjacentPoints;
};

struct PathfindingAreaOverride {
	bool _active = false;
	uint16 _index = 0;
	uint16 _overrideValue = 0;
};

struct PathRoute {
	Common::Array<uint16> nodes;
	int16 startIndex = 0;
	Common::Point firstWaypoint;
	bool found = false;
};

class Pathfinding {
public:
	Graphics::ManagedSurface _map;
	Common::Array<PathfindingPoint> _points;
	Common::Array<PathfindingAreaOverride> _walkOverrides;
	uint16 _areaOverrides[AREA_OVERRIDE_COUNT] = {0};
	uint16 _numPoints = 0;

	void createMap(int width, int height);
	void clearWalkOverrides();
	void clearAreaOverrides();

	// Walkability threshold 0xC8 uses signed 16-bit comparison in the binary (JL/JGE).
	// Values with (int16)value < 0xC8 are walkable heights; e.g. -2 (0xFFFE) is walkable.
	static inline bool isWalkabilityBlocking(uint16 value) {
		return (int16)value >= 0xC8;
	}
	static inline bool isWalkabilityWalkable(uint16 value) {
		return (int16)value < 0xC8;
	}

	uint16 walkabilityAt(int16 y, int16 x) const;
	uint16 walkabilityAt(const Common::Point &p) const;
	uint16 areaAt(uint16 x, uint16 y) const;

	bool getWalkOverride(uint16 index, uint16 &result) const;
	void setWalkOverride(uint16 index, uint16 overrideValue);
	void removeWalkOverride(uint16 index);

	bool isLineWalkable(int16 y1, int16 x1, int16 y2, int16 x2) const;
	void snapToWalkable(int16 *pTargetY, int16 *pTargetX, int16 charY, int16 charX) const;

	int nodeCount() const { return (int)_numPoints; }
	int euclideanDistance(const Common::Point &a, const Common::Point &b) const;
	int walkableDistance(int nodeA, int nodeB) const;

	PathRoute calculateRoute(const Common::Point &from, const Common::Point &to);

private:
	int _visitedStack[17] {};
	int _visitedCount = 0;
	uint16 areaOverrideAt(uint16 index) const;

	int computeMinCostToReachable(int nodeIndex, int prevNode, const bool *reachable, int nodeCount, const Common::Point &finalDest);
	bool canNodeConnectSourceToTarget(uint16 nodeIndex, const Common::Point &charPos, const Common::Point &target, const bool *reachable, int nodeCount) const;
	void floodFillConnectedNodes(int nodeIndex, bool *visited, int nodeCount) const;
};

} // namespace Macs2

#endif
