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

#ifndef PINK_WALK_SHORTEST_PATH_H
#define PINK_WALK_SHORTEST_PATH_H

#include "common/array.h"

namespace Pink {

class WalkLocation;
class WalkMgr;

class WalkShortestPath {
public:
	WalkShortestPath(WalkMgr *manager);
	WalkLocation *next(WalkLocation *start, WalkLocation *destination);

private:
	void add(WalkLocation *wl, double val, WalkLocation *nearest);
	void remove(WalkLocation *location);
	WalkLocation *build();
	WalkLocation *getNearestNeighbor(WalkLocation *location);
	WalkLocation *findNearestNeighbor(WalkLocation *location);
	double getLengthToNearestNeigbor(WalkLocation *location);
	double getWeight(WalkLocation *location);
	void addLocationsToVisit();
	bool isLocationVisited(WalkLocation *location);


	WalkMgr *_manager;
	Common::Array<WalkLocation *> _locations;
	Common::Array<WalkLocation *> _toVisit;
	Common::Array<double> _weight;
	Common::Array<WalkLocation *> _visited;
	Common::Array<WalkLocation *> _nearestNeigbor;
};

} // End of namespace Pink

#endif
