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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/crab.h"
#include "crab/PathfindingAgent.h"

namespace Crab {

PathfindingAgent::PathfindingAgent(void) : nodeQueue(compareNodes) {
	grid = NULL;

	destinationSet = false;
	destinationReachable = false;
	nodeBufferDistance = 1.0f;
	solutionFound = noSolution = false;

	m_pStartTile = nullptr;
	m_pGoalTile = nullptr;
	m_pClickedTile = nullptr;
}

PathfindingAgent::~PathfindingAgent(void) {
}

void PathfindingAgent::initialize(PathfindingGrid *g) {
	grid = g;

	nodeBufferDistance = grid->GetCellSize().x / 2.0f;

	nodeBufferDistance *= nodeBufferDistance;
}

void PathfindingAgent::SetDestination(Vector2i d, bool r) {
	Vector2f iVec = Vector2f((float)d.x, (float)d.y);

	SetDestination(iVec, r);
}

void PathfindingAgent::SetDestination(Vector2i d) {
	SetDestination(d, true);
}

void PathfindingAgent::SetDestination(Vector2f d) {
	SetDestination(d, true);
}

void PathfindingAgent::SetDestination(Vector2f d, bool r) {
	if (grid == NULL)
		return;

	destination = d;

	// TODO: This could be optimized to cache the route somehow... (SZ)
	reset();

	m_pStartTile = grid->GetNodeAtPoint(position);
	// m_pGoalTile = grid->GetNodeAtPoint(d);

	// I am now tracking the goal node and the clicked tile separately to solve problems
	// with hangups and trying to reach un-reachable destinations.
	m_pClickedTile = grid->GetNodeAtPoint(d);
	m_pGoalTile = grid->GetNearestOpenNode(d, position);

	PlannerNode *startingNode = new PlannerNode();
	startingNode->SetLocation(m_pStartTile);
	startingNode->SetHCost((position - destination).Magnitude());
	startingNode->SetFinalCost((position - destination).Magnitude());
	startingNode->SetGivenCost(0.0);

	nodeQueue.push(startingNode);
	m_mCreatedList[m_pStartTile] = (startingNode);

	destinationSet = true;
	solutionFound = noSolution = false;

	destinationReachable = r;
}

void PathfindingAgent::Update(long timeslice) {

	long prevTime = g_system->getMillis();
	long time = timeslice;

	double dTempCost;

	if (solutionFound) {
		if (m_vSolution.size() > 0) {
			float distSqr = (position - m_vSolution.back()->GetPosition()).MagSqr();
			if (distSqr < nodeBufferDistance) // Have to find the right deadzone buffer
			{
				m_vSolution.pop_back();
			}
		}
		if (m_vSolution.size() > 0) {
			immediateDest = Vector2i(m_vSolution.back()->GetPosition().x, m_vSolution.back()->GetPosition().y);
		} else {
			if (destinationReachable)
				immediateDest = Vector2i((int)destination.x, (int)destination.y);
			else
				immediateDest = Vector2i((int)position.x, (int)position.y);
		}

		return;
	}

	// No nodes, no pathing.
	if (nodeQueue.empty()) {
		return;
	}

	Common::StableMap<PathfindingGraphNode *, PlannerNode *>::iterator currentIter;

	do {
		PlannerNode *current = nodeQueue.front();
		nodeQueue.pop();

		if (current->GetLocation() == m_pGoalTile)
		//|| //We're done.
		{
			// m_vSolution = getSolution();
			m_vSolution = getPrunedSolution(NULL);
			solutionFound = true;
			return;
		} else if (current->GetLocation()->GetMovementCost() > 0 && current->GetLocation()->AdjacentToNode(m_pClickedTile) && m_pClickedTile->GetMovementCost() < 0) {
			m_vSolution = getPrunedSolution(current->GetLocation());
			solutionFound = true;
			return;
		}

		// PathfindingGraphNode* _checkTile;

		// Find the successors
		// for(int x = -1; x < 2; ++x)
		//{
		//	for(int y = -1; y < 2; ++y)
		//	{
		Common::Array<PathfindingGraphNode *>::iterator i;

		for (i = current->location->neighborNodes.begin(); i != current->location->neighborNodes.end(); ++i) {
			// Get the new tile to check
			//_checkTile = m_pTileMap->getTile(current->GetLocation()->getRow() + x, current->GetLocation()->getColumn() + y);

			if ((*i)->GetMovementCost() > 0) {
				// Compute the temp given cost
				dTempCost = current->GetGivenCost() + ((*i)->GetMovementCost() * distExact((*i), current->GetLocation()));

				// If it's a duplicate...
				currentIter = m_mCreatedList.find((*i));
				if (currentIter != m_mCreatedList.end()) {
					if (dTempCost < currentIter->second->GetGivenCost()) {
						// If the current planner node has already been added, but the current path is cheaper,
						// replace it.

						nodeQueue.remove(currentIter->second);

						currentIter->second->SetGivenCost(dTempCost);
						currentIter->second->SetFinalCost(
							currentIter->second->GetHCost() * 1.1 +
							currentIter->second->GetGivenCost());

						currentIter->second->SetParent(current);

						nodeQueue.push(currentIter->second);
					}
				}

				// Otherwise...
				else {
					PlannerNode *successor = new PlannerNode();
					successor->SetLocation((*i));

					// Set the new heuristic (distance from node to the goal)
					successor->SetHCost(distExact((*i), m_pGoalTile));
					successor->SetGivenCost(dTempCost);
					// Final cost is the distance to goal (scaled by 10%) plus the distance of the path.
					successor->SetFinalCost(successor->GetHCost() * 1.1 + successor->GetGivenCost());

					successor->SetParent(current);

					m_mCreatedList[(*i)] = (successor);
					nodeQueue.push(successor); // When the node is pushed onto the PriorityQueue it ends up beings sorted cheapest -> most expensive
				}
			}
		}
		//	}
		//}

		// Update the time
		if (timeslice != 0) {
			time -= (g_system->getMillis() - prevTime);
			prevTime = g_system->getMillis();
		}

	} while (!isDone() && (time >= 0 || timeslice == 0));
	noSolution = true; // You can't get there from here (SZ)
}

bool PathfindingAgent::isDone() const {
	if (nodeQueue.empty())
		return true;

	return false;
}

// Clear everything.
void PathfindingAgent::reset() {
	Common::StableMap<PathfindingGraphNode *, PlannerNode *>::iterator iter;
	if (!m_mCreatedList.empty()) {
		for (iter = m_mCreatedList.begin(); iter != m_mCreatedList.end(); ++iter) {
			delete (*iter).second;
		}
	}

	nodeQueue.clear();
	m_mCreatedList.clear();
	m_vSolution.clear();

	solutionFound = false;

	m_pGoalTile = NULL;
	m_pStartTile = NULL;
}

void PathfindingAgent::shutdown() {
	reset();

	grid = NULL;
}

Common::Array<PathfindingGraphNode const *> const PathfindingAgent::getSolution(PathfindingGraphNode *destNode) const {
	Common::Array<PathfindingGraphNode const *> temp;

	PlannerNode *current = NULL;

	if (m_mCreatedList.find(m_pGoalTile) != m_mCreatedList.end()) {
		current = m_mCreatedList.find(m_pGoalTile)->second;
	}

	// If the dest node passed in is not null, that means we did not reach the goal but came close
	// so we should start with that node instead when we are constructing our path
	else if (destNode != NULL) {
		current = m_mCreatedList.find(destNode)->second;
	}

	// Iterate through the planner nodes to create a vector to return.
	while (current) {
		if (current->GetLocation() != m_pStartTile) {
			// You don't have to path to the start
			if (current->GetLocation() != m_pStartTile)
				temp.push_back(current->GetLocation());
		}

		current = current->GetParent();
	}

	return temp;
}

Common::Array<PathfindingGraphNode const *> const PathfindingAgent::getPrunedSolution(PathfindingGraphNode *destNode) {
	Common::Array<PathfindingGraphNode const *> temp = getSolution(destNode);

	Common::Array<PathfindingGraphNode const *> returnVec = temp;

	// Any node that is not adjacent to an obstacle or an obstacle corner can be removed.
	for (int i = 0; (unsigned int)i < temp.size(); ++i) {
		if (!temp[i]->AdjacentToObstacle()) {
			if (i > 0 && (unsigned int)i < temp.size() - 1) {
				// This check to see if the node is a "corner" to an obstacle that should not be pruned
				// to prevent hanging on corners.
				Common::Array<PathfindingGraphNode *> corners = grid->CornerCheck(temp[i - 1], temp[i + 1]);

				if (corners.size() == 0) {
					Common::Array<PathfindingGraphNode const *>::iterator theEnd = Common::remove(returnVec.begin(), returnVec.end(), temp[i]);
					returnVec.erase(theEnd);
				}
			}
		}
	}

	return returnVec;
}

double PathfindingAgent::distSquared(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB) {
	Vector2f vecTo = tileA->GetPosition() - tileB->GetPosition();

	return vecTo.MagSqr();
}

double PathfindingAgent::distExact(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB) {
	Vector2f vecTo = tileA->GetPosition() - tileB->GetPosition();

	return vecTo.Magnitude();
}

// This keeps the PriorityQueue organized based on the cost of the paths.
bool compareNodes(PlannerNode const *nodeA, PlannerNode const *nodeB) {
	return nodeA->GetFinalCost() > nodeB->GetFinalCost();
}

bool PathfindingAgent::AdjacentToGoal(PathfindingGraphNode *node) {
	for (auto iter : node->neighborNodes) {
		if (iter == m_pGoalTile) {
			return true;
		}
	}

	return false;
}

} // End of namespace Crab
