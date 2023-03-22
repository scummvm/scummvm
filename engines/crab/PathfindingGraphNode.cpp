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

#include "PathfindingGraphNode.h"
#include "stdafx.h"

PathfindingGraphNode::PathfindingGraphNode(void) {
	id = -1;
	movementCost = -1.0f;
}

PathfindingGraphNode::PathfindingGraphNode(Vector2f pos, int i) : position(pos) {
	id = i;
	movementCost = -1.0f;
}

PathfindingGraphNode::~PathfindingGraphNode(void) {
}

void PathfindingGraphNode::AddNeighbor(PathfindingGraphNode *node) {
	AddNeighbor(node, false);
}

void PathfindingGraphNode::AddNeighbor(PathfindingGraphNode *node, bool ignoreDistance) {
	// You can't be your own neighbor. Sorry.
	if (node->id == this->id)
		return;

	// Make sure that the node is not already a neighbor (SZ)
	for (int i = 0; i < neighborNodes.size(); ++i) {
		if (neighborNodes[i]->id == node->id) {
			return;
		}
	}

	neighborNodes.push_back(node);

	// Determine the cost.
	if (ignoreDistance) {
		neighborCosts.push_back(node->movementCost);
	} else {
		Vector2f distVec = node->position - this->position;

		neighborCosts.push_back(distVec.Magnitude() * node->movementCost);
	}
}

bool PathfindingGraphNode::AdjacentToObstacle() const {
	for (auto iter = neighborNodes.begin(); iter != neighborNodes.end(); ++iter) {
		if ((*iter)->GetMovementCost() < 0)
			return true;
	}

	return false;
}

bool PathfindingGraphNode::AdjacentToNode(PathfindingGraphNode *otherNode) {
	for (int i = 0; i < neighborNodes.size(); ++i) {
		if (neighborNodes[i] == otherNode)
			return true;
	}

	return false;
}

//
// const std::vector< PathfindingGraphNode*>& PathfindingGraphNode::GetNeighbors() const
//{
//	return neighborNodes;
//}
