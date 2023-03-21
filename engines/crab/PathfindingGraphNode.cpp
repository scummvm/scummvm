#include "stdafx.h"
#include "PathfindingGraphNode.h"

PathfindingGraphNode::PathfindingGraphNode(void)
{
	id = -1;
	movementCost = -1.0f;
}

PathfindingGraphNode::PathfindingGraphNode(Vector2f pos, int i) : position(pos)
{
	id = i;
	movementCost = -1.0f;
}

PathfindingGraphNode::~PathfindingGraphNode(void)
{
}

void PathfindingGraphNode::AddNeighbor(PathfindingGraphNode* node)
{
	AddNeighbor(node, false);
}

void PathfindingGraphNode::AddNeighbor(PathfindingGraphNode* node, bool ignoreDistance)
{
	//You can't be your own neighbor. Sorry.
	if (node->id == this->id)
		return;

	//Make sure that the node is not already a neighbor (SZ)
	for (int i = 0; i < neighborNodes.size(); ++i)
	{
		if (neighborNodes[i]->id == node->id)
		{
			return;
		}
	}

	neighborNodes.push_back(node);

	//Determine the cost.
	if (ignoreDistance)
	{
		neighborCosts.push_back(node->movementCost);
	}
	else
	{
		Vector2f distVec = node->position - this->position;

		neighborCosts.push_back(distVec.Magnitude() * node->movementCost);
	}
}

bool PathfindingGraphNode::AdjacentToObstacle() const
{
	for (auto iter = neighborNodes.begin(); iter != neighborNodes.end(); ++iter)
	{
		if ((*iter)->GetMovementCost() < 0)
			return true;
	}

	return false;
}

bool PathfindingGraphNode::AdjacentToNode(PathfindingGraphNode* otherNode)
{
	for (int i = 0; i < neighborNodes.size(); ++i)
	{
		if (neighborNodes[i] == otherNode)
			return true;
	}

	return false;
}

//
//const std::vector< PathfindingGraphNode*>& PathfindingGraphNode::GetNeighbors() const
//{
//	return neighborNodes;
//}