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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/rails.h"

namespace MADS {

WalkNode::WalkNode() {
	_active = false;
	Common::fill(&_distances[0], &_distances[MAX_ROUTE_NODES], 0);
}

void WalkNode::load(Common::SeekableReadStream *f) {
	_walkPos.x = f->readSint16LE();
	_walkPos.y = f->readSint16LE();
	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		_distances[i] = f->readUint16LE();
}

/*------------------------------------------------------------------------*/

Rails::Rails() {
	_depthSurface = nullptr;
	_routeLength = 0;
	_depthStyle = 0;
	_next = 0;
}

void Rails::load(const WalkNodeList &nodes, DepthSurface *depthSurface, int depthStyle) {
	// Store the depth surface and depth style to use
	_depthSurface = depthSurface;
	_depthStyle = depthStyle;

	// Load the passed node list
	_nodes.clear();

	for (uint i = 0; i < nodes.size(); ++i)
		_nodes.push_back(nodes[i]);

	// Add two more empty nodes for the start and end points of any walk sequence
	_nodes.push_back(WalkNode());
	_nodes.push_back(WalkNode());
}


void Rails::setupRoute(bool bitFlag, const Common::Point &srcPos, const Common::Point &destPos) {
	// Reset the nodes in as being inactive
	for (uint i = 0; i < _nodes.size(); ++i)
		_nodes[i]._active = false;

	// Set the two extra walk nodes to the start and destination positions
	setNodePosition(_nodes.size() - 2, srcPos);
	setNodePosition(_nodes.size() - 1, destPos);

	// Start constructing route node list
	_routeLength = 0x3FFF;
	_routeIndexes.clear();

	// Recursively form a route from the destination walk node back to the player's position
	setupRouteNode(&_tempRoute[0], _nodes.size() - 1, bitFlag ? 0xC000 : 0x8000, 0);

	_next = 0;
	if (_routeIndexes.size() > 0) {
		Common::Point currPos = srcPos;
		for (int routeCtr = size() - 1; (routeCtr >= 0) && !_next; --routeCtr) {
			int idx = _routeIndexes[routeCtr];
			const Common::Point &pt = _nodes[idx]._walkPos;

			_next = scanPath(currPos, pt);
			currPos = pt;
		}
	}
}

void Rails::setupRouteNode(int *routeIndexP, int nodeIndex, int flags, int routeLength) {
	WalkNode &currentNode = _nodes[nodeIndex];
	currentNode._active = true;

	*routeIndexP++ = nodeIndex;

	// Get the index of the ultimate source position (the player)
	int subIndex = _nodes.size() - 2;

	int distanceVal = _nodes[nodeIndex]._distances[subIndex];
	if (distanceVal & flags) {
		routeLength += distanceVal & 0x3FFF;
		if (routeLength < _routeLength) {
			// Found a new shorter route to destination, so set up the route with the found one
			_routeIndexes.clear();
			for (int i = 0; routeIndexP != &_tempRoute[i]; ++i)
				_routeIndexes.push(_tempRoute[i]);
			_routeLength = routeLength;
		}
	} else {
		for (int idx = _nodes.size() - 2; idx > 0; --idx) {
			int nodePos = idx - 1;
			if (!_nodes[nodePos]._active && ((currentNode._distances[nodePos] & flags) != 0))
				setupRouteNode(routeIndexP, nodePos, 0x8000, routeLength + (distanceVal & 0x3fff));
		}
	}

	currentNode._active = false;
}


int Rails::scanPath(const Common::Point &srcPos, const Common::Point &destPos) {
	// For compressed depth surfaces, always return 0
	if (_depthStyle == 2)
		return 0;

	int yDiff = destPos.y - srcPos.y;
	int yAmount = MADS_SCREEN_WIDTH;

	if (yDiff < 0) {
		yDiff = -yDiff;
		yAmount = -yAmount;
	}

	int xDiff = destPos.x - srcPos.x;
	int xDirection = 1;
	int xAmount = 0;
	if (xDiff < 0) {
		xDiff = -xDiff;
		xDirection = -xDirection;
		xAmount = MIN(yDiff, xDiff);
	}

	++xDiff;
	++yDiff;

	const byte *srcP = (const byte *)_depthSurface->getBasePtr(srcPos.x, srcPos.y);
	int index = xAmount;

	// Outer loop
	for (int xCtr = 0; xCtr < xDiff; ++xCtr, srcP += xDirection) {
		index += yDiff;
		int v = (*srcP & 0x7F) >> 4;
		if (v)
			return v;

		// Inner loop for handling vertical movement
		while (index >= xDiff) {
			index -= xDiff;

			v = (*srcP & 0x7F) >> 4;
			if (v)
				return v;

			srcP += yAmount;
		}
	}

	return 0;
}

void Rails::resetRoute() {
	_routeIndexes.clear();
	_next = 0;
}

const WalkNode &Rails::popNode() {
	assert(!_routeIndexes.empty());

	return _nodes[_routeIndexes.pop()];
}

void Rails::setNodePosition(int nodeIndex, const Common::Point &pt) {
	int flags, hypotenuse;

	_nodes[nodeIndex]._walkPos = pt;

	// Recalculate inter-node lengths
	for (uint idx = 0; idx < _nodes.size(); ++idx) {
		int entry;
		if (idx == (uint)nodeIndex) {
			entry = 0x3FFF;
		} else {
			// Process the node
			flags = getRouteFlags(pt, _nodes[idx]._walkPos);

			int xDiff = ABS(_nodes[idx]._walkPos.x - pt.x);
			int yDiff = ABS(_nodes[idx]._walkPos.y - pt.y);
			hypotenuse = (int)sqrt((double)(xDiff * xDiff + yDiff * yDiff));

			if (hypotenuse >= 0x3FFF)
				// Shouldn't ever be this large
				hypotenuse = 0x3FFF;

			entry = hypotenuse | flags;
		}

		_nodes[idx]._distances[nodeIndex] = entry;
		_nodes[nodeIndex]._distances[idx] = entry;
	}
}

int Rails::getRouteFlags(const Common::Point &src, const Common::Point &dest) {
	int result = 0x8000;
	bool flag = false;

	int xDiff = ABS(dest.x - src.x);
	int yDiff = ABS(dest.y - src.y);
	int xDirection = dest.x >= src.x ? 1 : -1;
	int yDirection = dest.y >= src.y ? _depthSurface->w : -_depthSurface->w;
	int minorDiff = 0;
	if (dest.x < src.x)
		minorDiff = MIN(xDiff, yDiff);
	++xDiff;
	++yDiff;

	byte *srcP = _depthSurface->getBasePtr(src.x, src.y);

	int totalCtr = minorDiff;
	for (int xCtr = 0; xCtr < xDiff; ++xCtr, srcP += xDirection) {
		totalCtr += yDiff;

		if ((*srcP & 0x80) == 0)
			flag = false;
		else if (!flag) {
			flag = true;
			result -= 0x4000;
			if (result == 0)
				break;
		}

		while (totalCtr >= xDiff) {
			totalCtr -= xDiff;

			if ((*srcP & 0x80) == 0)
				flag = false;
			else if (!flag) {
				flag = true;
				result -= 0x4000;
				if (result == 0)
					break;
			}

			srcP += yDirection;
		}
		if (result == 0)
			break;
	}

	return result;
}

void Rails::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_routeLength);
	s.syncAsSint16LE(_next);

	if (s.isLoading()) {
		_routeIndexes.clear();
	}
}

void Rails::disableNode(int nodeIndex) {
	_nodes[nodeIndex]._active = false;

	for (uint16 i = 0; i < _nodes.size(); i++) {
		if (i != nodeIndex)
			disableLine(i, nodeIndex);
	}
}

void Rails::disableLine(int from, int to) {
	_nodes[from]._distances[to] = 0x3FFF;
	_nodes[to]._distances[from] = 0x3FFF;
}

} // End of namespace MADS
