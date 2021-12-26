/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PATROL_H
#define SAGA2_PATROL_H

namespace Saga2 {

struct TilePoint;
class  PatrolRouteList;
extern PatrolRouteList **patrolRouteList;   // Global patrol route array

/* ===================================================================== *
   PatrolRoute class
 * ===================================================================== */

//	This class represents an individual patrol route.  A patrol route
//	consists of an integer representing the number of way points in the
//	patrol route followed by a variable number of TilePoints representing
//	the coordinates of the patrol routes.

class PatrolRoute {
	int16 _wayPoints;	// The number of way points in this patrol route
	TilePoint **_route;

public:
	PatrolRoute(Common::SeekableReadStream *stream);
	~PatrolRoute();

	// Return the number of way points
	int16 vertices() const {
		return _wayPoints;
	}

	// Returns a const reference to a specified way point
	const TilePoint &operator[](int16 index) const;
};

/* ===================================================================== *
   PatrolRouteList class
 * ===================================================================== */

//	This class represent a variable sized list of patrol routes.  The list
//	consists of a pointer to the actual patrol route data followed by a
//	pointer to a dynamically allocated array of 32-bit integers representing
//	the offset, in bytes, of each corresponding patrol route from the
//	beginning of the patrol route data.

class PatrolRouteList {
	friend void initPatrolRoutes();
	friend void cleanupPatrolRoutes();

	int16 _numRoutes;
	PatrolRoute **_routes;

public:
	PatrolRouteList(Common::SeekableReadStream *stream);
	~PatrolRouteList();

	// Returns the number of patrol routes in the list
	int16 routes() {
		return _numRoutes;
	}

	// Returns a reference to the specified patrol route
	PatrolRoute &getRoute(int16 index) {
		return *_routes[index];
	}
};

/* ===================================================================== *
   PatrolRouteIterator class
 * ===================================================================== */

//	This class represents a view of a patrol route by introducing the notion
//	of iteration to the patrol routes.  In other words, it provides a
//	method of moving from one way point in a patrol route to the next.

enum PatrolRouteIteratorFlags {

	// These flags define the type of iterator, and are only initialized
	// when the iterator is constructed.
	patrolRouteReverse      = (1 << 0), // Iterate in reverse
	patrolRouteAlternate    = (1 << 1), // Iterate back and forth
	patrolRouteRepeat       = (1 << 2), // Iterate repeatedly
	patrolRouteRandom       = (1 << 3), // Iterate randomly

	// These flags define the current state of the iterator.
	patrolRouteInAlternate  = (1 << 4) // Iterator is currently going in
	// the alternate direction
};

class PatrolRouteIterator {
	int16 _routeNo;		// Index of the patrol route
	int16 _vertexNo;	// Current waypoint index
	byte _mapNum;		// Map in which this route exists
	byte _flags;		// various flags

public:
	PatrolRouteIterator() { _routeNo = _vertexNo = 0; _mapNum = _flags = 0;}
	PatrolRouteIterator(uint8 map, int16 rte, uint8 type);
	PatrolRouteIterator(uint8 map, int16 rte, uint8 type, int16 startingPoint);

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out) const;

private:
	void increment();		// Increment waypoint index
	void decrement();		// Decrement waypoint index
	void altIncrement();	// Increment in alternate direction
	void altDecrement();	// Decrement in alternate direction

public:
	// Determine if the iterator will repeat infinitely
	bool isRepeating() const {
		return _flags & (patrolRouteRepeat | patrolRouteRandom);
	}

	// Return the current way point number
	int16 wayPointNum() const {
		return _vertexNo;
	}

	// Return the coordinates of the current waypoint
	const TilePoint operator*() const;

	// Iterate
	const PatrolRouteIterator &operator++();

	// Determine if this iterator is equivalent to the specified iterator
	bool operator==(const PatrolRouteIterator &iter) const {
		return _routeNo == iter._routeNo && _vertexNo == iter._vertexNo
				&& _mapNum == iter._mapNum && _flags == iter._flags;
	}
};

/* ===================================================================== *
   PatrolRoute list management function prototypes
 * ===================================================================== */

// Load the patrol routes from the resource file
void initPatrolRoutes();

// Cleanup the patrol routes
void cleanupPatrolRoutes();

} // end of namespace Saga2

#endif
