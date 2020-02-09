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

#ifndef NUVIE_PATHFINDER_ASTAR_PATH_H
#define NUVIE_PATHFINDER_ASTAR_PATH_H

#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/pathfinder/path.h"

namespace Ultima {
namespace Nuvie {

typedef struct astar_node_s {
	MapCoord loc; // location
	uint32 to_start; // costs from this node to start and to goal
	uint32 to_goal;
	uint32 score; // node score
	uint32 len; // number of nodes before this one, regardless of score
	struct astar_node_s *parent;
	astar_node_s() : loc(0, 0, 0), to_start(0), to_goal(0), score(0), len(0),
		parent(NULL) { }
} astar_node;
/* Provides A* search and cost methods for PathFinder and subclasses.
 */class AStarPath: public Path {
protected:
	Std::list<astar_node *> open_nodes, closed_nodes; // nodes seen
	astar_node *final_node; // last node in path search, used by create_path()
	/* Forms a usable path from results of a search. */
	void create_path();
	/* Search routine. */
	bool search_node_neighbors(astar_node *nnode, MapCoord &goal, const uint32 max_score);
	bool compare_neighbors(astar_node *nnode, astar_node *neighbor,
	                       sint32 nnode_to_neighbor, astar_node *in_open,
	                       astar_node *in_closed);
	bool score_to_neighbor(sint8 dir, astar_node *nnode, astar_node *neighbor,
	                       sint32 &nnode_to_neighbor);
public:
	AStarPath();
	~AStarPath() override { }
	bool path_search(MapCoord &start, MapCoord &goal) override;
	uint32 path_cost_est(MapCoord &s, MapCoord &g) override  {
		return (Path::path_cost_est(s, g));
	}
	uint32 get_max_score(uint32 cost) override {
		return (Path::get_max_score(cost));
	}
	uint32 path_cost_est(astar_node &n1, astar_node &n2) {
		return (Path::path_cost_est(n1.loc, n2.loc));
	}
	sint32 step_cost(MapCoord &c1, MapCoord &c2) override;
protected:
	/* FIXME: These node functions can be replaced with a priority_queue and a list. */
	astar_node *find_open_node(astar_node *ncmp);
	void push_open_node(astar_node *node);
	astar_node *pop_open_node();
	astar_node *find_closed_node(astar_node *ncmp);
	void remove_closed_node(astar_node *ncmp);
	void delete_nodes();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
