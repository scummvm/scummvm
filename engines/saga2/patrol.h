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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_PATROL_H
#define SAGA2_PATROL_H

#include "saga2/cmisc.h"
#include "saga2/tcoords.h"

namespace Saga2 {

class  PatrolRouteList;
extern PatrolRouteList  *patrolRouteList;   //  Global patrol route array

/* ===================================================================== *
   PatrolRoute class
 * ===================================================================== */

//	This class represents an individual patrol route.  A patrol route
//	consists of an integer representing the number of way points in the
//	patrol route followed by a variable number of TilePoints representing
//	the coordinates of the patrol routes.

class PatrolRoute {
	int16       wayPoints;  //  The number of way points in this patrol route

public:
	//  Return the number of way points
	int16 vertices(void) const {
		return wayPoints;
	}

	//  Returns a const reference to a specified way point
	const TilePoint &operator [](int16 index) const {
		return *((TilePoint *)&this[ 1 ] + index);
	}
};

/* ===================================================================== *
   PatrolRouteData struct
 * ===================================================================== */

//	This class represents the patrol route data as it is read in from the
//	resource file.  The patrol route data consists of an integer representing
//	the number of patrol routes, followed by the actual patrol route data.

struct PatrolRouteData {
	int16       routes;     //  The number of routes in this data
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

	friend void initPatrolRoutes(void);
	friend void cleanupPatrolRoutes(void);

	PatrolRouteData     *routeData;     //  A pointer to the raw patrol route
	//  data.
	int32               *offsetArray;   //  A pointer to a dynaically
	//  allocated array of 32-bit offsets.

public:
	//  Remove current patrol route data
	void clearRouteData(void);
	//  Set up an initialize new patrol route data
	void setRouteData(PatrolRouteData *data);

	//  Returns the number of patrol routes in the list
	int16 routes(void) {
		return routeData ? routeData->routes : 0;
	}

	//  Returns a reference to the specified patrol route
	PatrolRoute &operator [](int16 index) {
		// FIXME: This is evil
		warning("PartolRoute[]: dangerous pointer arithmetic, this will not work");
		return *((PatrolRoute *)(routeData
		                         +   offsetArray[ index ]));
	}
};

/* ===================================================================== *
   PatrolRouteIterator class
 * ===================================================================== */

//	This class represents a view of a patrol route by introducing the notion
//	of iteration to the patrol routes.  In other words, it provides a
//	method of moving from one way point in a patrol route to the next.

enum PatrolRouteIteratorFlags {

	//  These flags define the type of iterator, and are only initialized
	//  when the iterator is constructed.
	patrolRouteReverse      = (1 << 0), //  Iterate in reverse
	patrolRouteAlternate    = (1 << 1), //  Iterate back and forth
	patrolRouteRepeat       = (1 << 2), //  Iterate repeatedly
	patrolRouteRandom       = (1 << 3), //  Iterate randomly

	//  These flags define the current state of the iterator.
	patrolRouteInAlternate  = (1 << 4), //  Iterator is currently going in
	//  the alternate direction
};

class PatrolRouteIterator {
	int16           routeNo,            //  Index of the patrol route
	                vertexNo;           //  Current waypoint index

	uint8           mapNum;             //  Map in which this route exists

	uint8           flags;              //  various flags

	// 6 bytes

public:
	//  Default constructor -- do nothing
	PatrolRouteIterator(void) {}

	//  Copy constructor
	PatrolRouteIterator(const PatrolRouteIterator &iter) {
		//  Copy first four bytes
		((int32 *)this)[ 0 ] = ((int32 *)&iter)[ 0 ];
		//  Copy next two bytes
		*((int16 *) & ((int32 *)this)[ 1 ]) =
		    *((int16 *) & ((int32 *)&iter)[ 1 ]);
	}

	//  Constructors
	PatrolRouteIterator(uint8 map, int16 rte, uint8 type);
	PatrolRouteIterator(
	    uint8 map,
	    int16 rte,
	    uint8 type,
	    int16 startingPoint);

	//  Overloaded assignment operator
	PatrolRouteIterator operator = (const PatrolRouteIterator &iter) {
		//  Copy first four bytes
		((int32 *)this)[ 0 ] = ((int32 *)&iter)[ 0 ];
		//  Copy next two bytes
		*((int16 *) & ((int32 *)this)[ 1 ]) =
		    *((int16 *) & ((int32 *)&iter)[ 1 ]);

		return *this;
	}

private:
	void increment(void);        //  Increment waypoint index
	void decrement(void);        //  Decrement waypoint index
	void altIncrement(void);     //  Increment in alternate direction
	void altDecrement(void);     //  Decrement in alternate direction

public:
	//  Determine if the iterator will repeat infinitely
	bool isRepeating(void) const {
		return flags & (patrolRouteRepeat | patrolRouteRandom);
	}

	//  Return the current way point number
	int16 wayPointNum(void) const {
		return vertexNo;
	}

	//  Return the coordinates of the current waypoint
	const TilePoint &operator * (void) const;

	//  Iterate
	const PatrolRouteIterator &operator ++ (void);

	//  Determine if this iterator is equivalent to the specified iterator
	bool operator == (const PatrolRouteIterator &iter) const {
		return      routeNo == iter.routeNo && vertexNo == iter.vertexNo
		            &&  mapNum == iter.mapNum && flags == iter.flags;
	}
};

/* ===================================================================== *
   PatrolRoute list management function prototypes
 * ===================================================================== */

//  Load the patrol routes from the resource file
void initPatrolRoutes(void);

//  Cleanup the patrol routes
void cleanupPatrolRoutes(void);

} // end of namespace Saga2

#endif
