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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/pathfinder/path.h"
#include "ultima/nuvie/pathfinder/actor_path_finder.h"

namespace Ultima {
namespace Nuvie {

ActorPathFinder::ActorPathFinder(Actor *a, MapCoord g)
	: PathFinder(a->get_location(), g), actor(a) {

}

ActorPathFinder::~ActorPathFinder() {

}

bool ActorPathFinder::get_next_move(MapCoord &step) {
	MapCoord rel_step;
	if (have_path()) {
		step = search->get_first_step();
		return check_loc(step);
	}

	get_closest_dir(rel_step);
	if (check_dir(loc, rel_step)) {
		step = loc.abs_coords(rel_step.sx, rel_step.sy);
		return true;
	}
	if (search_towards_target(goal, rel_step)) {
		step = loc.abs_coords(rel_step.sx, rel_step.sy);
		return true;
	}

	if (find_path()) {
		step = search->get_first_step();
		return check_loc(step);
	}
	return false;
}

/* Get relative direction from Loc to Goal and place in Rel_step. */
void ActorPathFinder::get_closest_dir(MapCoord &rel_step) {
	rel_step.sx = clamp(goal.x - loc.x, -1, 1);
	rel_step.sy = clamp(goal.y - loc.y, -1, 1);
	rel_step.z = loc.z;

	uint16 dx = loc.xdistance(goal), dy = loc.ydistance(goal);
	if (dx > dy) rel_step.sy = 0;
	else if (dx < dy) rel_step.sx = 0;
}

bool ActorPathFinder::check_loc(const MapCoord &mapLoc) {
	return actor->check_move(mapLoc.x, mapLoc.y, mapLoc.z);
}

/* Find a move from actor to g, starting with rel_step. Replace
 * rel_step with the result. */
bool ActorPathFinder::search_towards_target(const MapCoord &g, MapCoord &rel_step) {
	MapCoord mapLoc = actor->get_location();
	MapCoord ccw_rel_step = rel_step, cw_rel_step = rel_step;
	if (check_dir(mapLoc, rel_step)) // check original direction
		return true;
	bool try_ccw = check_dir_and_distance(mapLoc, g, ccw_rel_step, -1); // check adjacent directions
	bool try_cw = check_dir_and_distance(mapLoc, g, cw_rel_step, 1);
	if (!try_ccw) try_ccw = check_dir_and_distance(mapLoc, g, ccw_rel_step, -2); // check perpendicular directions
	if (!try_cw) try_cw = check_dir_and_distance(mapLoc, g, cw_rel_step, 2);
	if (!try_ccw && !try_cw)
		return false;
	rel_step = ccw_rel_step;
	if (!try_ccw) rel_step = cw_rel_step;
	else if (!try_cw) rel_step = ccw_rel_step;
	else { // both valid, use closest
		MapCoord ccw_step = mapLoc.abs_coords(ccw_rel_step.sx, ccw_rel_step.sy);
		MapCoord cw_step = mapLoc.abs_coords(cw_rel_step.sx, cw_rel_step.sy);
		MapCoord target(g);
		if (cw_step.distance(target) < ccw_step.distance(target))
			rel_step = cw_rel_step;
	}
	return true;
}

// check rotated dir, and copy results to rel_step if neighbor is passable
bool ActorPathFinder::check_dir_and_distance(MapCoord mapLoc, MapCoord g, MapCoord &rel_step, sint8 rotate) {
	MapCoord rel_step_2 = rel_step;
	if (check_dir(mapLoc, rel_step_2, rotate)) {
		MapCoord neighbor = mapLoc.abs_coords(rel_step_2.sx, rel_step_2.sy);
		if (neighbor.distance(g) <= mapLoc.distance(g)) {
			rel_step = rel_step_2;
			return true;
		}
	}
	return false;
}

// new direction is copied to rel if true
bool ActorPathFinder::check_dir(const MapCoord &mapLoc, MapCoord &rel, sint8 rot) {
	sint8 xdir = rel.sx, ydir = rel.sy;
	get_adjacent_dir(xdir, ydir, rot);
	MapCoord new_loc = MapCoord(mapLoc).abs_coords(xdir, ydir);
	if (check_loc(new_loc)) {
		rel.sx = xdir;
		rel.sy = ydir;
		return true;
	}
	return false;
}

void ActorPathFinder::actor_moved() {
	update_location();
	// pop step
	if (have_path())
		search->remove_first_step();
}

void ActorPathFinder::set_actor(Actor *a) {
	actor = a;
}

bool ActorPathFinder::update_location() {
	if (!actor)
		return false;
	actor->get_location(&loc.x, &loc.y, &loc.z);
	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
