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

#ifndef NUVIE_PATHFINDER_SEEK_PATH_H
#define NUVIE_PATHFINDER_SEEK_PATH_H

#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/pathfinder/path.h"

namespace Ultima {
namespace Nuvie {

/* Provides routines for building short paths around obstacles and seeking a
 * target. Much of the work doesn't involve finding a path at all, but instead
 * finding the direction closest to the target.
 */
class SeekPath: public Path {
protected:
	Std::vector<MapCoord> A_scan, B_scan; // nodes of a line scanned by trace_obstacle()

	void create_path(MapCoord &start, MapCoord &goal);
	Std::vector<MapCoord> *get_best_scan(MapCoord &start, MapCoord &goal);
	void delete_nodes();
	bool trace_check_obstacle(bool &turned, MapCoord &line, sint32 &deltax, sint32 &deltay, sint32 &xdir, sint32 &ydir, Std::vector<MapCoord> *scan);
	void trace_around_corner(MapCoord &line, sint32 &deltax, sint32 &deltay, sint32 &xdir, sint32 &ydir, Std::vector<MapCoord> *scan);

public:
	SeekPath();
	~SeekPath() override;
	sint32 step_cost(MapCoord &c1, MapCoord &c2) override {
		return -1;
	}
	bool path_search(MapCoord &start, MapCoord &goal) override;
	void delete_path() {
		Path::delete_path();
		delete_nodes();
	}

	/* Trace obstacle towards xdir,ydir for a possible opening. */
	bool trace_obstacle(MapCoord line, sint32 deltax, sint32 deltay, sint32 xdir, sint32 ydir, Std::vector<MapCoord> *scan);
	/* Get two relative directions that a line can travel to trace around an
	   obstacle towards `xdir',`ydir'. */
	bool get_obstacle_tracer(MapCoord &start, sint32 xdir, sint32 ydir,
	                         sint32 &Axdir, sint32 &Aydir,
	                         sint32 &Bxdir, sint32 &Bydir);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
