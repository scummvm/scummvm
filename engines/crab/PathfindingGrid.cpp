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

#include "PathfindingGrid.h"
#include "TMXMap.h"
#include "stdafx.h"

using namespace TMX;

PathfindingGrid::PathfindingGrid(void) {
	blockedCost = BLOCKED;
	openCost = OPEN;
	stairsCost = STAIRS;
	nodes = nullptr;
}

PathfindingGrid::~PathfindingGrid(void) {
	for (int x = 0; x < dimensions.x; ++x) {
		delete[] nodes[x];
	}

	delete nodes;
}

void PathfindingGrid::SetupNodes(TMXMap map) {
	dimensions.x = map.path_rows; // Logically, this is incorrect but it matches the format of cols and rows used elsewhere (SZ)
	dimensions.y = map.path_cols;

	cellSize.x = (float)map.path_size.x;
	cellSize.y = (float)map.path_size.y;

	// Check to see if the costs have been loaded from the level file.
	// If not, assign to defaults.
	if (map.movementCosts.no_walk != 0) {
		blockedCost = map.movementCosts.no_walk;
	}
	if (map.movementCosts.open != 0) {
		openCost = map.movementCosts.open;
	}
	if (map.movementCosts.stairs != 0) {
		stairsCost = map.movementCosts.stairs;
	}

	nodes = new PathfindingGraphNode *[dimensions.x];

	// Allocate some nodes!
	// TODO: probably want to change this to a one chunk allocation...
	for (int i = 0; i < dimensions.x; ++i) {
		nodes[i] = new PathfindingGraphNode[dimensions.y];
	}

	// Fill up those nodes!
	int idCounter = 0;

	Vector2f pos = Vector2f(0.0f, 0.0f);
	Vector2f topLeftPos = pos;

	// Initialize the nodes
	for (int x = 0; x < dimensions.x; ++x) {
		for (int y = 0; y < dimensions.y; ++y) {
			// PathfindingGraphNode* newNode = new PathfindingGraphNode(pos, idCounter++);

			// nodes[x][y] = *newNode;
			nodes[x][y].collisionRect = Rect(pos.x, pos.y, cellSize.x, cellSize.y);

			nodes[x][y].position.x = pos.x + cellSize.x / 2.0f;
			nodes[x][y].position.y = pos.y + cellSize.y / 2.0f;
			nodes[x][y].id = idCounter++;

			nodes[x][y].movementCost = openCost;
			nodes[x][y].neighborCosts.reserve(4); // since its a square based grid, 4 is the greatest number of costs and nodes possible.
			nodes[x][y].neighborNodes.reserve(4);

			pos.y += cellSize.y;

			std::vector<Shape> noWalk = map.AreaNoWalk();

			// Check if the square should count as blocked
			for (auto i = noWalk.begin(); i != noWalk.end(); ++i) {
				if (i->Collide(nodes[x][y].collisionRect).intersect) {
					nodes[x][y].movementCost = (float)blockedCost;
					break;
				}
			}

			// Check for stairs if the cell isn't blocked
			if (nodes[x][y].movementCost >= 0.0f) {
				std::vector<pyrodactyl::level::Stairs> stairs = map.AreaStairs();

				for (auto i = stairs.begin(); i != stairs.end(); ++i) {
					if (i->Collide(nodes[x][y].collisionRect).intersect) {
						nodes[x][y].movementCost = (float)stairsCost;
						break;
					}
				}
			}

			// More collision checks can be added for the node as long as it checks for the high cost objects first
			// since the highest cost collider in any given tile would be used for the path cost. (SZ)
		}

		pos.x += cellSize.x;
		pos.y = topLeftPos.y;
	}

	// Connect the nodes
	for (int x = 0; x < dimensions.x; ++x) {
		for (int y = 0; y < dimensions.y; ++y) {
			// Check horizontal
			if (x < dimensions.x - 1) {
				ConnectNodes(&nodes[x][y], &nodes[x + 1][y]);

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
			if (y < dimensions.y - 1) {
				ConnectNodes(&nodes[x][y], &nodes[x][y + 1]);
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

void PathfindingGrid::ConnectNodes(PathfindingGraphNode *node1, PathfindingGraphNode *node2) {
	node1->AddNeighbor(node2, true);
	node2->AddNeighbor(node1, true);
}

PathfindingGraphNode *PathfindingGrid::GetNodeAtPoint(Vector2f point) {
	int x = (int)floor(point.x / cellSize.x);
	int y = (int)floor(point.y / cellSize.y);

	return &nodes[x][y];
}

std::vector<PathfindingGraphNode *> PathfindingGrid::CornerCheck(const PathfindingGraphNode *node1, const PathfindingGraphNode *node2) {
	std::vector<PathfindingGraphNode *> returnNodes;

	// Iterat through both nodes neighbors. If a blocked neighbor is found that is shared between the two,
	// It is a corner to them.
	for (auto iter : node1->neighborNodes) {
		for (auto iter2 : node2->neighborNodes) {
			if (iter == iter2 && iter->movementCost < 0) {
				if (returnNodes.size() == 0 || (*(std::find(returnNodes.begin(), returnNodes.end(), iter))) == NULL)
					returnNodes.push_back(iter);
			}
		}
	}

	return returnNodes;
}

PathfindingGraphNode *PathfindingGrid::GetNearestOpenNode(Vector2f nodePos, Vector2f comparePos) {
	PathfindingGraphNode *startNode = GetNodeAtPoint(nodePos);

	if (startNode->GetMovementCost() > 0) // If the clicked node is open, we're done!
		return startNode;

	PathfindingGraphNode *returnNode = NULL;

	float shortestDistance = 0.0f;

	std::list<PathfindingGraphNode *> checkNodes;
	checkNodes.push_back(startNode);

	std::vector<PathfindingGraphNode *> allUsedNodes;
	allUsedNodes.push_back(startNode);

	// Iterate through the nodes, check if they are open then check their distance from the compare point.
	while (!checkNodes.empty()) {
		if (checkNodes.front()->GetMovementCost() > 0) {
			float distance = (comparePos - checkNodes.front()->GetPosition()).MagSqr();

			if (shortestDistance == 0.0f || distance) // If this is the new shortest distance, this becomes the new return.
			{
				shortestDistance = distance;

				returnNode = checkNodes.front();
			}
		} else {
			for (int i = 0; i < checkNodes.front()->neighborNodes.size(); ++i) {
				// If the neighbor hasn't been checked yet, add it to the list to check.
				if (std::find(allUsedNodes.begin(), allUsedNodes.end(), checkNodes.front()->neighborNodes[i]) == allUsedNodes.end()) {
					allUsedNodes.push_back(checkNodes.front()->neighborNodes[i]);
					checkNodes.push_back(checkNodes.front()->neighborNodes[i]);
				}
			}
		}

		if (returnNode != NULL) // If a node has been found, we are done. We don't want to continue iterating through neighbors since it would take us further from the clicked node.
			return returnNode;

		checkNodes.pop_front();
	}

	return NULL;
}
