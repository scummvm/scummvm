#pragma once

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
