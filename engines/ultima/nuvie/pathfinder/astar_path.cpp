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

#include "ultima/shared/std/containers.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/pathfinder/dir_finder.h"
#include "ultima/nuvie/pathfinder/astar_path.h"

namespace Ultima {
namespace Nuvie {

AStarPath::AStarPath() : final_node(0) {
} void AStarPath::create_path() {
	astar_node *i = final_node; // iterator through steps, from back
	delete_path();
	Std::vector<astar_node *> reverse_list;
	while (i) {
		reverse_list.push_back(i);
		i = i->parent;
	}
	while (!reverse_list.empty()) {
		i = reverse_list.back();
		add_step(i->loc);
		reverse_list.pop_back();
	}
	set_path_size(step_count);
}/* Get a new neighbor to nnode and score it, returning true if it's usable. */
bool AStarPath::score_to_neighbor(sint8 dir, astar_node *nnode, astar_node *neighbor,
                                  sint32 &nnode_to_neighbor) {
	sint8 sx = -1, sy = -1;
	DirFinder::get_adjacent_dir(sx, sy, dir); // sx,sy = neighbor -1,-1 + dir
	// get neighbor of nnode towards sx,sy, and cost to that neighbor
	neighbor->loc = nnode->loc.abs_coords(sx, sy);
	nnode_to_neighbor = step_cost(nnode->loc, neighbor->loc);
	if (nnode_to_neighbor == -1) {
		delete neighbor; // this neighbor is blocked
		return false;
	}
	return true;
}/* Compare a node's score to the start node to already scored neighbors. */
bool AStarPath::compare_neighbors(astar_node *nnode, astar_node *neighbor,
                                  sint32 nnode_to_neighbor, astar_node *in_open,
                                  astar_node *in_closed) {
	neighbor->to_start = nnode->to_start + nnode_to_neighbor;
	// ignore this neighbor if already checked and closer to start
	if ((in_open && in_open->to_start <= neighbor->to_start)
	        || (in_closed && in_closed->to_start <= neighbor->to_start)) {
		delete neighbor;
		return false;
	}
	return true;
}/* Check all neighbors of a node (location) and save them to the "seen" list. */
bool AStarPath::search_node_neighbors(astar_node *nnode, MapCoord &goal,
                                      const uint32 max_score) {
	for (uint32 dir = 1; dir < 8; dir += 2) {
		astar_node *neighbor = new astar_node;
		sint32 nnode_to_neighbor = -1;
		if (!score_to_neighbor(dir, nnode, neighbor, nnode_to_neighbor))
			continue; // this neighbor is blocked
		astar_node *in_open = find_open_node(neighbor),
		            *in_closed = find_closed_node(neighbor);
		if (!compare_neighbors(nnode, neighbor, nnode_to_neighbor, in_open, in_closed))
			continue;
		neighbor->parent = nnode;
		neighbor->to_goal = path_cost_est(neighbor->loc, goal);
		neighbor->score = neighbor->to_start + neighbor->to_goal;
		neighbor->len = nnode->len + 1;
		if (neighbor->score > max_score) {
			delete neighbor; // too far away
			continue;
		}
		// take neighbor out of closed list and put into open list
		if (in_closed)
			remove_closed_node(in_closed);
		if (!in_open)
			push_open_node(neighbor);
	}
	return true;
}/* Do A* search of tiles to create a path from `start' to `goal'.
 * Don't search past nodes with a score over the max. score.
 * Create a partial path to low-score nodes with a distance-to-start over the
 * max_steps count, defined here. Actor may perform another search when needed.
 * Returns true if a path is created
 */bool AStarPath::path_search(MapCoord &start, MapCoord &goal) {
	//DEBUG(0,LEVEL_DEBUGGING,"SEARCH: %d: %d,%d -> %d,%d\n",actor->get_actor_num(),start.x,start.y,goal.x,goal.y);
	astar_node *start_node = new astar_node;
	start_node->loc = start;
	start_node->to_start = 0;
	start_node->to_goal = path_cost_est(start, goal);
	start_node->score = start_node->to_start + start_node->to_goal;
	start_node->len = 0;
	push_open_node(start_node);
	const uint32 max_score = get_max_score(start_node->to_goal);
	const uint32 max_steps = 8 * 2 * 4; // walk up to four screen lengths before searching again
	while (!open_nodes.empty()) {
		astar_node *nnode = pop_open_node(); // next closest
		if (nnode->loc == goal || nnode->len >= max_steps) {
			if (nnode->loc != goal)
				DEBUG(0, LEVEL_DEBUGGING, "out of steps, making partial path (nnode->len=%d)\n", nnode->len);
//DEBUG(0,LEVEL_DEBUGGING,"GOAL\n");
			final_node = nnode;
			create_path();
			delete_nodes();
			return (true); // reached goal - success
		}
		// check cardinal neighbors (starting at top going clockwise)
		search_node_neighbors(nnode, goal, max_score);
		// node and neighbors checked, put into closed
		closed_nodes.push_back(nnode);
	}
//DEBUG(0,LEVEL_DEBUGGING,"FAIL\n");
	delete_nodes();
	return (false); // out of open nodes - failure
}/* Return the cost of moving one step from `c1' to `c2', which is always 1. This
 * isn't very helpful, so subclasses should provide their own function.
 * Returns -1 if c2 is blocked. */
sint32 AStarPath::step_cost(MapCoord &c1, MapCoord &c2) {
	if (!pf->check_loc(c2.x, c2.y, c2.z)
	        || c2.distance(c1) > 1)
		return (-1);
	return (1);
}/* Return an item in the list of closed nodes whose location matches `ncmp'.
 */astar_node *AStarPath::find_closed_node(astar_node *ncmp) {
	Std::list<astar_node *>::iterator n;
	for (n = closed_nodes.begin(); n != closed_nodes.end(); n++)
		if ((*n)->loc == ncmp->loc)
			return (*n);
	return (NULL);
}/* Return an item in the list of closed nodes whose location matches `ncmp'.
 */astar_node *AStarPath::find_open_node(astar_node *ncmp) {
	Std::list<astar_node *>::iterator n;
	for (n = open_nodes.begin(); n != open_nodes.end(); n++)
		if ((*n)->loc == ncmp->loc)
			return (*n);
	return (NULL);
}/* Add new node pointer to the list of open nodes (sorting by score).
 */void AStarPath::push_open_node(astar_node *node) {
	Std::list<astar_node *>::iterator n, next;
	if (open_nodes.empty()) {
		open_nodes.push_front(node);
		return;
	}
	n = open_nodes.begin();
	// get to end of list or to a node with equal or greater score
	while (n != open_nodes.end() && (*n++)->score < node->score);
	open_nodes.insert(n, node); // and add before that location
}/* Return pointer to the highest priority node from the list of open nodes, and
 * remove it.
 */astar_node *AStarPath::pop_open_node() {
	astar_node *best = open_nodes.front();
	open_nodes.pop_front(); // remove it
	return (best);
}

/* Find item in the list of closed nodes whose location matched `ncmp', and
 * remove it from the list.
 */
void AStarPath::remove_closed_node(astar_node *ncmp) {
	Std::list<astar_node *>::iterator n;
	for (n = closed_nodes.begin(); n != closed_nodes.end(); n++)
		if ((*n)->loc == ncmp->loc) {
			closed_nodes.erase(n);
			return;
		}
}

/* Delete nodes dereferenced from pointers in the lists.
 */
void AStarPath::delete_nodes() {
	while (!open_nodes.empty()) {
		astar_node *delnode = open_nodes.front();
		open_nodes.pop_front();
		delete delnode;
	}
	while (!closed_nodes.empty()) {
		astar_node *delnode = closed_nodes.front();
		closed_nodes.pop_front();
		delete delnode;
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
