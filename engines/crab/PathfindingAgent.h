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

#include "PathfindingGrid.h"
#include "PriorityQueue.h"
#include "vectors.h"

// This class represents the actual pathfinding and following agent that utilizes
// the pathfinding grid
class PlannerNode {
	PlannerNode *parent;
	PlannerNode *child;

	double cost;      // Heuristic cost equivalent to cost to reach goal from planner node's position.
	double finalCost; // Final cost of route through the planner node. Used to determine optimal path.
	double givenCost; // The current distance of the route.

public:
	PathfindingGraphNode *location;

	PlannerNode() {
		location = NULL;
		parent = NULL;
		child = NULL;
		cost = 0;
		finalCost = 0;
		givenCost = 0;
	}
	~PlannerNode() {}

	PathfindingGraphNode *GetLocation(void) { return location; }
	PlannerNode *GetParent(void) { return parent; }
	PlannerNode *GetChild(void) { return child; }
	double GetHCost(void) const { return cost; }
	double GetFinalCost(void) const { return finalCost; }
	double GetGivenCost(void) const { return givenCost; }

	void SetLocation(PathfindingGraphNode *loc) { location = loc; }
	void SetParent(PlannerNode *p) { parent = p; }
	void SetChild(PlannerNode *c) { child = c; }
	void SetHCost(double c) { cost = c; }
	void SetFinalCost(double cost) { finalCost = cost; }
	void SetGivenCost(double cost) { givenCost = cost; }
};

class PathfindingAgent {
	Vector2f position;
	Vector2f prevPosition;  // Used to determine that we are making progress toward the goal
	Vector2i immediateDest; // The next stop on the AI's path

	bool destinationSet;       // Was a destination specified.
	bool destinationReachable; // Can the agent actually get to the destination?

	float nodeBufferDistance; // How much leeway is there for reaching the destination

public:
	PathfindingAgent();
	~PathfindingAgent();

	PathfindingGrid *grid;

	Vector2f destination;

	PathfindingGraphNode *m_pStartTile;   // The system originally used tiles, but this one uses discreet points.
	PathfindingGraphNode *m_pGoalTile;    // The tile we are trying to reach. May not be the tile that was clicked if the clicked tile is blocked.
	PathfindingGraphNode *m_pClickedTile; // The tile that was clicked. If it is open, it will be the goal node.

	bool solutionFound;
	bool noSolution;

	std::vector<PathfindingGraphNode const *> m_vSolution;

	void SetNodeBufferDistance(float w) { nodeBufferDistance = w; }
	float GetNodeBufferDistance() { return nodeBufferDistance; }

	// Added for Greedy search
	double distSquared(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB);
	// Added for A* search
	double distExact(PathfindingGraphNode *tileA, PathfindingGraphNode *tileB);

	PriorityQueue<PlannerNode> nodeQueue;

	std::map<PathfindingGraphNode *, PlannerNode *> m_mCreatedList;

	// void SetSprite(pyrodactyl::anim::Sprite* s){entitySprite = s;}

	//! \brief Sets the tile map.
	//!
	//! Invoked when the user opens a tile map file.
	//!
	//! \param   _tileMap  the data structure that this algorithm will use
	//!                    to access each tile's location and weight data.
	void initialize(PathfindingGrid *g);

	void SetDestination(Vector2f d);
	void SetDestination(Vector2f d, bool r);
	void SetDestination(Vector2i d);
	void SetDestination(Vector2i d, bool r);

	void SetPosition(Vector2f p) { position = p; }
	void SetPrevPosition(Vector2f p) { prevPosition = p; }
	Vector2f GetPosition() { return position; }

	bool PositionChanged() { return position != prevPosition; }

	Vector2i GetImmediateDest() { return immediateDest; }

	//! \brief Performs the main part of the algorithm until the specified time has elapsed or
	//! no nodes are left open.
	void Update(long timeslice);

	//! \brief Returns <code>true</code> if and only if no nodes are left open.
	//!
	//! \return  <code>true</code> if no nodes are left open, <code>false</code> otherwise.
	bool isDone() const;

	//! \brief Returns an unmodifiable view of the solution path found by this algorithm.
	std::vector<PathfindingGraphNode const *> const getSolution(PathfindingGraphNode *destNode) const;

	// Get the solution removing any nodes that are completely surrounded by open space.
	// This will result in a more linear path to the goal.
	std::vector<PathfindingGraphNode const *> const getPrunedSolution(PathfindingGraphNode *destNode);

	//! \brief Resets the algorithm.
	void exit();

	//! \brief Uninitializes the algorithm before the tile map is unloaded.
	void shutdown();

	// Returns true if the node connects to the goal node
	bool AdjacentToGoal(PathfindingGraphNode *node);
};

bool compareNodes(PlannerNode const *nodeA, PlannerNode const *nodeB);

#endif // CRAB_PATHFINDINGAGENT_H
