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

#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/pathfinder/path.h"

namespace Ultima {
namespace Nuvie {

Path::Path()
	: path(0), step_count(0), path_size(0), pf(0) {
}

Path::~Path() {
	delete_path();
}

void Path::set_path_size(int alloc_size) {
	path_size = alloc_size;
	path = (MapCoord *)nuvie_realloc(path, path_size * sizeof(MapCoord));
}

/* Take estimate of a path, and return the highest allowed score of any nodes
 * in the search of that path.
 */
uint32 Path::get_max_score(uint32 cost) {
	uint32 max_score = cost * 2;
	// search at least this far (else short paths will have too
	//                           low of a maximum score to move around walls)
	if (max_score < 8 * 2 * 3)
		max_score = 8 * 2 * 3;
	return (max_score);
}

/* Return a weighted estimate of the highest cost from location `s' to `g'.
 */
uint32 Path::path_cost_est(MapCoord &s, MapCoord &g) {
	uint32 major = (s.xdistance(g) >= s.ydistance(g))
	               ? s.xdistance(g) : s.ydistance(g);
	uint32 minor = (s.xdistance(g) >= s.ydistance(g))
	               ? s.ydistance(g) : s.xdistance(g);
	return (2 * major + minor);
}

/* Free and zero path.
 */
void Path::delete_path() {
	if (path)
		free(path);
	path = NULL;
	step_count = 0;
	path_size = 0;
}

MapCoord Path::get_first_step() {
	return (Path::get_step(0));
}

MapCoord Path::get_last_step() {
	return (Path::get_step(step_count - 1));
}

MapCoord Path::get_step(uint32 step_index) {
	MapCoord step(0, 0, 0);
	step = path[step_index];
	return (step);
}

bool Path::have_path() {
	return (path && step_count > 0);
}

void Path::get_path(MapCoord **path_start, uint32 &pathSize) {
	if (path_start)
		*path_start = path;
	pathSize = step_count;
}

/* Increases path size in blocks and adds a step to the end of the path. */
void Path::add_step(MapCoord loc) {
	const int path_block_size = 8;
	if (step_count >= path_size) {
		path_size += path_block_size;
		path = (MapCoord *)nuvie_realloc(path, path_size * sizeof(MapCoord));
	}
	path[step_count++] = loc;
}

bool Path::remove_first_step() {
	if (have_path()) {
		step_count -= 1;
		path_size = step_count;
		MapCoord *new_path = (MapCoord *)malloc(path_size * sizeof(MapCoord));
		memcpy(new_path, &(path[1]), step_count * sizeof(MapCoord));
		free(path);
		path = new_path;
		return true;
	}
	return false;
}

bool Path::check_dir(const MapCoord &loc, MapCoord &rel) {
	return pf->check_dir(loc, rel);
}

bool Path::check_loc(const MapCoord &loc) {
	return pf->check_loc(loc);
}

} // End of namespace Nuvie
} // End of namespace Ultima
