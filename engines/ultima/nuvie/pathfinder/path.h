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

#ifndef NUVIE_PATHFINDER_PATH_H
#define NUVIE_PATHFINDER_PATH_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/pathfinder/path_finder.h"

namespace Ultima {
namespace Nuvie {

class MapCoord;

/* Abstract for a class that provides the path-search routines and cost methods
 * to a PathFinder. This includes useful default methods, and path handling.
 */
class Path {
protected:
	MapCoord *path; // list of tiles in the path, set by create_path()
	uint32 step_count; // number of locations in the path
	uint32 path_size; // allocated elements in list
	PathFinder *pf;

	void add_step(MapCoord loc);
	bool check_dir(const MapCoord &loc, MapCoord &rel);
	bool check_loc(const MapCoord &loc);
	void set_path_size(int alloc_size);

public:
	Path();
	virtual ~Path();
	void set_pathfinder(PathFinder *pathfinder) {
		pf = pathfinder;
	}

	/* The pathfinding routine. Can return success or failure of a search. */
	virtual bool path_search(MapCoord &start, MapCoord &goal) = 0;
	void delete_path();
	virtual bool have_path();

	/* Returns the real cost of moving from a node (or location) to a
	   neighboring node. (a single step) */
	virtual sint32 step_cost(MapCoord &c1, MapCoord &c2) = 0;

	/* Estimate highest possible cost from s to g */
	virtual uint32 path_cost_est(MapCoord &s, MapCoord &g);
	/* Returns maximum score of any single node in the search of a path with
	   a certain estimated cost.*/
	virtual uint32 get_max_score(uint32 cost);

	virtual MapCoord get_first_step();
	virtual MapCoord get_last_step();
	virtual MapCoord get_step(uint32 step_index);
	virtual void get_path(MapCoord **path_start, uint32 &path_size);
	uint32 get_num_steps() {
		return step_count;
	}

	virtual bool remove_first_step();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
