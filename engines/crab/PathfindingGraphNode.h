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

#include "Rectangle.h"
#include "vectors.h"

// This is the basic pathfinding node that will construct the pathfinding graph. (SZ)
// Although Unrest is using a square grid based pathfinding map, this is made to be a general use pathfinding node.
class PathfindingGraphNode {
	friend class PathfindingGrid;

	int id; // Each ID will be assigned when the pathfinding graph is generated and will identify each node.

	float movementCost; // 1 is open terrain, >1 is impeding terrain, <0 is completely obstructed

	Vector2f position; // Position of the node

	Rect collisionRect; // Represents spaced covered by the node.

public:
	std::vector<PathfindingGraphNode *> neighborNodes;
	std::vector<float> neighborCosts; // The movement cost for the neighbor nodes (distance to the node X the nodes movement cost)
									  // This is stored to prevent having to recalculate each frame.

	PathfindingGraphNode(void);
	PathfindingGraphNode(Vector2f pos, int i);

	~PathfindingGraphNode(void);

	float GetMovementCost() { return movementCost; }
	Vector2f GetPosition() const { return position; }

	// Adds node to neighbor vector and cost to neighbor costs
	void AddNeighbor(PathfindingGraphNode *node);

	// Same as above, but does not calculate distance. Used when all nodes
	// are equidistant
	void AddNeighbor(PathfindingGraphNode *node, bool ignoreDistance);

	// const std::vector< PathfindingGraphNode*>& GetNeighbors() const;

	Rect GetRect() const { return collisionRect; }

	// Return true if the node is adjacent to a blocked node
	bool AdjacentToObstacle() const;

	// Return true if the node is adjacent to the otherNode
	bool AdjacentToNode(PathfindingGraphNode *otherNode);
};

#endif // CRAB_PATHFINDINGGRAPHNODE_H
