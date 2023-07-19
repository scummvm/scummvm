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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_PATHFINDINGGRAPHNODE_H
#define CRAB_PATHFINDINGGRAPHNODE_H

#include "crab/Rectangle.h"
#include "crab/vectors.h"

namespace Crab {

// This is the basic pathfinding node that will construct the pathfinding graph. (SZ)
// Although Unrest is using a square grid based pathfinding map, this is made to be a general use pathfinding node.
class PathfindingGraphNode {
	friend class PathfindingGrid;

	int _id; // Each ID will be assigned when the pathfinding graph is generated and will identify each node.

	float _movementCost; // 1 is open terrain, >1 is impeding terrain, <0 is completely obstructed

	Vector2f _position; // Position of the node

	Rect _collisionRect; // Represents spaced covered by the node.

public:
	Common::Array<PathfindingGraphNode *> _neighborNodes;
	Common::Array<float> _neighborCosts; // The movement cost for the neighbor nodes (distance to the node X the nodes movement cost)
									  // This is stored to prevent having to recalculate each frame.

	PathfindingGraphNode(void);
	PathfindingGraphNode(Vector2f pos, int i);

	~PathfindingGraphNode(void);

	float getMovementCost() {
		return _movementCost;
	}

	Vector2f getPosition() const {
		return _position;
	}

	// Adds node to neighbor vector and cost to neighbor costs
	void addNeighbor(PathfindingGraphNode *node);

	// Same as above, but does not calculate distance. Used when all nodes
	// are equidistant
	void addNeighbor(PathfindingGraphNode *node, bool ignoreDistance);

	// const Common::Array< PathfindingGraphNode*>& GetNeighbors() const;

	Rect getRect() const {
		return _collisionRect;
	}

	// Return true if the node is adjacent to a blocked node
	bool adjacentToObstacle() const;

	// Return true if the node is adjacent to the otherNode
	bool adjacentToNode(PathfindingGraphNode *otherNode);
};

} // End of namespace Crab

#endif // CRAB_PATHFINDINGGRAPHNODE_H
