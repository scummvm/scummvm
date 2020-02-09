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

#ifndef NUVIE_PATHFINDER_SCHED_PATH_FINDER_H
#define NUVIE_PATHFINDER_SCHED_PATH_FINDER_H

#include "ultima/nuvie/pathfinder/actor_path_finder.h"

namespace Ultima {
namespace Nuvie {

/* Long-range pathfinder for NPCs.
 */
class SchedPathFinder: public ActorPathFinder {
protected:
	uint32 prev_step_i, next_step_i; /* step counters */

public:
	/* Pass 'path_type' to define search rules and methods to be used. The
	   PathFinder is responsible for deleting it when finished. */
	SchedPathFinder(Actor *a, MapCoord g, Path *path_type);
	~SchedPathFinder() override;

	bool get_next_move(MapCoord &step) override; /* returns the next step in the path */
	bool find_path() override; /* gets a NEW path from location->goal */
	void actor_moved() override; /* update location and step counters */

	bool check_loc(const MapCoord &loc) override; // ignores other actors
protected:
	bool is_location_in_path();
	void incr_step();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
