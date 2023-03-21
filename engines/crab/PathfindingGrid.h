#pragma once

#include "common_header.h"
#include "PathfindingGraphNode.h"
#include <math.h>

namespace TMX
{
	class TMXMap;
}

//This is the grid of pathfinding nodes that is formed for the level (SZ)
class PathfindingGrid
{
	friend class PathfindingGraphNode;

	PathfindingGraphNode** nodes;	//2D array of nodes (size is [dimensions.x][dimensions.y]

	Vector2i dimensions;	//rows and columns of nodes.
	Vector2f cellSize;		//size of a cell in width and height

	//Neighbor node1 to node2.
	void ConnectNodes(PathfindingGraphNode* node1, PathfindingGraphNode* node2);

public:

	//these are the default graph node costs.
	//they can be overwritten by values stored in the level's file.(SZ)
	static const int BLOCKED = -1;
	static const int  OPEN = 1;
	static const int  STAIRS = 5;

	//These are the actual data members used to assign costs. (SZ)
	int blockedCost;
	int openCost;
	int stairsCost;

	PathfindingGrid(void);
	~PathfindingGrid(void);

	void SetupNodes(TMX::TMXMap map);

	//Return the node at the given point (SZ)
	PathfindingGraphNode* GetNodeAtPoint(Vector2f point);
	//Return the node at the given coordinates (SZ)
	PathfindingGraphNode* GetNodeAtCoords(int x, int y){ return &nodes[x][y]; }

	Vector2i GetDimensions(){ return dimensions; }

	Vector2f GetCellSize(){ return cellSize; }

	//Returns the nearest open node to the compare spot, starting with the given nodePos
	//and iterating through its neighbors. (SZ)
	PathfindingGraphNode* GetNearestOpenNode(Vector2f nodePos, Vector2f comparePos);

	//Return true if two nodes share and adjacency to the same blocked node.
	//Can be used to find corners that shouldn't be cut.
	std::vector<PathfindingGraphNode*> CornerCheck(const PathfindingGraphNode* node1, const PathfindingGraphNode* node2);
};