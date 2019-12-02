/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_WORLD_ACTORS_PATHFINDER_H
#define ULTIMA8_WORLD_ACTORS_PATHFINDER_H

#include "ultima8/std/containers.h"
#include "ultima8/std/containers.h"
#include "ultima8/world/actors/animation.h"

namespace Ultima8 {

class Actor;
class Item;

struct PathfindingState {
	int32 x, y, z;
	Animation::Sequence lastanim;
	uint32 direction;
	bool flipped;
	bool firststep;
	bool combat;

	void load(Actor *actor);
	bool checkPoint(int32 x_, int32 y_, int32 z_, int range);
	bool checkItem(Item *item, int xyRange, int zRange);
	bool checkHit(Actor *actor, Actor *target);
};

struct PathfindingAction {
	Animation::Sequence action;
	uint32 direction;
	uint32 steps;
};

struct PathNode;

class PathNodeCmp {
public:
	bool operator()(PathNode *n1, PathNode *n2);
};

class Pathfinder {
public:
	Pathfinder();
	~Pathfinder();

	void init(Actor *actor, PathfindingState *state = 0);
	void setTarget(int32 x, int32 y, int32 z);
	void setTarget(Item *item, bool hit = false);

	//! try to reach the target by pathfinding
	bool canReach();

	//! pathfind. If true, the found path is returned in path
	bool pathfind(std::vector<PathfindingAction> &path);

#ifdef DEBUG
	//! "visualDebug" console command
	static void ConCmd_visualDebug(const Console::ArgvType &argv);
	static ObjId visualdebug_actor;
#endif


protected:
	PathfindingState start;
	Actor *actor;
	int32 targetx, targety, targetz;
	Item *targetitem;
	bool hitmode;
	int32 expandtime;

	int32 actor_xd, actor_yd, actor_zd;

	std::list<PathfindingState> visited;
	std::priority_queue<PathNode *, std::vector<PathNode *>, PathNodeCmp> nodes;

	std::list<PathNode *> nodelist;

	bool alreadyVisited(int32 x, int32 y, int32 z);
	void newNode(PathNode *oldnode, PathfindingState &state, unsigned int steps);
	void expandNode(PathNode *node);
	unsigned int costHeuristic(PathNode *node);
	bool checkTarget(PathNode *node);
};

} // End of namespace Ultima8

#endif
