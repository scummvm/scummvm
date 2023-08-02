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

#include "crab/PathfindingGrid.h"
#include "crab/TMX/TMXMap.h"

namespace Crab {

using namespace TMX;

PathfindingGrid::PathfindingGrid(void) {
	_blockedCost = BLOCKED;
	_openCost = OPEN;
	_stairsCost = STAIRS;
	_nodes = nullptr;

	_dimensions.x = 0;
	_dimensions.y = 0;

	_cellSize.x = 0.0;
	_cellSize.y = 0.0;
}

PathfindingGrid::~PathfindingGrid(void) {
	reset();
}

void PathfindingGrid::reset() {
	for (int x = 0; x < _dimensions.x; ++x) {
		delete[] _nodes[x];
	}

	delete[] _nodes;
	_nodes = nullptr;

	_dimensions.x = 0;
	_dimensions.y = 0;

	_cellSize.x = 0.0;
	_cellSize.y = 0.0;
}

void PathfindingGrid::setupNodes(TMX::TMXMap map) {
	// delete nodes if they exist
	reset();

	_dimensions.x = map._pathRows; // Logically, this is incorrect but it matches the format of cols and rows used elsewhere (SZ)
	_dimensions.y = map._pathCols;

	_cellSize.x = (float)map._pathSize.x;
	_cellSize.y = (float)map._pathSize.y;

	// Check to see if the costs have been loaded from the level file.
	// If not, assign to defaults.
	if (map._movementCosts._noWalk != 0) {
		_blockedCost = map._movementCosts._noWalk;
	}
	if (map._movementCosts._open != 0) {
		_openCost = map._movementCosts._open;
	}
	if (map._movementCosts._stairs != 0) {
		_stairsCost = map._movementCosts._stairs;
	}

	_nodes = new PathfindingGraphNode *[_dimensions.x];

	// Allocate some nodes!
	// TODO: probably want to change this to a one chunk allocation...
	for (int i = 0; i < _dimensions.x; ++i) {
		_nodes[i] = new PathfindingGraphNode[_dimensions.y];
	}

	// Fill up those nodes!
	int idCounter = 0;

	Vector2f pos = Vector2f(0.0f, 0.0f);
	Vector2f topLeftPos = pos;

	// Initialize the nodes
	for (int x = 0; x < _dimensions.x; ++x) {
		for (int y = 0; y < _dimensions.y; ++y) {
			// PathfindingGraphNode* newNode = new PathfindingGraphNode(pos, idCounter++);

			// nodes[x][y] = *newNode;
			_nodes[x][y]._collisionRect = Rect(pos.x, pos.y, _cellSize.x, _cellSize.y);

			_nodes[x][y]._position.x = pos.x + _cellSize.x / 2.0f;
			_nodes[x][y]._position.y = pos.y + _cellSize.y / 2.0f;
			_nodes[x][y]._id = idCounter++;

			_nodes[x][y]._movementCost = _openCost;
			_nodes[x][y]._neighborCosts.reserve(4); // since its a square based grid, 4 is the greatest number of costs and nodes possible.
			_nodes[x][y]._neighborNodes.reserve(4);

			pos.y += _cellSize.y;

			Common::Array<Shape> noWalk = map.areaNoWalk();

			// Check if the square should count as blocked
			for (auto i = noWalk.begin(); i != noWalk.end(); ++i) {
				if (i->collide(_nodes[x][y]._collisionRect)._intersect) {
					_nodes[x][y]._movementCost = (float)_blockedCost;
					break;
				}
			}

			// Check for stairs if the cell isn't blocked
			if (_nodes[x][y]._movementCost >= 0.0f) {
				Common::Array<pyrodactyl::level::Stairs> stairs = map.areaStairs();

				for (auto i = stairs.begin(); i != stairs.end(); ++i) {
					if (i->collide(_nodes[x][y]._collisionRect)._intersect) {
						_nodes[x][y]._movementCost = (float)_stairsCost;
						break;
					}
				}
			}

			// More collision checks can be added for the node as long as it checks for the high cost objects first
			// since the highest cost collider in any given tile would be used for the path cost. (SZ)
		}

		pos.x += _cellSize.x;
		pos.y = topLeftPos.y;
	}

	// Connect the nodes
	for (int x = 0; x < _dimensions.x; ++x) {
		for (int y = 0; y < _dimensions.y; ++y) {
			// Check horizontal
			if (x < _dimensions.x - 1) {
				connectNodes(&_nodes[x][y], &_nodes[x + 1][y]);

				// Check diagonals
				// This causes hangups since the collider has a greater width to take into account when traveling
				// diagonally compared to horizontal or vertical. (SZ)
				/*if( y < dimensions.y - 2)
				{
				ConnectNodes(&nodes[x][y], &nodes[x + 1][y + 1]);

				nodes[x][y].neighborCosts[nodes[x][y].neighborCosts.size() - 1] *= 1.41f;
				nodes[x + 1][y + 1].movementCost *= 1.41f;
				}

				if(y > 0)
				{
				ConnectNodes(&nodes[x][y], &nodes[x + 1][y - 1]);

				nodes[x][y].neighborCosts[nodes[x][y].neighborCosts.size() - 1] *= 1.41f;
				nodes[x + 1][y - 1].movementCost *= 1.41f;
				}*/
			}
			// Check vertical
			if (y < _dimensions.y - 1) {
				connectNodes(&_nodes[x][y], &_nodes[x][y + 1]);
			}
		}
	}

	////Check for adjacencies
	////This could be used if additional weight should be applied to nodes adjacent to blocked nodes.
	// for(int x = 0; x < dimensions.x; ++x)
	//{
	//	for(int y = 0; y < dimensions.y; ++y)
	//	{
	//		for(int i = 0; i < nodes[x][y].neighborNodes.size(); ++i)
	//		{
	//			if(nodes[x][y].neighborNodes[i]->movementCost == blockedCost)
	//			{
	//				nodes[x][y].movementCost *= 2.0f;
	//				break;
	//			}
	//		}
	//	}
	// }
}

void PathfindingGrid::connectNodes(PathfindingGraphNode *node1, PathfindingGraphNode *node2) {
	node1->addNeighbor(node2, true);
	node2->addNeighbor(node1, true);
}

PathfindingGraphNode *PathfindingGrid::getNodeAtPoint(Vector2f point) {
	int x = (int)floor(point.x / _cellSize.x);
	int y = (int)floor(point.y / _cellSize.y);

	return &_nodes[x][y];
}

Common::Array<PathfindingGraphNode *> PathfindingGrid::cornerCheck(const PathfindingGraphNode *node1, const PathfindingGraphNode *node2) {
	Common::Array<PathfindingGraphNode *> returnNodes;

	// Iterat through both nodes neighbors. If a blocked neighbor is found that is shared between the two,
	// It is a corner to them.
	for (auto iter : node1->_neighborNodes) {
		for (auto iter2 : node2->_neighborNodes) {
			if (iter == iter2 && iter->_movementCost < 0) {
				if (returnNodes.size() == 0 || (*(Common::find(returnNodes.begin(), returnNodes.end(), iter))) == nullptr)
					returnNodes.push_back(iter);
			}
		}
	}

	return returnNodes;
}

PathfindingGraphNode *PathfindingGrid::getNearestOpenNode(Vector2f nodePos, Vector2f comparePos) {
	PathfindingGraphNode *startNode = getNodeAtPoint(nodePos);

	if (startNode->getMovementCost() > 0) // If the clicked node is open, we're done!
		return startNode;

	PathfindingGraphNode *returnNode = nullptr;

	float shortestDistance = 0.0f;

	Common::List<PathfindingGraphNode *> checkNodes;
	checkNodes.push_back(startNode);

	Common::Array<PathfindingGraphNode *> allUsedNodes;
	allUsedNodes.push_back(startNode);

	// Iterate through the nodes, check if they are open then check their distance from the compare point.
	while (!checkNodes.empty()) {
		if (checkNodes.front()->getMovementCost() > 0) {
			float distance = (comparePos - checkNodes.front()->getPosition()).MagSqr();

			if (shortestDistance == 0.0f || distance) // If this is the new shortest distance, this becomes the new return.
			{
				shortestDistance = distance;

				returnNode = checkNodes.front();
			}
		} else {
			for (unsigned int i = 0; i < checkNodes.front()->_neighborNodes.size(); ++i) {
				// If the neighbor hasn't been checked yet, add it to the list to check.
				if (Common::find(allUsedNodes.begin(), allUsedNodes.end(), checkNodes.front()->_neighborNodes[i]) == allUsedNodes.end()) {
					allUsedNodes.push_back(checkNodes.front()->_neighborNodes[i]);
					checkNodes.push_back(checkNodes.front()->_neighborNodes[i]);
				}
			}
		}

		if (returnNode != nullptr) // If a node has been found, we are done. We don't want to continue iterating through neighbors since it would take us further from the clicked node.
			return returnNode;

		checkNodes.pop_front();
	}

	return NULL;
}

} // End of namespace Crab
