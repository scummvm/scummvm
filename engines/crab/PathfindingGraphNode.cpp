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

#include "crab/PathfindingGraphNode.h"

namespace Crab {

PathfindingGraphNode::PathfindingGraphNode() {
	_id = -1;
	_movementCost = -1.0f;
}

PathfindingGraphNode::PathfindingGraphNode(Vector2f pos, int i) : _position(pos) {
	_id = i;
	_movementCost = -1.0f;
}

PathfindingGraphNode::~PathfindingGraphNode() {
}

void PathfindingGraphNode::addNeighbor(PathfindingGraphNode *node) {
	addNeighbor(node, false);
}

void PathfindingGraphNode::addNeighbor(PathfindingGraphNode *node, bool ignoreDistance) {
	// You can't be your own neighbor. Sorry.
	if (node->_id == this->_id)
		return;

	// Make sure that the node is not already a neighbor (SZ)
	for (uint i = 0; i < _neighborNodes.size(); ++i) {
		if (_neighborNodes[i]->_id == node->_id) {
			return;
		}
	}

	_neighborNodes.push_back(node);

	// Determine the cost.
	if (ignoreDistance) {
		_neighborCosts.push_back(node->_movementCost);
	} else {
		Vector2f distVec = node->_position - this->_position;

		_neighborCosts.push_back(distVec.magnitude() * node->_movementCost);
	}
}

bool PathfindingGraphNode::adjacentToObstacle() const {
	for (const auto &iter : _neighborNodes) {
		if (iter->getMovementCost() < 0)
			return true;
	}

	return false;
}

bool PathfindingGraphNode::adjacentToNode(PathfindingGraphNode *otherNode) {
	for (uint i = 0; i < _neighborNodes.size(); ++i) {
		if (_neighborNodes[i] == otherNode)
			return true;
	}

	return false;
}

//
// const Common::Array< PathfindingGraphNode*>& PathfindingGraphNode::GetNeighbors() const
//{
//	return neighborNodes;
//}

} // End of namespace Crab
