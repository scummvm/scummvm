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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/fta.h"
#include "saga2/patrol.h"

namespace Saga2 {

extern int16    worldCount;             //  Number of worlds

/* ===================================================================== *
   Exports
 * ===================================================================== */

PatrolRouteList         *patrolRouteList = NULL;    //  Global patrol route array

/* ===================================================================== *
   Globals
 * ===================================================================== */

static PatrolRouteData  **patrolRouteData;      //  Data for patrol routes

/* ===================================================================== *
   PatrolRouteList member functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Clear the current patrol route data

void PatrolRouteList::clearRouteData(void) {
	//  If there is an offset array deallocated it.
	if (offsetArray) {
		RDisposePtr(offsetArray);
		offsetArray = NULL;
	}

	routeData = NULL;
}

//-----------------------------------------------------------------------
//	Set up and initialize new patrol route data

void PatrolRouteList::setRouteData(PatrolRouteData *data) {
	int16       i,
	            noRoutes = data->routes;
	PatrolRoute *currentRoute;

	//  If routeData is NULL simply return
	if ((routeData = data) == NULL) {
		offsetArray = NULL;
		return;
	}

	//  Allocate a new offsetArray
	offsetArray = (int32 *)RNewPtr(
	                  noRoutes * sizeof(int32),
	                  NULL,
	                  "patrol route offset array");

	if (offsetArray == NULL)
		error("Cannot allocate patrol route list offset array.");

	//  Iterate through each patrol route a compute its offset
	for (i = 0, currentRoute = (PatrolRoute *)&routeData[ 1 ];
	        i < noRoutes;
	        i++,
	        currentRoute =
	            (PatrolRoute *) & (*currentRoute)[ currentRoute->vertices() ]) {
#if DEBUG
		VERIFY(currentRoute->vertices() > 1);
#endif
		warning("STUB: PatrolRouteList::setRouteData: unsafe arithmetics");
		offsetArray[ i ] = 0; // FIXME: It was "currentRoute - routeData";
	}
}

/* ===================================================================== *
   PatrolRouteIterator member functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Constructors

PatrolRouteIterator::PatrolRouteIterator(uint8 map, int16 rte, uint8 type) :
	mapNum(map),
	routeNo(rte),
	flags(type & 0xF) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	if (flags & patrolRouteRandom)
		vertexNo = rand() % route.vertices();
	else {
		if (flags & patrolRouteReverse)
			vertexNo = route.vertices() - 1;
		else
			vertexNo = 0;
	}
}

PatrolRouteIterator::PatrolRouteIterator(
    uint8 map,
    int16 rte,
    uint8 type,
    int16 startingPoint) :
	mapNum(map),
	routeNo(rte),
	flags(type & 0xF) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	vertexNo = clamp(0, startingPoint, route.vertices() - 1);
}

//-----------------------------------------------------------------------
//	Increment the waypoint index

void PatrolRouteIterator::increment(void) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	vertexNo++;

	if (vertexNo >= route.vertices()) {
		if (flags & patrolRouteAlternate) {
			//  If alternating, initialize for iteration in the alternate
			//  direction
			flags |= patrolRouteInAlternate;
			vertexNo = MAX(route.vertices() - 2, 0);
		} else if (flags & patrolRouteRepeat)
			//  If repeating, reset the waypoint index
			vertexNo = 0;
	}
}

//-----------------------------------------------------------------------
//	Decrement the waypoint index

void PatrolRouteIterator::decrement(void) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	vertexNo--;

	if (vertexNo < 0) {
		if (flags & patrolRouteAlternate) {
			//  If alternating, initialize for iteration in the alternate
			//  direction
			flags |= patrolRouteInAlternate;
			vertexNo = MIN(1, route.vertices() - 1);
		} else if (flags & patrolRouteRepeat)
			//  If repeating, reset the waypoint index
			vertexNo = route.vertices() - 1;
	}
}

//-----------------------------------------------------------------------
//	Increment the waypoint index in the alternate direction

void PatrolRouteIterator::altIncrement(void) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	vertexNo++;

	if (vertexNo >= route.vertices()
	        && (flags & patrolRouteRepeat)) {
		//  If repeating, initialize for iteration in the standard
		//  direction, and reset the waypoint index
		flags &= ~patrolRouteInAlternate;
		vertexNo = MAX(route.vertices() - 2, 0);
	}
}

//-----------------------------------------------------------------------
//	Decrement the waypoint index in the alternate direction

void PatrolRouteIterator::altDecrement(void) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	vertexNo--;

	if (vertexNo < 0
	        && (flags & patrolRouteRepeat)) {
		//  If repeating, initialize for iteration in the standard
		//  direction, and reset the waypoint index
		flags &= ~patrolRouteInAlternate;
		vertexNo = MIN(1, route.vertices() - 1);
	}
}

//-----------------------------------------------------------------------
//	Return the coordinates of the current waypoint

const TilePoint &PatrolRouteIterator::operator * (void) const {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	return  vertexNo >= 0 && vertexNo < route.vertices()
	        ?   route[ vertexNo ]
	        :   Nowhere;
}

//-----------------------------------------------------------------------
//	Iterate

const PatrolRouteIterator &PatrolRouteIterator::operator ++ (void) {
	const PatrolRoute   &route = patrolRouteList[ mapNum ][ routeNo ];

	if (vertexNo >= 0 & vertexNo < route.vertices()) {
		if (!(flags & patrolRouteRandom)) {
			if (!(flags & patrolRouteInAlternate)) {
				if (!(flags & patrolRouteReverse))
					increment();
				else
					decrement();
			} else {
				if (!(flags & patrolRouteReverse))
					altDecrement();
				else
					altIncrement();
			}
		} else {
			vertexNo = rand() % route.vertices();
		}
	}

	return *this;
}

/* ===================================================================== *
   PatrolRoute list management functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Load the patrol routes from the resource file

void initPatrolRoutes(void) {
	int16           i;
	hResContext     *patrolRouteRes;

	//  Get patrol route resource context
	patrolRouteRes =    auxResFile->newContext(
	                        RES_ID('P', 'T', 'R', 'L'),
	                        "patrol route resource");
	if (patrolRouteRes == NULL || !patrolRouteRes->_valid)
		error("Error accessing patrol route resource group.");

	//  Allocate the patrol route list array
	patrolRouteList =
	    (PatrolRouteList *)RNewPtr(
	        sizeof(PatrolRouteList) * worldCount,
	        NULL,
	        "patrol route list");

	if (patrolRouteList == NULL)
		error("Unable to allocate the patrol route list");

	//  Allocate the patrol route data pointer array
	patrolRouteData =
	    (PatrolRouteData **)RNewPtr(
	        sizeof(PatrolRouteData *) * worldCount,
	        NULL,
	        "patrol route data pointers");

	if (patrolRouteData == NULL)
		error("Unable to allocate the patrol route data pointers");

	for (i = 0; i < worldCount; i++) {
		//  Initialize the patrol route data members
		patrolRouteList[ i ].routeData = NULL;
		patrolRouteList[ i ].offsetArray = NULL;

		//  Load this worlds's patrol routes
		if (patrolRouteRes->size(
		            RES_ID('R', 'T', 'E', i)) > 0) {
			patrolRouteData[ i ] =
			    (PatrolRouteData *)LoadResource(
			        patrolRouteRes,
			        RES_ID('R', 'T', 'E', i),
			        "patrol route data");

			if (patrolRouteData[ i ] == NULL)
				error("Unable to load the patrol route data");

			//  Initialize the PatrolRouteList with the resource data
			patrolRouteList[ i ].setRouteData(patrolRouteData[ i ]);
		} else
			patrolRouteData[ i ] = NULL;
	}

	//  Dispose of the patrol route resource context
	auxResFile->disposeContext(patrolRouteRes);
}

//-----------------------------------------------------------------------
//	Cleanup the patrol routes

void cleanupPatrolRoutes(void) {
	int16   i;

	//  Cleanup the patrol route list
	for (i = 0; i < worldCount; i++) {
		patrolRouteList[ i ].clearRouteData();
		RDisposePtr(patrolRouteData[ i ]);
	}

	//  Deallocate the patrol route data pointer list
	RDisposePtr(patrolRouteData);

	//  Deallocate the patrol route list
	RDisposePtr(patrolRouteList);
}

} // end of namespace Saga2
