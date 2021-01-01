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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/ultima8.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima8 {


#ifdef DEBUG
ObjId Pathfinder::_visualDebugActor = 0xFFFF;
#endif

struct PathNode {
	PathfindingState state;
	unsigned int depth;
	unsigned int cost;
	unsigned int heuristicTotalCost;
	PathNode *parent;
	uint32 stepsfromparent;
};

// NOTE: this is just to keep some statistics
static unsigned int expandednodes = 0;

void PathfindingState::load(const Actor *_actor) {
	_actor->getLocation(_x, _y, _z);
	_lastAnim = _actor->getLastAnim();
	_direction = _actor->getDir();
	_firstStep = _actor->hasActorFlags(Actor::ACT_FIRSTSTEP);
	_flipped = _actor->hasFlags(Item::FLG_FLIPPED);
	_combat = _actor->isInCombat();
}

bool PathfindingState::checkPoint(int32 x, int32 y, int32 z,
                                  int sqr_range) const {
	int distance = (_x - x) * (_x - x) + (_y - y) * (_y - y) + (_z - z) * (_z - z);
	return distance < sqr_range;
}

bool PathfindingState::checkItem(const Item *item, int xyRange, int zRange) const {
	int32 itemX, itemY, itemZ;
	int32 itemXd, itemYd, itemZd;
	int32 itemXmin, itemYmin;

	item->getLocationAbsolute(itemX, itemY, itemZ);
	item->getFootpadWorld(itemXd, itemYd, itemZd);

	itemXmin = itemX - itemXd;
	itemYmin = itemY - itemYd;

	int range = 0;
	if (_x - itemX > range)
		range = _x - itemX;
	if (itemXmin - _x > range)
		range = itemXmin - _x;
	if (_y - itemY > range)
		range = _y - itemY;
	if (itemYmin - _y > range)
		range = itemYmin - _y;

	// FIXME: check _z properly

	return (range <= xyRange);
}

bool PathfindingState::checkHit(const Actor *_actor, const Actor *target) const {
	assert(target);
#if 0
	pout << "Trying hit in _direction " << _actor->getDirToItemCentre(*target) << Std::endl;
#endif
	AnimationTracker tracker;
	if (!tracker.init(_actor, Animation::attack,
	                  _actor->getDirToItemCentre(*target), this)) {
		return false;
	}

	while (tracker.step()) {
		if (tracker.hitSomething()) break;
	}

	ObjId hit = tracker.hitSomething();
	if (hit == target->getObjId()) return true;

	return false;
}

bool PathNodeCmp::operator()(const PathNode *n1, const PathNode *n2) const {
	return (n1->heuristicTotalCost > n2->heuristicTotalCost);
}

Pathfinder::Pathfinder() : _actor(nullptr), _targetItem(nullptr),
		_hitMode(false), _expandTime(0), _targetX(0), _targetY(0),
		_targetZ(0), _actorXd(0), _actorYd(0), _actorZd(0) {
	expandednodes = 0;
	_visited.reserve(1500);
}

Pathfinder::~Pathfinder() {
#if 1
	pout << "~Pathfinder: " << _cleanupNodes.size() << " nodes to clean up, visited "
		 << _visited.size() << " and "
	     << expandednodes << " expanded nodes in " << _expandTime << "ms." << Std::endl;
#endif

	// clean up _nodes
	Std::vector<PathNode *>::iterator iter;
	for (iter = _cleanupNodes.begin(); iter != _cleanupNodes.end(); ++iter)
		delete *iter;
	_cleanupNodes.clear();
}

void Pathfinder::init(Actor *actor, PathfindingState *state) {
	_actor = actor;

	_actor->getFootpadWorld(_actorXd, _actorYd, _actorZd);

	if (state)
		_start = *state;
	else
		_start.load(_actor);
}

void Pathfinder::setTarget(int32 x, int32 y, int32 z) {
	_targetX = x;
	_targetY = y;
	_targetZ = z;
	_targetItem = 0;
	_hitMode = false;
}

void Pathfinder::setTarget(Item *item, bool hit) {
	_targetItem = item;
	while (_targetItem->getParentAsContainer())
		_targetItem = _targetItem->getParentAsContainer();

	// set target to centre of item for the cost heuristic
	item->getCentre(_targetX, _targetY, _targetZ);
	_targetZ = item->getZ();

	if (hit) {
		assert(_start._combat);
		assert(dynamic_cast<Actor *>(_targetItem));
		_hitMode = true;
	} else {
		_hitMode = false;
	}
}

bool Pathfinder::canReach() {
	Std::vector<PathfindingAction> path;
	return pathfind(path);
}

bool Pathfinder::alreadyVisited(int32 x, int32 y, int32 z) const {
	//
	// There are more efficient search structures we could use for
	// this, but for the number of points we end up having even on
	// pathfind failure (~1200) the fancy structures don't justify
	// their extra overhead.
	//
	// Linear search of an array is just as fast, or slightly faster.
	//
	Common::Array<PathfindingState>::const_iterator iter;
	for (iter = _visited.begin(); iter != _visited.end(); iter++) {
		if (iter->checkPoint(x, y, z, 8*8))
			return true;
	}

	return false;
}

bool Pathfinder::checkTarget(const PathNode *node) const {
	// TODO: these ranges are probably a bit too high,
	// but otherwise it won't work properly yet -wjp
	if (_targetItem) {
		if (_hitMode) {
			return node->state.checkHit(_actor,
			                            dynamic_cast<Actor *>(_targetItem));
		} else {
			return node->state.checkItem(_targetItem, 32, 8);
		}
	} else {
		return node->state.checkPoint(_targetX, _targetY, _targetZ, 48*48);
	}
}

unsigned int Pathfinder::costHeuristic(PathNode *node) const {
	unsigned int cost = node->cost;

#if 0
	double sqrddist;

	sqrddist = (_targetX - node->state._x + _actorXd / 2) *
	           (_targetX - node->state._x + _actorXd / 2);
	sqrddist += (_targetY - node->state._y + _actorYd / 2) *
	            (_targetY - node->state._y + _actorYd / 2);

	unsigned int dist = static_cast<unsigned int>(Std::sqrt(sqrddist));
#else
	// This calculates the distance to the target using only lines in
	// the 8 available directions (instead of the straight line above)
	int xd = ABS(_targetX - node->state._x + _actorXd / 2);
	int yd = ABS(_targetY - node->state._y + _actorYd / 2);
	double m = (xd < yd) ? xd : yd;
	unsigned int dist = ABS(xd - yd) + static_cast<unsigned int>(m * 1.41421356);

#endif

#if 0
	//!! TODO: divide dist by walking distance
	// (using 32 for now)
	dist /= 32;

	node->heuristicTotalCost = cost + (dist * 4); //!! constant
#else

	// Weigh remaining distance more than already travelled distance,
	// to try to explore more nodes closer to the target.
	node->heuristicTotalCost = 2 * cost + 3 * dist;
#endif

	return node->heuristicTotalCost;
}


#ifdef DEBUG

static void drawbox(const Item *item) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Rect d;
	screen->GetSurfaceDims(d);

	int32 ix, iy, iz;
	item->getLocation(ix, iy, iz);

	int32 xd, yd, zd;
	item->getFootpadWorld(xd, yd, zd);

	ix -= cx;
	iy -= cy;
	iz -= cz;

	int32 x0, y0, x1, y1, x2, y2, x3, y3;

	x0 = (d.width() / 2) + (ix - iy) / 4;
	y0 = (d.height() / 2) + (ix + iy) / 8 - iz;

	x1 = (d.width() / 2) + (ix - iy) / 4;
	y1 = (d.height() / 2) + (ix + iy) / 8 - (iz + zd);

	x2 = (d.width() / 2) + (ix - xd - iy) / 4;
	y2 = (d.height() / 2) + (ix - xd + iy) / 8 - iz;

	x3 = (d.width() / 2) + (ix - iy + yd) / 4;
	y3 = (d.height() / 2) + (ix + iy - yd) / 8 - iz;

	screen->Fill32(0xFF0000FF, x0 - 1, y0 - 1, 3, 3);

	screen->DrawLine32(0xFF00FF00, x0, y0, x1, y1);
	screen->DrawLine32(0xFF00FF00, x0, y0, x2, y2);
	screen->DrawLine32(0xFF00FF00, x0, y0, x3, y3);
}

static void drawdot(int32 x, int32 y, int32 Z, int size, uint32 rgb) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Rect d;
	screen->GetSurfaceDims(d);
	x -= cx;
	y -= cy;
	Z -= cz;
	int32 x0, y0;
	x0 = (d.width() / 2) + (x - y) / 4;
	y0 = (d.height() / 2) + (x + y) / 8 - Z;
	screen->Fill32(rgb, x0 - size, y0 - size, 2 * size + 1, 2 * size + 1);
}

static void drawedge(const PathNode *from, const PathNode *to, uint32 rgb) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Rect d;
	screen->GetSurfaceDims(d);

	int32 x0, y0, x1, y1;

	cx = from->state._x - cx;
	cy = from->state._y - cy;
	cz = from->state._z - cz;

	x0 = (d.width() / 2) + (cx - cy) / 4;
	y0 = (d.height() / 2) + (cx + cy) / 8 - cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	cx = to->state._x - cx;
	cy = to->state._y - cy;
	cz = to->state._z - cz;

	x1 = (d.width() / 2) + (cx - cy) / 4;
	y1 = (d.height() / 2) + (cx + cy) / 8 - cz;

	screen->DrawLine32(rgb, x0, y0, x1, y1);
}

static void drawpath(PathNode *to, uint32 rgb, bool done) {
	PathNode *n1 = to;
	PathNode *n2 = to->parent;

	while (n2) {
		drawedge(n1, n2, rgb);

		if (done && n1 == to)
			drawdot(n1->state._x, n1->state._y, n1->state._z, 2, 0xFFFF0000);
		else
			drawdot(n1->state._x, n1->state._y, n1->state._z, 1, 0xFFFFFFFF);


		drawdot(n2->state._x, n2->state._y, n2->state._z, 2, 0xFFFFFFFF);

		n1 = n2;
		n2 = n1->parent;
	}
}

#endif

void Pathfinder::newNode(PathNode *oldnode, PathfindingState &state,
                         unsigned int steps) {
	PathNode *newnode = new PathNode();
	newnode->state = state;
	newnode->parent = oldnode;
	newnode->depth = oldnode->depth + 1;
	newnode->stepsfromparent = 0;

	double sqrddist;

	sqrddist = ((newnode->state._x - oldnode->state._x) *
	            (newnode->state._x - oldnode->state._x));
	sqrddist += ((newnode->state._y - oldnode->state._y) *
	             (newnode->state._y - oldnode->state._y));
	sqrddist += ((newnode->state._z - oldnode->state._z) *
	             (newnode->state._z - oldnode->state._z));

	unsigned int dist;
	dist = static_cast<unsigned int>(Std::sqrt(sqrddist));

	int turn = 0;

	if (oldnode->depth > 0) {
		turn = state._direction - oldnode->state._direction;
		if (turn < 0) turn = -turn;
		if (turn > 8) turn = 16 - turn;
	}

	newnode->cost = oldnode->cost + dist + 32 * turn; //!! constant

	bool done = checkTarget(newnode);
	if (done)
		newnode->heuristicTotalCost = 0;
	else
		costHeuristic(newnode);

#if 0
	perr << "trying dir " << state._direction;

	if (steps > 0) {
		perr << ", " << steps << " steps";
	}
	perr << " from ("
	     << oldnode->state._x << "," << oldnode->state._y << ") to ("
	     << newnode->state._x << "," << newnode->state._y
	     << "), cost = " << newnode->cost << ", heurtotcost = "
	     << newnode->heuristicTotalCost << Std::endl;
#endif

#ifdef DEBUG
	if (_actor->getObjId() == _visualDebugActor) {
		RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
		screen->BeginPainting();
		drawpath(newnode, 0xFFFFFF00, done);
		screen->EndPainting();
		g_system->delayMillis(50);
		if (!done) {
			screen->BeginPainting();
			drawpath(newnode, 0xFFB0B000, done);
			screen->EndPainting();
		}
	}
#endif

	_nodes.push(newnode);
}

void Pathfinder::expandNode(PathNode *node) {
	Animation::Sequence walkanim = Animation::walk;
	PathfindingState state, closeststate;
	AnimationTracker tracker;
	expandednodes++;

	if (_actor->isInCombat())
		walkanim = Animation::advance;

	// try walking in all 8 directions - TODO: should this support 16 dirs?
	Direction dir = dir_north;
	for (int i = 0; i < 8; i++) {
		dir = Direction_OneRight(dir, dirmode_8dirs);
		state = node->state;
		state._lastAnim = walkanim;
		state._direction = dir;
		state._combat = _actor->isInCombat();

		if (!tracker.init(_actor, walkanim, dir, &state)) continue;

		// determine how far the _actor will travel if the animation runs to completion
		int32 max_endx, max_endy;
		tracker.evaluateMaxAnimTravel(max_endx, max_endy, dir);
		if (alreadyVisited(max_endx, max_endy, state._z)) continue;

		const int x_travel = ABS(max_endx - state._x);
		const int y_travel = ABS(max_endy - state._y);
		const int xy_maxtravel = MAX(x_travel, y_travel);

		int sqrddist = x_travel * x_travel + y_travel * y_travel;
		if (sqrddist > 400) {
			// range is greater than 20; see if a node has been visited at range 10
			if (alreadyVisited(state._x + x_travel * 10 / xy_maxtravel,
			                   state._y + y_travel * 10 / xy_maxtravel,
			                   state._z)) {
				continue;
			}
		}

		uint32 steps = 0, beststeps = 0;
		int bestsqdist;
		bestsqdist = (_targetX - node->state._x + _actorXd / 2) *
					 (_targetX - node->state._x + _actorXd / 2);
		bestsqdist += (_targetY - node->state._y + _actorYd / 2) *
					  (_targetY - node->state._y + _actorYd / 2);

		while (tracker.step()) {
			steps++;
			tracker.updateState(state);

			sqrddist = (_targetX - state._x + _actorXd / 2) *
			           (_targetX - state._x + _actorXd / 2);
			sqrddist += (_targetY - state._y + _actorYd / 2) *
			            (_targetY - state._y + _actorYd / 2);

			if (sqrddist < bestsqdist) {
				bestsqdist = sqrddist;
				beststeps = steps;
				closeststate = state;
			}
		}

		if (tracker.isDone()) {
			tracker.updateState(state);
			if (!alreadyVisited(state._x, state._y, state._z)) {
				newNode(node, state, 0);
				_visited.push_back(state);
			}
		} else {
			// an obstruction was encountered, so generate a visited node to block
			// future evaluation at the endpoint.
			_visited.push_back(state);
		}

		// TODO: maybe only allow partial steps close to target?
		if (beststeps != 0 && (beststeps != steps ||
		                       (!tracker.isDone() && _targetItem))) {
			newNode(node, closeststate, beststeps);
			_visited.push_back(closeststate);
		}
	}
}

bool Pathfinder::pathfind(Std::vector<PathfindingAction> &path) {
#if 0
	pout << "Actor " << _actor->getObjId();

	if (_targetItem) {
		pout << " pathfinding to item: ";
		_targetItem->dumpInfo();
	} else {
		pout << " pathfinding to (" << _targetX << "," << _targetY << "," << _targetZ << ")" << Std::endl;
	}
#endif

#ifdef DEBUG
	if (_actor->getObjId() == _visualDebugActor) {
		RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
		screen->BeginPainting();
		if (_targetItem)
			drawbox(_targetItem);
		else
			drawdot(_targetX, _targetY, _targetZ, 2, 0xFF0000FF);
		screen->EndPainting();
	}
#endif


	path.clear();

	PathNode *startnode = new PathNode();
	startnode->state = _start;
	startnode->cost = 0;
	startnode->parent = nullptr;
	startnode->depth = 0;
	startnode->stepsfromparent = 0;
	_nodes.push(startnode);

	unsigned int expandedNodes = 0;
	const unsigned int NODELIMIT_MIN = 30;  //! constant
	const unsigned int NODELIMIT_MAX = 200; //! constant
	bool found = false;
	uint32 starttime = g_system->getMillis();

	while (expandedNodes < NODELIMIT_MAX && !_nodes.empty() && !found) {
		// Take a copy here as the pop() below deletes the old node
		PathNode *node = new PathNode(*_nodes.top());
		_cleanupNodes.push_back(node);
		_nodes.pop();

#if 0
		pout << "Trying node: (" << node->state._x << "," << node->state._y
		     << "," << node->state._z << ") target=(" << _targetX << ","
		     << _targetY << "," << _targetZ << ")" << Std::endl;
#endif

		if (checkTarget(node)) {
			// done!

			// find path length
			const PathNode *n = node;
			unsigned int length = 0;
			while (n->parent) {
				n = n->parent;
				length++;
			}
#if 0
			pout << "Pathfinder: path found (length = " << length << ")"
			     << Std::endl;
#endif

			unsigned int i = length;
			if (length > 0) length++; // add space for final 'stand' action
			path.resize(length);

			// now backtrack through the _nodes to assemble the final animation
			while (node->parent) {
				PathfindingAction action;
				action._action = node->state._lastAnim;
				action._direction = node->state._direction;
				action._steps = node->stepsfromparent;
				path[--i] = action;
#if 0
				pout << "anim = " << node->state._lastAnim << ", dir = "
				     << node->state._direction << ", steps = "
				     << node->stepsfromparent << Std::endl;
#endif

				//TODO: check how turns work
				//TODO: append final 'stand' animation

				node = node->parent;
			}

			if (length) {
				if (node->state._combat)
					path[length - 1]._action = Animation::combatStand;
				else
					path[length - 1]._action = Animation::stand;
				path[length - 1]._direction = path[length - 2]._direction;
			}

			_expandTime = g_system->getMillis() - starttime;
			return true;
		}

		expandNode(node);
		expandedNodes++;

		if (expandedNodes >= NODELIMIT_MIN && ((expandedNodes) % 5) == 0) {
			uint32 elapsed_ms = g_system->getMillis() - starttime;
			if (elapsed_ms > 350) break;
		}
	}

	_expandTime = g_system->getMillis() - starttime;

#if 0
	static int32 pfcalls = 0;
	static int32 pftotaltime = 0;
	pfcalls++;
	pftotaltime += _expandTime;
	pout << "maxout average = " << (pftotaltime / pfcalls) << "ms." << Std::endl;
#endif

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
