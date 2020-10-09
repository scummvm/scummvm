/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_MOVEMENT_SHORTEST_PATH_H
#define STARK_MOVEMENT_SHORTEST_PATH_H

#include "common/list.h"
#include "common/hashmap.h"

namespace Stark {

namespace Resources {
class FloorEdge;
}

/**
 * Find the shortest path between two nodes in a graph
 *
 * This is an implementation of Dijsktra's search algorithm
 */
class ShortestPath {
public:
	typedef Common::List<const Resources::FloorEdge *> NodeList;

	/** Computes the shortest path between the start and the goal graph nodes */
	NodeList search(const Resources::FloorEdge *start, const Resources::FloorEdge *goal);

private:
	typedef Common::HashMap<const Resources::FloorEdge *, const Resources::FloorEdge *> NodePrecedenceMap;
	typedef Common::HashMap<const Resources::FloorEdge *, float> NodeCostMap;

	const Resources::FloorEdge *popEdgeWithLowestCost(NodeList &frontier, const NodeCostMap &costSoFar) const;

	NodeList rebuildPath(const Resources::FloorEdge *start, const Resources::FloorEdge *goal,
	                                    const NodePrecedenceMap &cameFrom) const;
};

} // End of namespace Stark

#endif // STARK_MOVEMENT_SHORTEST_PATH_H
