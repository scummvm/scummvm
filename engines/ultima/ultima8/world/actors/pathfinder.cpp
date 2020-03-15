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
	_firstStep = (_actor->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	_flipped = (_actor->getFlags() & Item::FLG_FLIPPED) != 0;
	_combat = _actor->isInCombat();
}

bool PathfindingState::checkPoint(int32 x_, int32 y_, int32 z_,
                                  int range) const {
	int distance = (_x - x_) * (_x - x_) + (_y - y_) * (_y - y_) + (_z - z_) * (_z - z_);
	return distance < range * range;
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

bool PathfindingState::checkHit(Actor *_actor, const Actor *target) {
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
	return (n1->heuristicTotalCost < n2->heuristicTotalCost);
}

Pathfinder::Pathfinder() {
	expandednodes = 0;
}

Pathfinder::~Pathfinder() {
#if 1
	pout << "~Pathfinder: " << _nodeList.size() << " _nodes, "
	     << expandednodes << " expanded _nodes in " << _expandTime << "ms." << Std::endl;
#endif

	// clean up _nodes
	Std::list<PathNode *>::iterator iter;
	for (iter = _nodeList.begin(); iter != _nodeList.end(); ++iter)
		delete *iter;
	_nodeList.clear();
}

void Pathfinder::init(Actor *actor_, PathfindingState *state) {
	_actor = actor_;

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
		assert(p_dynamic_cast<Actor *>(_targetItem));
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
	//! this may need optimization

	Std::list<PathfindingState>::const_iterator iter;

	for (iter = _visited.begin(); iter != _visited.end(); ++iter)
		if (iter->checkPoint(x, y, z, 8))
			return true;

	return false;
}

bool Pathfinder::checkTarget(PathNode *node) {
	// TODO: these ranges are probably a bit too high,
	// but otherwise it won't work properly yet -wjp
	if (_targetItem) {
		if (_hitMode) {
			return node->state.checkHit(_actor,
			                            p_dynamic_cast<Actor *>(_targetItem));
		} else {
			return node->state.checkItem(_targetItem, 32, 8);
		}
	} else {
		return node->state.checkPoint(_targetX, _targetY, _targetZ, 48);
	}
}

unsigned int Pathfinder::costHeuristic(PathNode *node) {
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
	// to try to explore more _nodes closer to the target.
	node->heuristicTotalCost = 2 * cost + 3 * dist;
#endif

	return node->heuristicTotalCost;
}


#ifdef DEBUG

// FIXME: these functions assume that we're using a 2x scaler...
// (and the whole system is generally a very big hack...)

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

	x0 = (d.w / 2) + (ix - iy) / 2;
	y0 = (d.h / 2) + (ix + iy) / 4 - iz * 2;

	x1 = (d.w / 2) + (ix - iy) / 2;
	y1 = (d.h / 2) + (ix + iy) / 4 - (iz + zd) * 2;

	x2 = (d.w / 2) + (ix - xd - iy) / 2;
	y2 = (d.h / 2) + (ix - xd + iy) / 4 - iz * 2;

	x3 = (d.w / 2) + (ix - iy + yd) / 2;
	y3 = (d.h / 2) + (ix + iy - yd) / 4 - iz * 2;

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
	x0 = (d.w / 2) + (x - y) / 2;
	y0 = (d.h / 2) + (x + y) / 4 - Z * 2;
	screen->Fill32(rgb, x0 - size, y0 - size, 2 * size + 1, 2 * size + 1);
}

static void drawedge(PathNode *from, PathNode *to, uint32 rgb) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getRenderScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Rect d;
	screen->GetSurfaceDims(d);

	int32 x0, y0, x1, y1;

	cx = from->state._x - cx;
	cy = from->state._y - cy;
	cz = from->state._z - cz;

	x0 = (d.w / 2) + (cx - cy) / 2;
	y0 = (d.h / 2) + (cx + cy) / 4 - cz * 2;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	cx = to->state._x - cx;
	cy = to->state._y - cy;
	cz = to->state._z - cz;

	x1 = (d.w / 2) + (cx - cy) / 2;
	y1 = (d.h / 2) + (cx + cy) / 4 - cz * 2;

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
	_nodeList.push_back(newnode); // for garbage collection
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
		if (turn > 4) turn = 8 - turn;
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
		g_system->delayMillis(250);
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

	// try walking in all 8 directions
	for (uint32 dir = 0; dir < 8; ++dir) {
		state = node->state;
		state._lastAnim = walkanim;
		state._direction = dir;
		state._combat = _actor->isInCombat();
		uint32 steps = 0, beststeps = 0;
		int bestsqdist;
		bestsqdist = (_targetX - node->state._x + _actorXd / 2) *
		             (_targetX - node->state._x + _actorXd / 2);
		bestsqdist += (_targetY - node->state._y + _actorYd / 2) *
		              (_targetY - node->state._y + _actorYd / 2);

		if (!tracker.init(_actor, walkanim, dir, &state)) continue;

		// determine how far the _actor will travel if the animation runs to completion
		int32 max_endx, max_endy;
		tracker.evaluateMaxAnimTravel(max_endx, max_endy, dir);
		if (alreadyVisited(max_endx, max_endy, state._z)) continue;
		int sqrddist;
		const int x_travel = ABS(max_endx - state._x);
		int xy_maxtravel = x_travel;    // don't have the max(a,b) macro...
		const int y_travel = ABS(max_endy - state._y);
		if (y_travel > xy_maxtravel) xy_maxtravel = y_travel;

		sqrddist = x_travel * x_travel + y_travel * y_travel;
		if (sqrddist > 400) {
			// range is greater than 20; see if a node has been _visited at range 10
			if (alreadyVisited(state._x + x_travel * 10 / xy_maxtravel,
			                   state._y + y_travel * 10 / xy_maxtravel,
			                   state._z)) {
				continue;
			}
		}
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
			// an obstruction was encountered, so generate a _visited node to block
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
	_nodeList.push_back(startnode);
	_nodes.push(startnode);

	unsigned int expandedNodes = 0;
	const unsigned int NODELIMIT_MIN = 30;  //! constant
	const unsigned int NODELIMIT_MAX = 200; //! constant
	bool found = false;
	uint32 starttime = g_system->getMillis();

	while (expandedNodes < NODELIMIT_MAX && !_nodes.empty() && !found) {
		PathNode *node = _nodes.top();
		_nodes.pop();

#if 0
		pout << "Trying node: (" << node->state._x << "," << node->state._y
		     << "," << node->state._z << ") target=(" << _targetX << ","
		     << _targetY << "," << _targetZ << ")" << Std::endl;
#endif

		if (checkTarget(node)) {
			// done!

			// find path length
			PathNode *n = node;
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
