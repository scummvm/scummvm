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

#include "pink/objects/walk/walk_location.h"
#include "pink/objects/walk/walk_mgr.h"

namespace Pink {

WalkShortestPath::WalkShortestPath(WalkMgr *manager)
	: _manager(manager)
{}

WalkLocation *WalkShortestPath::next(WalkLocation *start, WalkLocation *destination) {
	if (start == destination)
		return nullptr;
	add(start, 0.0, 0);
	while (build() != destination) {}
	return getNearestNeighbor(destination);
}

void WalkShortestPath::add(WalkLocation *wl, double val, WalkLocation *nearest) {
	_locations.push_back(wl);
	_visited.push_back(wl);
	_weight.push_back(val);
	_nearestNeigbor.push_back(nearest);
}

WalkLocation *WalkShortestPath::build() {
	WalkLocation *nearest = nullptr;
	WalkLocation *location = nullptr;
	double len = -1.0;
	addLocationsToVisit();
	for (uint i = 0; i < _toVisit.size(); ++i) {
		double curLen = getLengthToNearestNeigbor(_toVisit[i]);
		if (curLen < 0) {
			remove(_toVisit[i]);
			continue;
		}
		curLen += getWeight(_toVisit[i]);
		if (len < 0.0 || len > curLen) {
			len = curLen;
			location = _toVisit[i];
			nearest = getNearestNeighbor(_toVisit[i]);
			if (!nearest)
				nearest = findNearestNeighbor(_toVisit[i]);
		}
	}

	WalkLocation *neighbor = findNearestNeighbor(location);
	if (neighbor)
		add(neighbor, len, nearest);

	return neighbor;
}

WalkLocation *WalkShortestPath::getNearestNeighbor(WalkLocation *location) {
	for(uint i = 0; i < _visited.size(); ++i) {
		if (_visited[i] == location)
			return _nearestNeigbor[i];
	}

	return nullptr;
}

void WalkShortestPath::addLocationsToVisit() {
	_toVisit.resize(_locations.size());
	for (uint i = 0; i < _locations.size(); ++i) {
		_toVisit[i] = _locations[i];
	}
}

double WalkShortestPath::getLengthToNearestNeigbor(WalkLocation *location) {
	double minLength = -1.0;
	Common::StringArray &neighbors = location->getNeigbors();
	for (uint i = 0; i < neighbors.size(); ++i) {
		WalkLocation *neighbor = _manager->findLocation(neighbors[i]);
		if (!isLocationVisited(neighbor)) {
			double length = _manager->getLengthBetweenLocations(location, neighbor);
			if (minLength >= 0.0) {
				if (length < minLength)
					minLength = length;
			} else
				minLength = length;
		}
	}

	return minLength;
}

WalkLocation *WalkShortestPath::findNearestNeighbor(WalkLocation *location) {
	double minLength = -1.0;
	WalkLocation *nearest = nullptr;
	Common::StringArray &neighbors = location->getNeigbors();
	for (uint i = 0; i < neighbors.size(); ++i) {
		WalkLocation *neighbor = _manager->findLocation(neighbors[i]);
		if (!isLocationVisited(neighbor)) {
			double length = _manager->getLengthBetweenLocations(location, neighbor);
			if (minLength >= 0.0) {
				if (length < minLength) {
					nearest = neighbor;
					minLength = length;
				}
			} else {
				nearest = neighbor;
				minLength = length;
			}
		}
	}

	return nearest;
}

double WalkShortestPath::getWeight(WalkLocation *location) {
	for (uint i = 0; i < _locations.size(); ++i) {
		if (_locations[i] == location)
			return _weight[i];
	}
	return 0.0;
}

bool WalkShortestPath::isLocationVisited(WalkLocation *location) {
	for (uint i = 0; i < _visited.size(); ++i) {
		if (_visited[i] == location)
			return true;
	}
	return false;
}

void WalkShortestPath::remove(WalkLocation *location) {
	for (uint i = 0; i < _locations.size(); ++i) {
		if (_locations[i] == location) {
			_locations.remove_at(i);
			_weight.remove_at(i);
			break;
		}
	}
}

} // End of namespace Pink
