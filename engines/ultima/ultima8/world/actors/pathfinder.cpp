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

#include "common/system.h"
#include "ultima/ultima.h"
#include "ultima/ultima8/misc/direction_util.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"

#ifdef DEBUG_PATHFINDER
#include "graphics/screen.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#endif

namespace Ultima {
namespace Ultima8 {


#ifdef DEBUG_PATHFINDER
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
	_point = _actor->getLocation();
	_lastAnim = _actor->getLastAnim();
	_direction = _actor->getDir();
	_firstStep = _actor->hasActorFlags(Actor::ACT_FIRSTSTEP);
	_flipped = _actor->hasFlags(Item::FLG_FLIPPED);
	_combat = _actor->isInCombat();
}

bool PathfindingState::checkPoint(const Point3 &pt,
								  int sqr_range) const {
	int distance = _point.sqrDist(pt);
	return distance < sqr_range;
}

bool PathfindingState::checkItem(const Item *item, int xyRange, int zRange) const {
	int32 itemXd, itemYd, itemZd;
	int32 itemXmin, itemYmin;

	Point3 pt = item->getLocationAbsolute();
	item->getFootpadWorld(itemXd, itemYd, itemZd);

	itemXmin = pt.x - itemXd;
	itemYmin = pt.y - itemYd;

	int range = 0;
	if (_point.x - pt.x > range)
		range = _point.x - pt.x;
	if (itemXmin - _point.x > range)
		range = itemXmin - _point.x;
	if (_point.y - pt.y > range)
		range = _point.y - pt.y;
	if (itemYmin - _point.y > range)
		range = itemYmin - _point.y;

	// FIXME: check _point.z properly

	return (range <= xyRange);
}

bool PathfindingState::checkHit(const Actor *_actor, const Item *target) const {
	assert(target);
	debugC(kDebugPath, "Trying hit in _direction %d", _actor->getDirToItemCentre(*target));
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
		_hitMode(false), _expandTime(0), _target(),
		_actorXd(0), _actorYd(0), _actorZd(0) {
	expandednodes = 0;
	_visited.reserve(1500);
}

Pathfinder::~Pathfinder() {
	debugC(kDebugPath, "~Pathfinder: %u nodes to clean up, visited %u and %u expanded nodes in %dms.",
		_cleanupNodes.size(), _visited.size(), expandednodes, _expandTime);

	// clean up _nodes
	for (auto *node : _cleanupNodes)
		delete node;
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

void Pathfinder::setTarget(const Point3 &pt) {
	_target = pt;
	_targetItem = 0;
	_hitMode = false;
}

void Pathfinder::setTarget(Item *item, bool hit) {
	Container *root = item->getRootContainer();
	_targetItem = root ? root : item;

	// set target to centre of item for the cost heuristic
	_target = item->getCentre();
	_target.z = item->getZ();

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

bool Pathfinder::alreadyVisited(const Point3 &pt) const {
	//
	// There are more efficient search structures we could use for
	// this, but for the number of points we end up having even on
	// pathfind failure (~1200) the fancy structures don't justify
	// their extra overhead.
	//
	// Linear search of an array is just as fast, or slightly faster.
	//
	for (const auto &i : _visited) {
		if (i.checkPoint(pt, 8*8))
			return true;
	}

	return false;
}

bool Pathfinder::checkTarget(const PathNode *node) const {
	// TODO: these ranges are probably a bit too high,
	// but otherwise it won't work properly yet -wjp
	if (_targetItem) {
		if (_hitMode) {
			return node->state.checkHit(_actor, _targetItem);
		} else {
			return node->state.checkItem(_targetItem, 32, 8);
		}
	} else {
		return node->state.checkPoint(_target, 48*48);
	}
}

unsigned int Pathfinder::costHeuristic(PathNode *node) const {
	unsigned int cost = node->cost;

#if 0
	double sqrddist;

	sqrddist = (_target.x - node->state._point.x + _actorXd / 2) *
	           (_target.x - node->state._point.x + _actorXd / 2);
	sqrddist += (_target.y - node->state._point.y + _actorYd / 2) *
	            (_target.y - node->state._point.y + _actorYd / 2);

	unsigned int dist = static_cast<unsigned int>(sqrt(sqrddist));
#else
	// This calculates the distance to the target using only lines in
	// the 8 available directions (instead of the straight line above)
	int xd = ABS(_target.x - node->state._point.x + _actorXd / 2);
	int yd = ABS(_target.y - node->state._point.y + _actorYd / 2);
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

#ifdef DEBUG_PATHFINDER

static void drawbox(Graphics::ManagedSurface *screen, const Item *item) {
	int32 cx, cy, cz;
	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Common::Rect d = screen->getBounds();

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

	uint32 color = screen->format.RGBToColor(0x00, 0x00, 0xFF);
	screen->fillRect(Common::Rect(x0 - 1, y0 - 1, x0 + 2, y0 + 2), color);

	color = screen->format.RGBToColor(0x00, 0xFF, 0x00);
	screen->drawLine(x0, y0, x1, y1, color);
	screen->drawLine(x0, y0, x2, y2, color);
	screen->drawLine(x0, y0, x3, y3, color);
}

static void drawdot(Graphics::ManagedSurface *screen, int32 x, int32 y, int32 Z, int size, uint32 rgb) {
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Common::Rect d = screen->getBounds();
	x -= cx;
	y -= cy;
	Z -= cz;
	int32 x0, y0;
	x0 = (d.width() / 2) + (x - y) / 4;
	y0 = (d.height() / 2) + (x + y) / 8 - Z;
	screen->fillRect(Common::Rect(x0 - size, y0 - size, x0 + size + 1, y0 + size + 1), rgb);
}

static void drawedge(Graphics::ManagedSurface *screen, const PathNode *from, const PathNode *to, uint32 rgb) {
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Common::Rect d = screen->getBounds();

	int32 x0, y0, x1, y1;

	cx = from->state._point.x - cx;
	cy = from->state._point.y - cy;
	cz = from->state._point.z - cz;

	x0 = (d.width() / 2) + (cx - cy) / 4;
	y0 = (d.height() / 2) + (cx + cy) / 8 - cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	cx = to->state._point.x - cx;
	cy = to->state._point.y - cy;
	cz = to->state._point.z - cz;

	x1 = (d.width() / 2) + (cx - cy) / 4;
	y1 = (d.height() / 2) + (cx + cy) / 8 - cz;

	screen->drawLine(x0, y0, x1, y1, rgb);
}

static void drawpath(Graphics::ManagedSurface *screen, PathNode *to, uint32 rgb, bool done) {
	PathNode *n1 = to;
	PathNode *n2 = to->parent;
	uint32 color1 = screen->format.RGBToColor(0xFF, 0x00, 0x00);
	uint32 color2 = screen->format.RGBToColor(0xFF, 0xFF, 0xFF);

	while (n2) {
		drawedge(screen, n1, n2, rgb);

		if (done && n1 == to)
			drawdot(screen, n1->state._point.x, n1->state._point.y, n1->state._point.z, 2, color1);
		else
			drawdot(screen, n1->state._point.x, n1->state._point.y, n1->state._point.z, 1, color2);


		drawdot(screen, n2->state._point.x, n2->state._point.y, n2->state._point.z, 2, color2);

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

	sqrddist = ((newnode->state._point.x - oldnode->state._point.x) *
	            (newnode->state._point.x - oldnode->state._point.x));
	sqrddist += ((newnode->state._point.y - oldnode->state._point.y) *
	             (newnode->state._point.y - oldnode->state._point.y));
	sqrddist += ((newnode->state._point.z - oldnode->state._point.z) *
	             (newnode->state._point.z - oldnode->state._point.z));

	unsigned int dist;
	dist = static_cast<unsigned int>(sqrt(sqrddist));

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

	debugC(kDebugPath, "Trying dir %d, steps %d from (%d, %d) to (%d, %d), cost %d, heurtotcost %d",
		   state._direction, steps,
		   oldnode->state._point.x, oldnode->state._point.y, newnode->state._point.x, newnode->state._point.y,
		   newnode->cost, newnode->heuristicTotalCost);

#ifdef DEBUG_PATHFINDER
	if (_actor->getObjId() == _visualDebugActor) {
		Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
		uint32 color = screen->format.RGBToColor(0xFF, 0xFF, 0x00);
		drawpath(screen, newnode, color, done);
		screen->update();
		g_system->delayMillis(50);
		if (!done) {
			color = screen->format.RGBToColor(0xB0, 0xB0, 0x00);
			drawpath(screen, newnode, color, done);
			screen->update();
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
		Point3 max_end;
		tracker.evaluateMaxAnimTravel(max_end.x, max_end.y, dir);
		max_end.z = state._point.z;
		if (alreadyVisited(max_end))
			continue;

		const int x_travel = ABS(max_end.x - state._point.x);
		const int y_travel = ABS(max_end.y - state._point.y);
		const int xy_maxtravel = MAX(x_travel, y_travel);

		int sqrddist = x_travel * x_travel + y_travel * y_travel;
		if (sqrddist > 400) {
			// range is greater than 20; see if a node has been visited at range 10
			Point3 pt = state._point;
			pt.x += x_travel * 10 / xy_maxtravel;
			pt.y += y_travel * 10 / xy_maxtravel;
			if (alreadyVisited(pt)) {
				continue;
			}
		}

		uint32 steps = 0, beststeps = 0;
		int bestsqdist;
		bestsqdist = (_target.x - node->state._point.x + _actorXd / 2) *
					 (_target.x - node->state._point.x + _actorXd / 2);
		bestsqdist += (_target.y - node->state._point.y + _actorYd / 2) *
					  (_target.y - node->state._point.y + _actorYd / 2);

		while (tracker.step()) {
			steps++;
			tracker.updateState(state);

			sqrddist = (_target.x - state._point.x + _actorXd / 2) *
			           (_target.x - state._point.x + _actorXd / 2);
			sqrddist += (_target.y - state._point.y + _actorYd / 2) *
			            (_target.y - state._point.y + _actorYd / 2);

			if (sqrddist < bestsqdist) {
				bestsqdist = sqrddist;
				beststeps = steps;
				closeststate = state;
			}
		}

		if (tracker.isDone()) {
			tracker.updateState(state);
			if (!alreadyVisited(state._point)) {
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
	if (_targetItem) {
		debugC(kDebugPath, "Actor %u pathfinding to item %u", _actor->getObjId(), _targetItem->getObjId());
		debugC(kDebugPath, "Target Item: %s", _targetItem->dumpInfo().c_str());
	} else {
		debugC(kDebugPath, "Actor %u pathfinding to (%d, %d, %d)", _actor->getObjId(), _target.x, _target.y, _target.z);
	}

#ifdef DEBUG_PATHFINDER
	if (_actor->getObjId() == _visualDebugActor) {
		Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
		if (_targetItem) {
			drawbox(screen, _targetItem);
		} else {
			uint32 color = screen->format.RGBToColor(0x00, 0x00, 0xFF);
			drawdot(screen, _targetX, _targetY, _targetZ, 2, color);
		}
		screen->update();
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

		debugC(kDebugPath, "Trying node: (%d, %d, %d) target=(%d, %d, %d)",
			node->state._point.x, node->state._point.y, node->state._point.z,
			_target.x, _target.y, _target.z);

		if (checkTarget(node)) {
			// done!

			// find path length
			const PathNode *n = node;
			unsigned int length = 0;
			while (n->parent) {
				n = n->parent;
				length++;
			}

			debugC(kDebugPath, "Pathfinder: path found (length = %u)", length);

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

				debugC(kDebugPath, "anim = %d, dir = %d, steps = %d",
					node->state._lastAnim, node->state._direction, node->stepsfromparent);

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

	static int32 pfcalls = 0;
	static int32 pftotaltime = 0;
	pfcalls++;
	pftotaltime += _expandTime;
	debugC(kDebugPath, "maxout average = %dms.", pftotaltime / pfcalls);

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
