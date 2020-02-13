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

#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/pathfinder/combat_path_finder.h"

namespace Ultima {
namespace Nuvie {

CombatPathFinder::CombatPathFinder(Actor *a)
	: ActorPathFinder(a, a->get_location()) {
	target_mode = PATHFINDER_NONE;
	max_dist = 0;
	target = NULL;
}

/* Without a mode set, CombatPathFinder is identical to ActorPathFinder. */
CombatPathFinder::CombatPathFinder(Actor *a, Actor *t)
	: ActorPathFinder(a, t->get_location()) {
	target_mode = PATHFINDER_CHASE;
	target = t;
	max_dist = 0;
}

CombatPathFinder::~CombatPathFinder() {

}

bool CombatPathFinder::reached_goal() {
	if (target_mode == PATHFINDER_CHASE)
		return (loc.distance(goal) <= 1);
	if (target_mode == PATHFINDER_FLEE)
		return (max_dist != 0 && loc.distance(goal) > max_dist);
	return true;
}

bool CombatPathFinder::set_flee_mode(Actor *targetActor) {
	target_mode = PATHFINDER_FLEE;
	target = targetActor;
	update_location();
	return true;
}

bool CombatPathFinder::set_chase_mode(Actor *targetActor) {
	target_mode = PATHFINDER_CHASE;
	target = targetActor;
	update_location();
	return true;
}

bool CombatPathFinder::set_mode(CombatPathFinderMode mode, Actor *targetActor) {
	target_mode = mode;
	target = targetActor;
	return true;
}

bool CombatPathFinder::update_location() {
	ActorPathFinder::update_location();
	set_goal(target->get_location());
	if (max_dist != 0 && loc.distance(goal) > max_dist)
		target_mode = PATHFINDER_NONE;
	return true;
}

bool CombatPathFinder::get_next_move(MapCoord &step) {
	if (target_mode == PATHFINDER_CHASE)
		return ActorPathFinder::get_next_move(step);
	if (target_mode == PATHFINDER_FLEE) {
		get_closest_dir(step);
		step.sx = -step.sx;
		step.sy = -step.sy;
		if (check_dir(loc, step)) {
			step = loc.abs_coords(step.sx, step.sy);
			return true;
		}
	}
	return false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
