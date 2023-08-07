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

#include "crab/crab.h"
#include "crab/PathfindingAgent.h"

namespace Crab {

// This keeps the PriorityQueue organized based on the cost of the paths.
static bool compareNodes(PlannerNode const *nodeA, PlannerNode const *nodeB) {
	return nodeA->getFinalCost() > nodeB->getFinalCost();
}

PathfindingAgent::PathfindingAgent() : _nodeQueue(compareNodes) {
	_grid = nullptr;

	_destinationSet = false;
	_destinationReachable = false;
	_nodeBufferDistance = 1.0f;
	_solutionFound = _noSolution = false;

	_startTile = nullptr;
	_goalTile = nullptr;
	_clickedTile = nullptr;
}

PathfindingAgent::~PathfindingAgent() {
}

void PathfindingAgent::initialize(PathfindingGrid *g) {
	_grid = g;

	_nodeBufferDistance = _grid->getCellSize().x / 2.0f;

	_nodeBufferDistance *= _nodeBufferDistance;
}

void PathfindingAgent::setDestination(Vector2i d, bool r) {
	Vector2f iVec = Vector2f((float)d.x, (float)d.y);

	setDestination(iVec, r);
}

void PathfindingAgent::setDestination(Vector2i d) {
	setDestination(d, true);
}

void PathfindingAgent::setDestination(Vector2f d) {
	setDestination(d, true);
}

void PathfindingAgent::setDestination(Vector2f d, bool r) {
	if (_grid == nullptr)
		return;

	_destination = d;

	// TODO: This could be optimized to cache the route somehow... (SZ)
	reset();

	_startTile = _grid->getNodeAtPoint(_position);
	// m_pGoalTile = grid->GetNodeAtPoint(d);

	// I am now tracking the goal node and the clicked tile separately to solve problems
	// with hangups and trying to reach un-reachable destinations.
	_clickedTile = _grid->getNodeAtPoint(d);
	_goalTile = _grid->getNearestOpenNode(d, _position);

	PlannerNode *startingNode = new PlannerNode();
	startingNode->setLocation(_startTile);
	startingNode->setHCost((_position - _destination).magnitude());
	startingNode->setFinalCost((_position - _destination).magnitude());
	startingNode->setGivenCost(0.0);

	_nodeQueue.push(startingNode);
	_createdList[_startTile] = (startingNode);

	_destinationSet = true;
	_solutionFound = _noSolution = false;

	_destinationReachable = r;
}

void PathfindingAgent::update(uint32 timeslice) {

	uint32 prevTime = g_system->getMillis();
	uint32 timeLeft = timeslice;

	double dTempCost;

	if (_solutionFound) {
		if (_vSolution.size() > 0) {
			float distSqr = (_position - _vSolution.back()->getPosition()).magSqr();
			if (distSqr < _nodeBufferDistance) { // Have to find the right deadzone buffer
				_vSolution.pop_back();
			}
		}
		if (_vSolution.size() > 0) {
			_immediateDest = Vector2i(_vSolution.back()->getPosition().x, _vSolution.back()->getPosition().y);
		} else {
			if (_destinationReachable)
				_immediateDest = Vector2i((int)_destination.x, (int)_destination.y);
			else
				_immediateDest = Vector2i((int)_position.x, (int)_position.y);
		}

		return;
	}

	// No nodes, no pathing.
	if (_nodeQueue.empty()) {
		return;
	}

	Common::StableMap<PathfindingGraphNode *, PlannerNode *>::iterator currentIter;

	do {
		PlannerNode *current = _nodeQueue.front();
		_nodeQueue.pop();

		if (current->getLocation() == _goalTile) { // We're done.
			// m_vSolution = getSolution();
			_vSolution = getPrunedSolution(NULL);
			_solutionFound = true;
			return;
		} else if (current->getLocation()->getMovementCost() > 0 && current->getLocation()->adjacentToNode(_clickedTile) && _clickedTile->getMovementCost() < 0) {
			_vSolution = getPrunedSolution(current->getLocation());
			_solutionFound = true;
			return;
		}

		for (auto &i : current->_location->_neighborNodes) {
			if (i->getMovementCost() > 0) {
				// Compute the temp given cost
				dTempCost = current->getGivenCost() + i->getMovementCost() * distExact(i, current->getLocation());

				// If it's a duplicate...
				currentIter = _createdList.find(i);
				if (currentIter != _createdList.end()) {
					if (dTempCost < currentIter->second->getGivenCost()) {
						// If the current planner node has already been added, but the current path is cheaper,
						// replace it.

						_nodeQueue.remove(currentIter->second);

						currentIter->second->setGivenCost(dTempCost);
						currentIter->second->setFinalCost(
							currentIter->second->getHCost() * 1.1 +
							currentIter->second->getGivenCost());

						currentIter->second->setParent(current);

						_nodeQueue.push(currentIter->second);
					}
				} else { // Otherwise...
					PlannerNode *successor = new PlannerNode();
					successor->setLocation(i);

					// Set the new heuristic (distance from node to the goal)
					successor->setHCost(distExact(i, _goalTile));
					successor->setGivenCost(dTempCost);
					// Final cost is the distance to goal (scaled by 10%) plus the distance of the path.
					successor->setFinalCost(successor->getHCost() * 1.1 + successor->getGivenCost());

					successor->setParent(current);

					_createdList[i] = (successor);
					_nodeQueue.push(successor); // When the node is pushed onto the PriorityQueue it ends up beings sorted cheapest -> most expensive
				}
			}
		}

		// Update the time
		if (timeslice != 0) {
			timeLeft -= (g_system->getMillis() - prevTime);
			prevTime = g_system->getMillis();
		}

	} while (!isDone() && ((int32)timeLeft >= 0 || timeslice == 0));
	_noSolution = true; // You can't get there from here (SZ)
}

bool PathfindingAgent::isDone() const {
	if (_nodeQueue.empty())
		return true;

	return false;
}

// Clear everything.
void PathfindingAgent::reset() {
	for (auto &iter : _createdList)
		delete iter.second;

	_nodeQueue.clear();
	_createdList.clear();
	_vSolution.clear();

	_solutionFound = false;

	_goalTile = nullptr;
	_startTile = nullptr;
}

void PathfindingAgent::shutdown() {
	reset();

	_grid = nullptr;
}

Common::Array<PathfindingGraphNode const *> const PathfindingAgent::getSolution(PathfindingGraphNode *destNode) const {
	Common::Array<PathfindingGraphNode const *> temp;

	PlannerNode *current = nullptr;

	if (_createdList.find(_goalTile) != _createdList.end()) {
		current = _createdList.find(_goalTile)->second;
	} else if (destNode != nullptr) {
		// If the dest node passed in is not null, that means we did not reach the goal but came close
		// so we should start with that node instead when we are constructing our path
		current = _createdList.find(destNode)->second;
	}

	// Iterate through the planner nodes to create a vector to return.
	while (current) {
		if (current->getLocation() != _startTile) {
			// You don't have to path to the start
			if (current->getLocation() != _startTile)
				temp.push_back(current->getLocation());
		}

		current = current->getParent();
	}

	return temp;
}

Common::Array<PathfindingGraphNode const *> const PathfindingAgent::getPrunedSolution(PathfindingGraphNode *destNode) {
	Common::Array<PathfindingGraphNode const *> temp = getSolution(destNode);

	Common::Array<PathfindingGraphNode const *> returnVec = temp;

	// Any node that is not adjacent to an obstacle or an obstacle corner can be removed.
	for (int i = 0; (uint)i < temp.size(); ++i) {
		if (!temp[i]->adjacentToObstacle()) {
			if (i > 0 && (uint)i < temp.size() - 1) {
				// This check to see if the node is a "corner" to an obstacle that should not be pruned
				// to prevent hanging on corners.
				Common::Array<PathfindingGraphNode *> corners = _grid->cornerCheck(temp[i - 1], temp[i + 1]);

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
	Vector2f vecTo = tileA->getPosition() - tileB->getPosition();

	return vecTo.magSqr();
}

double PathfindingAgent::distExact(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB) {
	Vector2f vecTo = tileA->getPosition() - tileB->getPosition();

	return vecTo.magnitude();
}

bool PathfindingAgent::adjacentToGoal(PathfindingGraphNode *node) {
	for (const auto &iter : node->_neighborNodes) {
		if (iter == _goalTile) {
			return true;
		}
	}

	return false;
}

} // End of namespace Crab
