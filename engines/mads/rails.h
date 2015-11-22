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

#ifndef MADS_RAILS_H
#define MADS_RAILS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stack.h"
#include "mads/msurface.h"

namespace MADS {

class WalkNode {
public:
	Common::Point _walkPos;
	uint16 _distances[MAX_ROUTE_NODES];
	bool _active;

	/**
	 * Constructor
	 */
	WalkNode();

	/**
	 * Loads the scene node
	 */
	void load(Common::SeekableReadStream *f);
};
typedef Common::Array<WalkNode> WalkNodeList;

/**
 * This class handles storing the intermediate walk node points for a
 * given scene, and calculating walking routes between any two positions.
 */
class Rails {
private:
	WalkNodeList _nodes;
	DepthSurface *_depthSurface;
	int _depthStyle;
	int _routeLength;
	int _next;
	int _tempRoute[MAX_ROUTE_NODES];
	Common::Stack<int> _routeIndexes;
private:
	/**
	* Change the position of a walking node. Doing so causes a recalculation of the
	* distance between it and every other node, and vice versa
	*/
	void setNodePosition(int nodeIndex, const Common::Point &pt);

	int getRouteFlags(const Common::Point &src, const Common::Point &dest);
public:
	/**
	 * Constructor
	 */
	Rails();

	/**
	 * Loads the scene data for the list of intermediate walk nodes and the
	 * depth surface to use.
	 * @param nodes			Intermediate walk-points
	 * @param depthSurface	Depth surface to use
	 */
	void load(const WalkNodeList &nodes, DepthSurface *depthSurface, int depthStyle);

	/**
	 * Set up a route between two points in a scene
	 */
	void setupRoute(bool bitFlag, const Common::Point &srcPos, const Common::Point &destPos);

	void setupRouteNode(int *routeIndexP, int nodeIndex, int flags, int routeLength);

	/**
	 * Resets any currently running route
	 */
	void resetRoute();

	/**
	* Scans along an edge connecting two points within the depths/walk surface, and returns the information of the first
	* pixel high nibble encountered with a non-zero value
	*/
	int scanPath(const Common::Point &srcPos, const Common::Point &destPos);

	/*
	 * Return the number of walk nodes in the calculated route
	 */
	int size() const { return _routeIndexes.size(); }

	/**
	 * Returns true if the current calculated walk route is empty
	 */
	bool empty() const { return _routeIndexes.empty(); }

	/**
	 * Returns the data for a given walk node
	 */
	const WalkNode &operator[](int idx) { return _nodes[_routeIndexes[idx]]; }

	const WalkNode &popNode();

	void resetNext() { _next = 0; }
	int  getNext() { return _next; }

	/**
	 * Synchronize the data for the route
	 */
	void synchronize(Common::Serializer &s);

	void disableNode(int idx);
	void disableLine(int from, int to);

};

} // End of namespace MADS

#endif /* MADS_RAILS_H */
