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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/patrol.h"
#include "saga2/cmisc.h"
#include "saga2/tcoords.h"
#include "saga2/hresmgr.h"

namespace Saga2 {

extern int16 worldCount;						// Number of worlds

PatrolRouteList **patrolRouteList = nullptr;		// Global patrol route array

PatrolRoute::PatrolRoute(Common::SeekableReadStream *stream) {
	_wayPoints = stream->readSint16LE();

	_route = (TilePoint **)malloc(sizeof(TilePoint *) * _wayPoints);

	for (int i = 0; i < _wayPoints; i++)
		_route[i] = new TilePoint(stream);
}

PatrolRoute::~PatrolRoute() {
	for (int i = 0; i < _wayPoints; i++)
		delete _route[i];

	free(_route);
}

// Returns a const reference to a specified way point
const TilePoint &PatrolRoute::operator [](int16 index) const {
	return *_route[index];
}


/* ===================================================================== *
   PatrolRouteList member functions
 * ===================================================================== */
PatrolRouteList::PatrolRouteList(Common::SeekableReadStream *stream) {
	_numRoutes = stream->readSint16LE();

	_routes = (PatrolRoute **)malloc(sizeof(PatrolRoute *) * _numRoutes);

	for (int i = 0; i < _numRoutes; i++)
		_routes[i] = new PatrolRoute(stream);
}

PatrolRouteList::~PatrolRouteList() {
	for (int i = 0; i < _numRoutes; i++)
		delete _routes[i];

	free(_routes);
}

/* ===================================================================== *
   PatrolRouteIterator member functions
 * ===================================================================== */

PatrolRouteIterator::PatrolRouteIterator(uint8 map, int16 rte, uint8 type) :
		_mapNum(map), _routeNo(rte), _flags(type & 0xF) {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	if (_flags & patrolRouteRandom)
		_vertexNo = g_vm->_rnd->getRandomNumber(route.vertices() - 1);
	else {
		if (_flags & patrolRouteReverse)
			_vertexNo = route.vertices() - 1;
		else
			_vertexNo = 0;
	}
}

PatrolRouteIterator::PatrolRouteIterator(uint8 map, int16 rte, uint8 type, int16 startingPoint) :
		_mapNum(map), _routeNo(rte), _flags(type & 0xF) {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	_vertexNo = clamp(0, startingPoint, route.vertices() - 1);
}

void PatrolRouteIterator::read(Common::InSaveFile *in) {
	_routeNo = in->readSint16LE();
	_vertexNo = in->readSint16LE();
	_mapNum = in->readByte();
	_flags = in->readByte();
}

void PatrolRouteIterator::write(Common::MemoryWriteStreamDynamic *out) const {
	out->writeSint16LE(_routeNo);
	out->writeSint16LE(_vertexNo);
	out->writeByte(_mapNum);
	out->writeByte(_flags);
}

//-----------------------------------------------------------------------
//	Increment the waypoint index
void PatrolRouteIterator::increment() {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	_vertexNo++;

	if (_vertexNo >= route.vertices()) {
		if (_flags & patrolRouteAlternate) {
			// If alternating, initialize for iteration in the alternate
			// direction
			_flags |= patrolRouteInAlternate;
			_vertexNo = MAX(route.vertices() - 2, 0);
		} else if (_flags & patrolRouteRepeat)
			// If repeating, reset the waypoint index
			_vertexNo = 0;
	}
}

//-----------------------------------------------------------------------
//	Decrement the waypoint index
void PatrolRouteIterator::decrement() {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	_vertexNo--;

	if (_vertexNo < 0) {
		if (_flags & patrolRouteAlternate) {
			// If alternating, initialize for iteration in the alternate
			// direction
			_flags |= patrolRouteInAlternate;
			_vertexNo = MIN(1, route.vertices() - 1);
		} else if (_flags & patrolRouteRepeat)
			// If repeating, reset the waypoint index
			_vertexNo = route.vertices() - 1;
	}
}

//-----------------------------------------------------------------------
//	Increment the waypoint index in the alternate direction
void PatrolRouteIterator::altIncrement() {
	const PatrolRoute   &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	_vertexNo++;

	if (_vertexNo >= route.vertices() && (_flags & patrolRouteRepeat)) {
		// If repeating, initialize for iteration in the standard
		// direction, and reset the waypoint index
		_flags &= ~patrolRouteInAlternate;
		_vertexNo = MAX(route.vertices() - 2, 0);
	}
}

//-----------------------------------------------------------------------
//	Decrement the waypoint index in the alternate direction
void PatrolRouteIterator::altDecrement() {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	_vertexNo--;

	if (_vertexNo < 0 && (_flags & patrolRouteRepeat)) {
		// If repeating, initialize for iteration in the standard
		// direction, and reset the waypoint index
		_flags &= ~patrolRouteInAlternate;
		_vertexNo = MIN(1, route.vertices() - 1);
	}
}

//-----------------------------------------------------------------------
//	Return the coordinates of the current waypoint
const TilePoint PatrolRouteIterator::operator*() const {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	return _vertexNo >= 0 && _vertexNo < route.vertices() ? route[_vertexNo] : Nowhere;
}

const PatrolRouteIterator &PatrolRouteIterator::operator++() {
	const PatrolRoute &route = patrolRouteList[_mapNum]->getRoute(_routeNo);

	if (_vertexNo >= 0 && _vertexNo < route.vertices()) {
		if (!(_flags & patrolRouteRandom)) {
			if (!(_flags & patrolRouteInAlternate)) {
				if (!(_flags & patrolRouteReverse))
					increment();
				else
					decrement();
			} else {
				if (!(_flags & patrolRouteReverse))
					altDecrement();
				else
					altIncrement();
			}
		} else {
			_vertexNo = g_vm->_rnd->getRandomNumber(route.vertices() - 1);
		}
	}

	return *this;
}

//-----------------------------------------------------------------------
//	Load the patrol routes from the resource file
void initPatrolRoutes() {
	// Get patrol route resource context
	hResContext *patrolRouteRes = auxResFile->newContext(MKTAG('P', 'T', 'R', 'L'), "patrol route resource");
	if (patrolRouteRes == nullptr || !patrolRouteRes->_valid)
		error("Error accessing patrol route resource group.");

	debugC(1, kDebugLoading, "Loading Patrol Routes for %d worlds", worldCount);

	// Allocate the patrol route list array
	patrolRouteList = (PatrolRouteList **)malloc(sizeof(PatrolRouteList *) * worldCount);

	if (patrolRouteList == nullptr)
		error("Unable to allocate the patrol route list");

	int count = 0;
	for (int16 i = 0; i < worldCount; i++) {
		patrolRouteList[i] = nullptr;

		// Load this worlds's patrol routes
		if (patrolRouteRes->size(MKTAG('R', 'T', 'E', i)) > 0) {
			Common::SeekableReadStream *stream = loadResourceToStream(patrolRouteRes, MKTAG('R', 'T', 'E', i), "patrol route data");

			patrolRouteList[i] = new PatrolRouteList(stream);
			count++;

			delete stream;
		}
	}

	debugC(1, kDebugLoading, "Loading Patrol Routes, loaded %d entries", count);

	// Dispose of the patrol route resource context
	auxResFile->disposeContext(patrolRouteRes);
}

void cleanupPatrolRoutes() {
	if (!patrolRouteList)
		return;

	for (int16 i = 0; i < worldCount; i++)
		delete patrolRouteList[i];

	free(patrolRouteList);

	patrolRouteList = nullptr;
}

} // end of namespace Saga2
