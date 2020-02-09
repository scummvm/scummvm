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

#ifndef NUVIE_PATHFINDER_COMBAT_PATH_FINDER_H
#define NUVIE_PATHFINDER_COMBAT_PATH_FINDER_H

#include "ultima/nuvie/pathfinder/actor_path_finder.h"

namespace Ultima {
namespace Nuvie {

typedef enum {
	PATHFINDER_NONE,
	PATHFINDER_CHASE,
	PATHFINDER_FLEE
} CombatPathFinderMode;

class CombatPathFinder: public ActorPathFinder {
protected:
	Actor *target;
	CombatPathFinderMode target_mode;

	bool update_location() override;

	uint8 max_dist;

public:
	CombatPathFinder(Actor *a);
	CombatPathFinder(Actor *a, Actor *t);
	~CombatPathFinder() override;
	bool set_flee_mode(Actor *actor);
	bool set_chase_mode(Actor *actor);
	bool set_mode(CombatPathFinderMode mode, Actor *actor);
	void set_distance(uint8 dist) {
		max_dist = dist;
	}

	bool get_next_move(MapCoord &step) override;
	bool reached_goal() override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
