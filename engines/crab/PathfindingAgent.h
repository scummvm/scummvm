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

#ifndef CRAB_PATHFINDINGAGENT_H
#define CRAB_PATHFINDINGAGENT_H

#include "crab/PathfindingGrid.h"
#include "crab/PriorityQueue.h"
#include "crab/vectors.h"

namespace Crab {

// This class represents the actual pathfinding and following agent that utilizes
// the pathfinding grid
class PlannerNode {
	PlannerNode *_parent;
	PlannerNode *_child;

	double _cost;      // Heuristic cost equivalent to cost to reach goal from planner node's position.
	double _finalCost; // Final cost of route through the planner node. Used to determine optimal path.
	double _givenCost; // The current distance of the route.

public:
	PathfindingGraphNode *_location;

	PlannerNode() {
		_location = NULL;
		_parent = NULL;
		_child = NULL;
		_cost = 0;
		_finalCost = 0;
		_givenCost = 0;
	}
	~PlannerNode() {}

	PathfindingGraphNode *getLocation(void) {
		return _location;
	}

	PlannerNode *getParent(void) {
		return _parent;
	}

	PlannerNode *getChild(void) {
		return _child;
	}

	double getHCost(void) const {
		return _cost;
	}

	double getFinalCost(void) const {
		return _finalCost;
	}

	double getGivenCost(void) const {
		return _givenCost;
	}

	void setLocation(PathfindingGraphNode *loc) {
		_location = loc;
	}

	void setParent(PlannerNode *p) {
		_parent = p;
	}

	void setChild(PlannerNode *c) {
		_child = c;
	}

	void setHCost(double c) {
		_cost = c;
	}

	void setFinalCost(double c) {
		_finalCost = c;
	}

	void setGivenCost(double c) {
		_givenCost = c;
	}
};

class PathfindingAgent {
	Vector2f _position;
	Vector2f _prevPosition;  // Used to determine that we are making progress toward the goal
	Vector2i _immediateDest; // The next stop on the AI's path

	bool _destinationSet;       // Was a destination specified.
	bool _destinationReachable; // Can the agent actually get to the destination?

	float _nodeBufferDistance; // How much leeway is there for reaching the destination

public:
	PathfindingAgent();
	~PathfindingAgent();

	PathfindingGrid *_grid;

	Vector2f _destination;

	PathfindingGraphNode *_startTile;   // The system originally used tiles, but this one uses discreet points.
	PathfindingGraphNode *_goalTile;    // The tile we are trying to reach. May not be the tile that was clicked if the clicked tile is blocked.
	PathfindingGraphNode *_clickedTile; // The tile that was clicked. If it is open, it will be the goal node.

	bool _solutionFound;
	bool _noSolution;

	Common::Array<PathfindingGraphNode const *> _vSolution;

	void setNodeBufferDistance(float w) {
		_nodeBufferDistance = w;
	}

	float getNodeBufferDistance() {
		return _nodeBufferDistance;
	}

	// Added for Greedy search
	double distSquared(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB);
	// Added for A* search
	double distExact(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB);

	PriorityQueue<PlannerNode> _nodeQueue;

	Common::StableMap<PathfindingGraphNode *, PlannerNode *> _createdList;

	// void SetSprite(pyrodactyl::anim::Sprite* s){entitySprite = s;}

	//! \brief Sets the tile map.
	//!
	//! Invoked when the user opens a tile map file.
	//!
	//! \param   _tileMap  the data structure that this algorithm will use
	//!                    to access each tile's location and weight data.
	void initialize(PathfindingGrid *g);

	void setDestination(Vector2f d);
	void setDestination(Vector2f d, bool r);
	void setDestination(Vector2i d);
	void setDestination(Vector2i d, bool r);

	void setPosition(Vector2f p) {
		_position = p;
	}

	void setPrevPosition(Vector2f p) {
		_prevPosition = p;
	}

	Vector2f getPosition() {
		return _position;
	}

	bool positionChanged() {
		return _position != _prevPosition;
	}

	Vector2i getImmediateDest() {
		return _immediateDest;
	}

	//! \brief Performs the main part of the algorithm until the specified time has elapsed or
	//! no nodes are left open.
	void update(long timeslice);

	//! \brief Returns <code>true</code> if and only if no nodes are left open.
	//!
	//! \return  <code>true</code> if no nodes are left open, <code>false</code> otherwise.
	bool isDone() const;

	//! \brief Returns an unmodifiable view of the solution path found by this algorithm.
	Common::Array<PathfindingGraphNode const *> const getSolution(PathfindingGraphNode *destNode) const;

	// Get the solution removing any nodes that are completely surrounded by open space.
	// This will result in a more linear path to the goal.
	Common::Array<PathfindingGraphNode const *> const getPrunedSolution(PathfindingGraphNode *destNode);

	//! \brief Resets the algorithm.
	void reset();

	//! \brief Uninitializes the algorithm before the tile map is unloaded.
	void shutdown();

	// Returns true if the node connects to the goal node
	bool adjacentToGoal(PathfindingGraphNode *node);
};

bool compareNodes(PlannerNode const *nodeA, PlannerNode const *nodeB);

} // End of namespace Crab

#endif // CRAB_PATHFINDINGAGENT_H
