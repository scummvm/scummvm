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

#include "engines/stark/movement/shortestpath.h"

#include "common/hash-ptr.h"

#include "engines/stark/resources/floor.h"

namespace Stark {

ShortestPath::NodeList ShortestPath::search(const Resources::FloorEdge *start, const Resources::FloorEdge *goal) {
	NodeList frontier;
	NodePrecedenceMap cameFrom;
	NodeCostMap costSoFar;

	frontier.push_back(start);
	cameFrom[start] = nullptr;
	costSoFar[start] = 0;

	while (!frontier.empty()) {
		const Resources::FloorEdge *current = popEdgeWithLowestCost(frontier, costSoFar);

		if (current == goal)
			break;

		Common::Array<Resources::FloorEdge *> neighbours = current->getNeighbours();
		for (uint i = 0; i < neighbours.size(); i++) {
			const Resources::FloorEdge *next = neighbours[i];
			if (!next->isEnabled())
				continue;

			float newCost = costSoFar[current] + current->costTo(next);
			if (!costSoFar.contains(next) || newCost < costSoFar[next]) {
				frontier.push_back(next);
				cameFrom[next] = current;
				costSoFar[next] = newCost;
			}
		}
	}

	return rebuildPath(start, goal, cameFrom);
}

ShortestPath::NodeList ShortestPath::rebuildPath(const Resources::FloorEdge *start, const Resources::FloorEdge *goal,
                                                 const NodePrecedenceMap &cameFrom) const {
	NodeList path;

	const Resources::FloorEdge *current = goal;
	path.push_front(goal);

	while (current && current != start) {
		current = cameFrom[current];
		path.push_front(current);
	}

	if (current != start) {
		// No path has been found from start to goal
		return NodeList();
	}

	path.push_front(start);
	return path;
}

const Resources::FloorEdge *ShortestPath::popEdgeWithLowestCost(NodeList &frontier, const NodeCostMap &costSoFar) const {
	// Poor man's priority queue using a list ...
	NodeList::iterator lowestCostItem = frontier.begin();
	for (NodeList::iterator it = frontier.begin(); it != frontier.end(); it++) {
		if (costSoFar[*it] < costSoFar[*lowestCostItem]) {
			lowestCostItem = it;
		}
	}

	const Resources::FloorEdge *result = *lowestCostItem;

	frontier.erase(lowestCostItem);

	return result;
}

} // End of namespace Stark
