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

#ifndef ULTIMA8_WORLD_ACTORS_PATHFINDER_H
#define ULTIMA8_WORLD_ACTORS_PATHFINDER_H

#include "ultima/shared/std/containers.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/point3.h"
#include "ultima/ultima8/misc/priority_queue.h"
#include "ultima/ultima8/world/actors/animation.h"

//#define DEBUG_PATHFINDER

namespace Ultima {
namespace Ultima8 {

class Actor;
class Item;

struct PathfindingState {
	PathfindingState() : _point(), _direction(dir_north),
		_lastAnim(Animation::walk), _flipped(false),
		_firstStep(true), _combat(false) {};
	Point3 _point;
	Animation::Sequence _lastAnim;
	Direction _direction;
	bool _flipped;
	bool _firstStep;
	bool _combat;

	void load(const Actor *actor);
	bool checkPoint(const Point3 &pt, int range) const;
	bool checkItem(const Item *item, int xyRange, int zRange) const;
	bool checkHit(const Actor *actor, const Item *target) const;
};

struct PathfindingAction {
	Animation::Sequence _action;
	Direction _direction;
	uint32 _steps;
};

struct PathNode;

class PathNodeCmp {
public:
	bool operator()(const PathNode *n1, const PathNode *n2) const;
};

class Pathfinder {
public:
	Pathfinder();
	~Pathfinder();

	void init(Actor *actor, PathfindingState *state = 0);
	void setTarget(const Point3 &pt);
	void setTarget(Item *item, bool hit = false);

	//! try to reach the target by pathfinding
	bool canReach();

	//! pathfind. If true, the found path is returned in path
	bool pathfind(Std::vector<PathfindingAction> &path);

#ifdef DEBUG_PATHFINDER
	static ObjId _visualDebugActor;
#endif


protected:
	PathfindingState _start;
	Actor *_actor;
	Point3 _target;
	Item *_targetItem;
	bool _hitMode;
	int32 _expandTime;

	int32 _actorXd, _actorYd, _actorZd;

	Common::Array<PathfindingState> _visited;
	PriorityQueue<PathNode *, Std::vector<PathNode *>, PathNodeCmp> _nodes;

	/** List of nodes for garbage collection later and order is not important */
	Std::vector<PathNode *> _cleanupNodes;

	bool alreadyVisited(const Point3 &pt) const;
	void newNode(PathNode *oldnode, PathfindingState &state,
				 unsigned int steps);
	void expandNode(PathNode *node);
	unsigned int costHeuristic(PathNode *node) const;
	bool checkTarget(const PathNode *node) const;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
