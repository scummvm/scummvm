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

#include "ultima/nuvie/pathfinder/path.h"
#include "ultima/nuvie/pathfinder/path_finder.h"

namespace Ultima {
namespace Nuvie {

PathFinder::PathFinder() : start(0, 0, 0), goal(0, 0, 0), loc(0, 0, 0), search(0) {

}

PathFinder::PathFinder(MapCoord s, MapCoord g)
	: start(s), goal(g), loc(0, 0, 0), search(0) {

}

PathFinder::~PathFinder() {
	delete search;
}

bool PathFinder::check_dir(const MapCoord &from, MapCoord &rel, sint8) {
	return check_loc(MapCoord(from.x + rel.sx, from.y + rel.sy, from.z));
}

bool PathFinder::check_loc(uint16 x, uint16 y, uint8 z) {
	return check_loc(MapCoord(x, y, z));
}

void PathFinder::new_search(Path *new_path) {
	delete search;
	search = new_path;
	search->set_pathfinder(this);
}

bool PathFinder::find_path() {
	if (search) {
		if (search->have_path())
			search->delete_path();
		return (search->path_search(loc, goal));
	}
	return false; // no path-search object
}

bool PathFinder::have_path() {
	return (search && search->have_path());
}

void PathFinder::set_goal(const MapCoord &g) {
	goal = g;
	if (have_path())
		search->delete_path();
}

void PathFinder::set_start(const MapCoord &s) {
	start = s;
	if (have_path())
		search->delete_path();
}

bool PathFinder::is_path_clear() {
	uint32 num_steps = search->get_num_steps();
	for (unsigned int n = 0; n < num_steps; n++) {
		MapCoord pos = search->get_step(n);
		if (!check_loc(pos))
			return false;
	}
	return true;
}

} // End of namespace Nuvie
} // End of namespace Ultima
