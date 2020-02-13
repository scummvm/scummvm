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

#ifndef NUVIE_PATHFINDER_PATH_FINDER_H
#define NUVIE_PATHFINDER_PATH_FINDER_H

#include "ultima/nuvie/core/map.h"

namespace Ultima {
namespace Nuvie {

class Path;

class PathFinder {
protected:
	MapCoord start, goal, loc; /* source, destination, current location */

	Path *search; /* contains path-search algorithms, and
                                 game-specific step costs */

	void new_search(Path *new_path);
//    bool is_hazardous(MapCoord &loc); will have to check objects and tiles

public:
	PathFinder();
	PathFinder(MapCoord s, MapCoord g);
	virtual ~PathFinder();
	void set_search(Path *new_path) {
		new_search(new_path);
	}

	virtual void set_start(const MapCoord &s);
	virtual void set_goal(const MapCoord &g);
	virtual void set_location(const MapCoord &l) {
		loc = l;
	}

	virtual MapCoord get_location() {
		return loc;
	}
	virtual MapCoord get_goal()     {
		return goal;
	}
	virtual bool reached_goal()     {
		return (loc.x == goal.x && loc.y == goal.y
		        && loc.z == goal.z);
	}

	virtual bool check_dir(const MapCoord &from, MapCoord &rel, sint8 unused = 0);
	virtual bool check_loc(const MapCoord &loc) = 0;
	bool check_loc(uint16 x, uint16 y, uint8 z);

	virtual bool find_path(); /* get path to goal if one doesn't already exist */
	virtual bool have_path(); /* a working path exists */
	virtual bool is_path_clear(); /* recheck each location in path */
	virtual bool get_next_move(MapCoord &step) = 0;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
