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

#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/pathfinder/dir_finder.h"
#include "ultima/nuvie/pathfinder/seek_path.h"

namespace Ultima {
namespace Nuvie {

using Std::vector;

SeekPath::SeekPath() {

}

SeekPath::~SeekPath() {

}

/* Get two relative directions that a line can travel to trace around an
   obstacle towards `xdir',`ydir'. */
bool SeekPath::get_obstacle_tracer(MapCoord &start, sint32 xdir, sint32 ydir,
                                   sint32 &Axdir, sint32 &Aydir,
                                   sint32 &Bxdir, sint32 &Bydir) {
	if (xdir && ydir) { // original direction is diagonal
		MapCoord checkA(start.x + xdir, start.y, start.z);
		MapCoord checkB(start.x, start.y + ydir, start.z);
		if (check_loc(checkA)) { // can go in X
			Axdir = xdir;
			Aydir = 0; // Horizontal; in X direction
		} else { // X is blocked, must go in Y
			Axdir = 0;
			Aydir = -ydir; // Vertical; opposite Y direction
		}
		if (check_loc(checkB)) { // can go in Y
			Bxdir = 0;
			Bydir = ydir; // Vertical; in Y direction
		} else { // Y is blocked, must go in X
			Bxdir = -xdir;
			Bydir = 0; // Horizontal; opposite X direction
		}
	} else { // orthagonal
		// scan in perpendicular straight line
		Axdir = ydir;
		Aydir = xdir;
		Bxdir = -Axdir;
		Bydir = -Aydir;
	}
	return false;
}

/* Returns true if an opening is found along the original line. */
bool SeekPath::trace_check_obstacle(bool &turned, MapCoord &line, sint32 &deltax, sint32 &deltay, sint32 &xdir, sint32 &ydir, Std::vector<MapCoord> *scan) {
	MapCoord obstacle(line.x + xdir, line.y + ydir, line.z);
	if (check_loc(obstacle)) { // no obstacle here; able to move closer
		if (scan->empty() || scan->back() != line)
			scan->push_back(line); // *ADD TRACE NODE*
		if (!turned) {
			scan->push_back(obstacle); // *ADD TRACE NODE*
			return true;
		}
		// bend line TOWARDS obstacle
		line.x += xdir;
		line.y += ydir; // step forward
		sint32 old_deltax = deltax, old_deltay = deltay;
		deltax = xdir;
		deltay = ydir; // now moving in that direction
		xdir = -old_deltax;
		ydir = -old_deltay; // and looking away from old delta
		turned = false;
	}
	return false;
}

void SeekPath::trace_around_corner(MapCoord &line, sint32 &deltax, sint32 &deltay, sint32 &xdir, sint32 &ydir, Std::vector<MapCoord> *scan) {
	line.x -= deltax;
	line.y -= deltay; // step back
	if (scan->empty() || scan->back() != line)
		scan->push_back(line); // *ADD TRACE NODE*
	sint8 old_xdir = xdir, old_ydir = ydir;
	xdir = deltax;
	ydir = deltay; // now looking in that direction
	deltax = -old_xdir;
	deltay = -old_ydir; // and moving scan away from old obstacle
}

/* Trace an obstacle from 'start' towards the direction 'deltax' and 'deltay',
   looking for openings towards 'xdir' and 'ydir'. The scan can bend 90 degrees
   to get around walls. Returns true if something that looks like an opening
   has been found. Trace nodes are placed at turns and where the scan ends. */
bool SeekPath::trace_obstacle(MapCoord line, sint32 deltax, sint32 deltay, sint32 xdir, sint32 ydir, Std::vector<MapCoord> *scan) {
	const uint32 scan_max = 8; // number of squares to check before giving up
	bool bend = false; // true if the scanning line is rotated 90 degrees
	uint32 s = 0;
	do {
		line.x += deltax;
		line.y += deltay;
		if (!check_loc(line)) {
			if (!bend) { // bend line AWAY from obstacle
				trace_around_corner(line, deltax, deltay, xdir, ydir, scan);
				bend = true;
			} else // blocked (we only allow one turn)
				break;
		} else if (trace_check_obstacle(bend, line, deltax, deltay, xdir, ydir, scan))
			return true;
	} while (++s < scan_max);
	scan->resize(0);
	return false;
}

// choose which set of nodes traced around an obstacle should be used for a path
Std::vector<MapCoord> *SeekPath::get_best_scan(MapCoord &start, MapCoord &goal) {
	if (A_scan.empty() && B_scan.empty())
		return 0;
	if (A_scan.empty())
		return &B_scan;
	if (B_scan.empty())
		return &A_scan;
	if (B_scan.back().distance(goal) < A_scan.back().distance(goal))
		return &B_scan;
	return &A_scan;
}

// copy A or B nodes to the path
void SeekPath::create_path(MapCoord &start, MapCoord &goal) {
	vector<MapCoord> *nodes = get_best_scan(start, goal); // points to line A or B
	MapCoord prev_node(start);

	// these nodes are only at certain locations in the path, so all steps in
	// between have to be added
	while (nodes && !nodes->empty()) {
		// create steps from prev_node to this_node
		MapCoord this_node = nodes->front();
		nodes->erase(nodes->begin());
		if (this_node == start) // start is the first prev_node, which results in duplicate steps
			continue;
		sint16 dx = clamp(this_node.x - prev_node.x, -1, 1), dy = clamp(this_node.y - prev_node.y, -1, 1);
		do {
			prev_node = prev_node.abs_coords(dx, dy); // add dx & dy
			add_step(prev_node);
		} while (prev_node != this_node);

		prev_node = this_node;
	}
}

/* Returns true if a path is found around the obstacle between locations. */
bool SeekPath::path_search(MapCoord &start, MapCoord &goal) {
	sint8 xdir = 0, ydir = 0; // direction start->goal
	DirFinder::get_normalized_dir(start, goal, xdir, ydir); // init xdir & ydir

	// confirm that goal is more than one square away
	if ((start.x + xdir) == goal.x && (start.y + ydir) == goal.y)
		return false;

	// determine if each line (A and B) will be vertical or horizontal
	sint32 Ax = 0, Ay = 0, Bx = 0, By = 0; // vector of line segments to scan
	get_obstacle_tracer(start, xdir, ydir, Ax, Ay, Bx, By);

	// direction from line to scan is perpendicular to line, towards obstacle
	delete_nodes();
	bool successA = trace_obstacle(start, Ax, Ay, (Ay) ? xdir : 0, (Ax) ? ydir : 0, &A_scan);
	bool successB = trace_obstacle(start, Bx, By, (By) ? xdir : 0, (Bx) ? ydir : 0, &B_scan);
	if (successA || successB)
		create_path(start, goal); // create path from available nodes
	delete_nodes();
	return (successA || successB);
}

void SeekPath::delete_nodes() {
	A_scan.clear();
	B_scan.clear();
}

} // End of namespace Nuvie
} // End of namespace Ultima
