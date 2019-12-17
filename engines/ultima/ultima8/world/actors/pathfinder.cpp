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
ObjId Pathfinder::visualdebug_actor = 0xFFFF;
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

void PathfindingState::load(Actor *actor) {
	actor->getLocation(x, y, z);
	lastanim = actor->getLastAnim();
	direction = actor->getDir();
	firststep = (actor->getActorFlags() & Actor::ACT_FIRSTSTEP) != 0;
	flipped = (actor->getFlags() & Item::FLG_FLIPPED) != 0;
	combat = actor->isInCombat();
}

bool PathfindingState::checkPoint(int32 x_, int32 y_, int32 z_,
                                  int range) {
	int distance = (x - x_) * (x - x_) + (y - y_) * (y - y_) + (z - z_) * (z - z_);
	return distance < range * range;
}

bool PathfindingState::checkItem(Item *item, int xyRange, int zRange) {
	int32 itemX, itemY, itemZ;
	int32 itemXd, itemYd, itemZd;
	int32 itemXmin, itemYmin;

	item->getLocationAbsolute(itemX, itemY, itemZ);
	item->getFootpadWorld(itemXd, itemYd, itemZd);

	itemXmin = itemX - itemXd;
	itemYmin = itemY - itemYd;

	int range = 0;
	if (x - itemX > range)
		range = x - itemX;
	if (itemXmin - x > range)
		range = itemXmin - x;
	if (y - itemY > range)
		range = y - itemY;
	if (itemYmin - y > range)
		range = itemYmin - y;

	// FIXME: check z properly

	return (range <= xyRange);
}

bool PathfindingState::checkHit(Actor *actor, Actor *target) {
#if 0
	pout << "Trying hit in direction " << actor->getDirToItemCentre(*target) << std::endl;
#endif
	AnimationTracker tracker;
	if (!tracker.init(actor, Animation::attack,
	                  actor->getDirToItemCentre(*target), this)) {
		return false;
	}

	while (tracker.step()) {
		if (tracker.hitSomething()) break;
	}

	ObjId hit = tracker.hitSomething();
	if (hit == target->getObjId()) return true;

	return false;
}

bool PathNodeCmp::operator()(PathNode *n1, PathNode *n2) {
	return (n1->heuristicTotalCost < n2->heuristicTotalCost);
}

Pathfinder::Pathfinder() {
	expandednodes = 0;
}

Pathfinder::~Pathfinder() {
#if 1
	pout << "~Pathfinder: " << nodelist.size() << " nodes, "
	     << expandednodes << " expanded nodes in " << expandtime << "ms." << std::endl;
#endif

	// clean up nodes
	std::list<PathNode *>::iterator iter;
	for (iter = nodelist.begin(); iter != nodelist.end(); ++iter)
		delete *iter;
	nodelist.clear();
}

void Pathfinder::init(Actor *actor_, PathfindingState *state) {
	actor = actor_;

	actor->getFootpadWorld(actor_xd, actor_yd, actor_zd);

	if (state)
		start = *state;
	else
		start.load(actor);
}

void Pathfinder::setTarget(int32 x, int32 y, int32 z) {
	targetx = x;
	targety = y;
	targetz = z;
	targetitem = 0;
	hitmode = false;
}

void Pathfinder::setTarget(Item *item, bool hit) {
	targetitem = item;
	while (targetitem->getParentAsContainer())
		targetitem = targetitem->getParentAsContainer();

	// set target to centre of item for the cost heuristic
	item->getCentre(targetx, targety, targetz);
	targetz = item->getZ();

	if (hit) {
		assert(start.combat);
		assert(p_dynamic_cast<Actor *>(targetitem));
		hitmode = true;
	} else {
		hitmode = false;
	}
}

bool Pathfinder::canReach() {
	std::vector<PathfindingAction> path;
	return pathfind(path);
}

bool Pathfinder::alreadyVisited(int32 x, int32 y, int32 z) {
	//! this may need optimization

	std::list<PathfindingState>::iterator iter;

	for (iter = visited.begin(); iter != visited.end(); ++iter)
		if (iter->checkPoint(x, y, z, 8))
			return true;

	return false;
}

bool Pathfinder::checkTarget(PathNode *node) {
	// TODO: these ranges are probably a bit too high,
	// but otherwise it won't work properly yet -wjp
	if (targetitem) {
		if (hitmode) {
			return node->state.checkHit(actor,
			                            p_dynamic_cast<Actor *>(targetitem));
		} else {
			return node->state.checkItem(targetitem, 32, 8);
		}
	} else {
		return node->state.checkPoint(targetx, targety, targetz, 48);
	}
}

unsigned int Pathfinder::costHeuristic(PathNode *node) {
	unsigned int cost = node->cost;

#if 0
	double sqrddist;

	sqrddist = (targetx - node->state.x + actor_xd / 2) *
	           (targetx - node->state.x + actor_xd / 2);
	sqrddist += (targety - node->state.y + actor_yd / 2) *
	            (targety - node->state.y + actor_yd / 2);

	unsigned int dist = static_cast<unsigned int>(std::sqrt(sqrddist));
#else
	// This calculates the distance to the target using only lines in
	// the 8 available directions (instead of the straight line above)
	int xd = ABS(targetx - node->state.x + actor_xd / 2);
	int yd = ABS(targety - node->state.y + actor_yd / 2);
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

// FIXME: these functions assume that we're using a 2x scaler...
// (and the whole system is generally a very big hack...)

static void drawbox(Item *item) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Pentagram::Rect d;
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

static void drawdot(int32 x, int32 y, int32 z, int size, uint32 rgb) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Pentagram::Rect d;
	screen->GetSurfaceDims(d);
	x -= cx;
	y -= cy;
	z -= cz;
	int32 x0, y0;
	x0 = (d.w / 2) + (x - y) / 2;
	y0 = (d.h / 2) + (x + y) / 4 - z * 2;
	screen->Fill32(rgb, x0 - size, y0 - size, 2 * size + 1, 2 * size + 1);
}

static void drawedge(PathNode *from, PathNode *to, uint32 rgb) {
	RenderSurface *screen = Ultima8Engine::get_instance()->getScreen();
	int32 cx, cy, cz;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	Pentagram::Rect d;
	screen->GetSurfaceDims(d);

	int32 x0, y0, x1, y1;

	cx = from->state.x - cx;
	cy = from->state.y - cy;
	cz = from->state.z - cz;

	x0 = (d.w / 2) + (cx - cy) / 2;
	y0 = (d.h / 2) + (cx + cy) / 4 - cz * 2;

	Ultima8Engine::get_instance()->getGameMapGump()->GetCameraLocation(cx, cy, cz);

	cx = to->state.x - cx;
	cy = to->state.y - cy;
	cz = to->state.z - cz;

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
			drawdot(n1->state.x, n1->state.y, n1->state.z, 2, 0xFFFF0000);
		else
			drawdot(n1->state.x, n1->state.y, n1->state.z, 1, 0xFFFFFFFF);


		drawdot(n2->state.x, n2->state.y, n2->state.z, 2, 0xFFFFFFFF);

		n1 = n2;
		n2 = n1->parent;
	}
}

#endif

void Pathfinder::newNode(PathNode *oldnode, PathfindingState &state,
                         unsigned int steps) {
	PathNode *newnode = new PathNode();
	nodelist.push_back(newnode); // for garbage collection
	newnode->state = state;
	newnode->parent = oldnode;
	newnode->depth = oldnode->depth + 1;
	newnode->stepsfromparent = 0;

	double sqrddist;

	sqrddist = ((newnode->state.x - oldnode->state.x) *
	            (newnode->state.x - oldnode->state.x));
	sqrddist += ((newnode->state.y - oldnode->state.y) *
	             (newnode->state.y - oldnode->state.y));
	sqrddist += ((newnode->state.z - oldnode->state.z) *
	             (newnode->state.z - oldnode->state.z));

	unsigned int dist;
	dist = static_cast<unsigned int>(std::sqrt(sqrddist));

	int turn = 0;

	if (oldnode->depth > 0) {
		turn = state.direction - oldnode->state.direction;
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
	perr << "trying dir " << state.direction;

	if (steps > 0) {
		perr << ", " << steps << " steps";
	}
	perr << " from ("
	     << oldnode->state.x << "," << oldnode->state.y << ") to ("
	     << newnode->state.x << "," << newnode->state.y
	     << "), cost = " << newnode->cost << ", heurtotcost = "
	     << newnode->heuristicTotalCost << std::endl;
#endif

#ifdef DEBUG
	if (actor->getObjId() == visualdebug_actor) {
		RenderSurface *screen = Ultima8Engine::get_instance()->getScreen();
		screen->BeginPainting();
		drawpath(newnode, 0xFFFFFF00, done);
		screen->EndPainting();
		SDL_Delay(250);
		if (!done) {
			screen->BeginPainting();
			drawpath(newnode, 0xFFB0B000, done);
			screen->EndPainting();
		}
	}
#endif

	nodes.push(newnode);
}

void Pathfinder::expandNode(PathNode *node) {
	Animation::Sequence walkanim = Animation::walk;
	PathfindingState state, closeststate;
	AnimationTracker tracker;
	expandednodes++;

	if (actor->isInCombat())
		walkanim = Animation::advance;

	// try walking in all 8 directions
	for (uint32 dir = 0; dir < 8; ++dir) {
		state = node->state;
		state.lastanim = walkanim;
		state.direction = dir;
		state.combat = actor->isInCombat();
		uint32 steps = 0, beststeps = 0;
		int bestsqdist;
		bestsqdist = (targetx - node->state.x + actor_xd / 2) *
		             (targetx - node->state.x + actor_xd / 2);
		bestsqdist += (targety - node->state.y + actor_yd / 2) *
		              (targety - node->state.y + actor_yd / 2);

		if (!tracker.init(actor, walkanim, dir, &state)) continue;

		// determine how far the actor will travel if the animation runs to completion
		int32 max_endx, max_endy;
		tracker.evaluateMaxAnimTravel(max_endx, max_endy, dir);
		if (alreadyVisited(max_endx, max_endy, state.z)) continue;
		int sqrddist;
		int x_travel = ABS(max_endx - state.x);
		int xy_maxtravel = x_travel;    // don't have the max(a,b) macro...
		int y_travel = ABS(max_endy - state.y);
		if (y_travel > xy_maxtravel) xy_maxtravel = y_travel;

		sqrddist = x_travel * x_travel + y_travel * y_travel;
		if (sqrddist > 400) {
			// range is greater than 20; see if a node has been visited at range 10
			if (alreadyVisited(state.x + x_travel * 10 / xy_maxtravel,
			                   state.y + y_travel * 10 / xy_maxtravel,
			                   state.z)) {
				continue;
			}
		}
		while (tracker.step()) {
			steps++;
			tracker.updateState(state);

			sqrddist = (targetx - state.x + actor_xd / 2) *
			           (targetx - state.x + actor_xd / 2);
			sqrddist += (targety - state.y + actor_yd / 2) *
			            (targety - state.y + actor_yd / 2);

			if (sqrddist < bestsqdist) {
				bestsqdist = sqrddist;
				beststeps = steps;
				closeststate = state;
			}
		}

		if (tracker.isDone()) {
			tracker.updateState(state);
			if (!alreadyVisited(state.x, state.y, state.z)) {
				newNode(node, state, 0);
				visited.push_back(state);
			}
		} else {
			// an obstruction was encountered, so generate a visited node to block
			// future evaluation at the endpoint.
			visited.push_back(state);
		}

		// TODO: maybe only allow partial steps close to target?
		if (beststeps != 0 && (beststeps != steps ||
		                       (!tracker.isDone() && targetitem))) {
			newNode(node, closeststate, beststeps);
			visited.push_back(closeststate);
		}
	}
}

bool Pathfinder::pathfind(std::vector<PathfindingAction> &path) {
#if 0
	pout << "Actor " << actor->getObjId();

	if (targetitem) {
		pout << " pathfinding to item: ";
		targetitem->dumpInfo();
	} else {
		pout << " pathfinding to (" << targetx << "," << targety << "," << targetz << ")" << std::endl;
	}
#endif

#ifdef DEBUG
	if (actor->getObjId() == visualdebug_actor) {
		RenderSurface *screen = Ultima8Engine::get_instance()->getScreen();
		screen->BeginPainting();
		if (targetitem)
			drawbox(targetitem);
		else
			drawdot(targetx, targety, targetz, 2, 0xFF0000FF);
		screen->EndPainting();
	}
#endif


	path.clear();

	PathNode *startnode = new PathNode();
	startnode->state = start;
	startnode->cost = 0;
	startnode->parent = 0;
	startnode->depth = 0;
	startnode->stepsfromparent = 0;
	nodelist.push_back(startnode);
	nodes.push(startnode);

	unsigned int expandedNodes = 0;
	const unsigned int NODELIMIT_MIN = 30;  //! constant
	const unsigned int NODELIMIT_MAX = 200; //! constant
	bool found = false;
	uint32 starttime = g_system->getMillis();

	while (expandedNodes < NODELIMIT_MAX && !nodes.empty() && !found) {
		PathNode *node = nodes.top();
		nodes.pop();

#if 0
		pout << "Trying node: (" << node->state.x << "," << node->state.y
		     << "," << node->state.z << ") target=(" << targetx << ","
		     << targety << "," << targetz << ")" << std::endl;
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
			     << std::endl;
#endif

			unsigned int i = length;
			if (length > 0) length++; // add space for final 'stand' action
			path.resize(length);

			// now backtrack through the nodes to assemble the final animation
			while (node->parent) {
				PathfindingAction action;
				action.action = node->state.lastanim;
				action.direction = node->state.direction;
				action.steps = node->stepsfromparent;
				path[--i] = action;
#if 0
				pout << "anim = " << node->state.lastanim << ", dir = "
				     << node->state.direction << ", steps = "
				     << node->stepsfromparent << std::endl;
#endif

				//TODO: check how turns work
				//TODO: append final 'stand' animation

				node = node->parent;
			}

			if (length) {
				if (node->state.combat)
					path[length - 1].action = Animation::combatStand;
				else
					path[length - 1].action = Animation::stand;
				path[length - 1].direction = path[length - 2].direction;
			}

			expandtime = g_system->getMillis() - starttime;
			return true;
		}

		expandNode(node);
		expandedNodes++;

		if (expandedNodes >= NODELIMIT_MIN && ((expandedNodes) % 5) == 0) {
			uint32 elapsed_ms = g_system->getMillis() - starttime;
			if (elapsed_ms > 350) break;
		}
	}

	expandtime = g_system->getMillis() - starttime;

#if 0
	static int32 pfcalls = 0;
	static int32 pftotaltime = 0;
	pfcalls++;
	pftotaltime += expandtime;
	pout << "maxout average = " << (pftotaltime / pfcalls) << "ms." << std::endl;
#endif

	return false;
}


#ifdef DEBUG
void Pathfinder::ConCmd_visualDebug(const Console::ArgvType &argv) {
	if (argv.size() != 2) {
		pout << "Usage: Pathfinder::visualDebug objid" << std::endl;
		pout << "Specify objid -1 to stop tracing." << std::endl;
		return;
	}
	int p = strtol(argv[1].c_str(), 0, 0);
	if (p == -1) {
		visualdebug_actor = 0xFFFF;
		pout << "Pathfinder: stopped visual tracing" << std::endl;
	} else {
		visualdebug_actor = (uint16)p;
		pout << "Pathfinder: visually tracing actor " << visualdebug_actor << std::endl;
	}
}
#endif

} // End of namespace Ultima8
} // End of namespace Ultima
