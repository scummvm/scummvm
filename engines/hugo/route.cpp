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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// Find shortest route from hero to destination

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/route.h"
#include "hugo/global.h"

namespace Hugo {
Route::Route(HugoEngine &vm) : _vm(vm) {
}

// Face hero in new direction, based on cursor key input by user.
void Route::setDirection(uint16 keyCode) {
	object_t *obj = _vm._hero;                      // Pointer to hero object

	debugC(1, kDebugRoute, "setDirection(%d)", keyCode);

	// Set first image in sequence
	switch (keyCode) {
	case Common::KEYCODE_UP:
		obj->currImagePtr = obj->seqList[_UP].seqPtr;
		break;
	case Common::KEYCODE_DOWN:
		obj->currImagePtr = obj->seqList[DOWN].seqPtr;
		break;
	case Common::KEYCODE_LEFT:
		obj->currImagePtr = obj->seqList[LEFT].seqPtr;
		break;
	case Common::KEYCODE_RIGHT:
		obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
		break;
	case Common::KEYCODE_HOME:
		obj->currImagePtr = obj->seqList[LEFT].seqPtr;
		break;
	case Common::KEYCODE_END:
		obj->currImagePtr = obj->seqList[LEFT].seqPtr;
		break;
//	case Common::KEYCODE_PRIOR:
//		obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
//		break;
//	case Common::KEYCODE_NEXT:
//		obj->currImagePtr = obj->seqList[RIGHT].seqPtr;
//		break;
	}
}

// Set hero walking, based on cursor key input by user.
// Hitting same key twice will stop hero.
void Route::setWalk(uint16 direction) {
	object_t *obj = _vm._hero;                      // Pointer to hero object
	static uint16 oldDirection = 0;                 // Last direction char

	debugC(1, kDebugRoute, "setWalk(%d)", direction);

	if (_vm.getGameStatus().storyModeFl || obj->pathType != USER)           // Make sure user has control
		return;

	if (!obj->vx && !obj->vy)
		oldDirection = 0;                           // Fix for consistant restarts

	if (direction != oldDirection) {
		// Direction has changed
		setDirection(direction);                    // Face new direction
		obj->vx = obj->vy = 0;
		switch (direction) {                        // And set correct velocity
		case Common::KEYCODE_UP:
			obj->vy = -DY;
			break;
		case Common::KEYCODE_DOWN:
			obj->vy =  DY;
			break;
		case Common::KEYCODE_LEFT:
			obj->vx = -DX;
			break;
		case Common::KEYCODE_RIGHT:
			obj->vx =  DX;
			break;
		case Common::KEYCODE_HOME:
			obj->vx = -DX;
			obj->vy = -DY / 2;
			break;
		case Common::KEYCODE_END:
			obj->vx = -DX;
			obj->vy =  DY / 2;
			break;
//		case Common::KEYCODE_PRIOR:
//			obj->vx =  DX;
//			obj->vy = -DY / 2;
//			break;
//		case Common::KEYCODE_NEXT:
//			obj->vx =  DX;
//			obj->vy =  DY / 2;
//			break;
		}
		oldDirection = direction;
		obj->cycling = CYCLE_FORWARD;
	} else {
		// Same key twice - halt hero
		obj->vy = 0;
		obj->vx = 0;
		oldDirection = 0;
		obj->cycling = NOT_CYCLING;
	}
}

// Recursive algorithm!  Searches from hero to dest_x, dest_y
// Find horizontal line segment about supplied point and recursively
// find line segments for each point above and below that segment.
// When destination point found in segment, start surfacing and leave
// a trail in segment[] from destination back to hero.
//
// Note:  there is a bug which allows a route through a 1-pixel high
// narrow gap if between 2 segments wide enough for hero.  To work
// around this, make sure any narrow gaps are 2 or more pixels high.
// An example of this was the blocking guard in Hugo1/Dead-End.
void Route::segment(int16 x, int16 y) {
	int16 x1, x2;                                   // Range of segment
// Note use of static - can't waste stack
	static image_pt   p;                            // Ptr to _boundaryMap[y]
	static segment_t *seg_p;                        // Ptr to segment

	debugC(1, kDebugRoute, "segment(%d, %d)", x, y);

	// Bomb out if stack exhausted
	// Vinterstum: Is this just a safeguard, or actually used?
	//_fullStackFl = _stackavail () < 256;
	_fullStackFl = false;

	// Find and fill on either side of point
	p = _boundaryMap[y];
	for (x1 = x; x1 > 0; x1--)
		if (p[x1] == 0) {
#if DEBUG_ROUTE
			SetPixel(hDC, (int16)((long)config.cx * x1  / XPIX), (int16)((long)config.cy *(y - DIBOFF_Y) / VIEW_DY), GetPalIndex(_TLIGHTMAGENTA));
#endif
			p[x1] = kMapFill;
		} else
			break;
	for (x2 = x + 1; x2 < XPIX; x2++)
		if (p[x2] == 0) {
#if DEBUG_ROUTE
			SetPixel(hDC, (int16)((long)config.cx * x2  / XPIX), (int16)((long)config.cy *(y - DIBOFF_Y) / VIEW_DY), GetPalIndex(_TLIGHTGREEN));
#endif
			p[x2] = kMapFill;
		} else
			break;
	x1++;
	x2--;

	// Discard path if not wide enough for hero - dead end
	if (_heroWidth > x2 - x1 + 1)
		return;

	// Have we found the destination yet?
	if (y == _destY && x1 <= _destX && x2 >= _destX)
		_routeFoundFl = true;

	// Bounds check y in case no boundary around screen
	if (y <= 0 || y >= YPIX - 1)
		return;
#if FALSE
	// Find all segments above and below current
	if (hero_p->x < x1 || hero_p->x + HERO_MAX_WIDTH > x2) {
		// Hero x not in segment, search x1..x2
		// Find all segments above current
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y - 1][x] == 0)
				segment(x, y - 1);

		// Find all segments below current
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y + 1][x] == 0)
				segment(x, y + 1);
	}
#endif
	if (_vm._hero->x < x1) {
		// Hero x not in segment, search x1..x2
		// Find all segments above current
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y - 1][x] == 0)
				segment(x, y - 1);

		// Find all segments below current
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y + 1][x] == 0)
				segment(x, y + 1);
	} else if (_vm._hero->x + HERO_MAX_WIDTH > x2) {
		// Hero x not in segment, search x1..x2
		// Find all segments above current
		for (x = x2; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x >= x1; x--)
			if (_boundaryMap[y - 1][x] == 0)
				segment(x, y - 1);

		// Find all segments below current
		for (x = x2; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x >= x1; x--)
			if (_boundaryMap[y + 1][x] == 0)
				segment(x, y + 1);
	} else {
		// Organize search around hero x position - this gives
		// better chance for more direct route.
		for (x = _vm._hero->x; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y - 1][x] == 0)
				segment(x, y - 1);
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x < _vm._hero->x; x++)
			if (_boundaryMap[y - 1][x] == 0)
				segment(x, y - 1);
		for (x = _vm._hero->x; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x <= x2; x++)
			if (_boundaryMap[y + 1][x] == 0)
				segment(x, y + 1);
		for (x = x1; !(_routeFoundFl | _fullStackFl | _fullSegmentFl) && x < _vm._hero->x; x++)
			if (_boundaryMap[y + 1][x] == 0)
				segment(x, y + 1);
	}

	// If found, surface, leaving trail back to hero
	if (_routeFoundFl) {
		// Bomb out if too many segments (leave one spare)
		if (_segmentNumb >= kMaxSeg - 1)
			_fullSegmentFl = true;
		else {
			// Create segment
			seg_p = &_segment[_segmentNumb];
			seg_p->y  = y;
			seg_p->x1 = x1;
			seg_p->x2 = x2;
			_segmentNumb++;
		}
	}
}

// Create and return ptr to new node.  Initialize with previous node.
// Returns NULL if MAX_NODES exceeded
Point *Route::newNode() {
	debugC(1, kDebugRoute, "newNode");

	if (_routeListIndex >= kMaxNodes)               // Too many nodes
		return(NULL);                               // Incomplete route - failure
	_routeListIndex++;
	_route[_routeListIndex] = _route[_routeListIndex - 1];  // Initialize with previous node
	return(&_route[_routeListIndex]);
}

// Construct route to cx, cy.  Return TRUE if successful.
// 1.  Copy boundary bitmap to local byte map (include object bases)
// 2.  Construct list of segments segment[] from hero to destination
// 3.  Compress to shortest route in route[]
bool Route::findRoute(int16 cx, int16 cy) {
	int16      i,   j,   x, y;                      // Loop on coordinates
	int16      x1,  x2,  dx;                        // Overlap between segments
	int16      herox1, herox2, heroy;               // Current hero baseline
	object_t  *obj;                                 // Ptr to object
	segment_t *seg_p;                               // Ptr to segment
	Point     *routeNode;                           // Ptr to route node

	debugC(1, kDebugRoute, "findRoute(%d, %d)", cx, cy);

	// Initialize for search
	_routeFoundFl  = false;                         // Path not found yet
	_fullStackFl = false;                           // Stack not exhausted
	_fullSegmentFl  = false;                        // Segments not exhausted
	_segmentNumb = 0;                               // Segment index
	_heroWidth = HERO_MIN_WIDTH;                    // Minimum width of hero
	_destY = cy;                                    // Destination coords
	_destX = cx;                                    // Destination coords
	herox1 = _vm._hero->x + _vm._hero->currImagePtr->x1;        // Hero baseline
	herox2 = _vm._hero->x + _vm._hero->currImagePtr->x2;        // Hero baseline
	heroy  = _vm._hero->y + _vm._hero->currImagePtr->y2;        // Hero baseline

	// Store all object baselines into objbound (except hero's = [0])
	for (i = 1, obj = &_vm._objects[i]; i < _vm._numObj; i++, obj++)
		if ((obj->screenIndex == *_vm._screen_p) && (obj->cycling != INVISIBLE) && (obj->priority == FLOATING))
			_vm.storeBoundary(obj->oldx + obj->currImagePtr->x1, obj->oldx + obj->currImagePtr->x2, obj->oldy + obj->currImagePtr->y2);

	// Combine objbound and boundary bitmaps to local byte map
	for (y = 0; y < YPIX; y++)
		for (x = 0; x < XBYTES; x++)
			for (i = 0; i < 8; i++)
				_boundaryMap[y][x * 8 + i] = ((_vm.getObjectBoundaryOverlay()[y * XBYTES + x] | _vm.getBoundaryOverlay()[y * XBYTES + x]) & (0x80 >> i)) ? kMapBound : 0;

	// Clear all object baselines from objbound
	for (i = 0, obj = _vm._objects; i < _vm._numObj; i++, obj++)
		if ((obj->screenIndex == *_vm._screen_p) && (obj->cycling != INVISIBLE) && (obj->priority == FLOATING))
			_vm.clearBoundary(obj->oldx + obj->currImagePtr->x1, obj->oldx + obj->currImagePtr->x2, obj->oldy + obj->currImagePtr->y2);

#if DEBUG_ROUTE
	{
//	hDC = GetDC(hview);
		for (y = 0; y < YPIX; y++)
			for (x = 0; x < XPIX; x++)
				if (_boundaryMap[y][x])
					SetPixel(hDC, (int16)((long)config.cx * x  / XPIX), (int16)((long)config.cy *(y - DIBOFF_Y) / VIEW_DY), GetPalIndex(_TBRIGHTWHITE));
	}
#endif

	// Search from hero to destination
	segment(herox1, heroy);

//#if DEBUG_ROUTE
//	ReleaseDC(hview, hDC);
//#endif

	// Not found or not enough stack or MAX_SEG exceeded
	if (!_routeFoundFl || _fullStackFl || _fullSegmentFl) {
#if DEBUG_ROUTE
		Box(BOX_ANY, "%s", (_fullStackFl) ? "Stack blown!" : (_fullSegmentFl) ? "Ran out of segments!" : "No Route!");
#endif
		return(false);
	}

	// Now find the route of nodes from destination back to hero
	// Assign first node as destination
	_route[0].x = _destX;
	_route[0].y = _destY;

	// Make a final segment for hero's base (we left a spare)
	_segment[_segmentNumb].y  = heroy;
	_segment[_segmentNumb].x1 = herox1;
	_segment[_segmentNumb].x2 = herox2;
	_segmentNumb++;

	// Look in segments[] for straight lines from destination to hero
	for (i = 0, _routeListIndex = 0; i < _segmentNumb - 1; i++) {
		if ((routeNode = newNode()) == NULL)        // New node for new segment
			return(false);                          // Too many nodes
		routeNode->y = _segment[i].y;

		// Look ahead for furthest straight line
		for (j = i + 1; j < _segmentNumb; j++) {
			seg_p = &_segment[j];
			// Can we get to this segment from previous node?
			if (seg_p->x1 <= routeNode->x && seg_p->x2 >= routeNode->x + _heroWidth - 1)
				routeNode->y = seg_p->y;            // Yes, keep updating node
			else {
				// No, create another node on previous segment to reach it
				if ((routeNode = newNode()) == NULL)    // Add new route node
					return (false);                 // Too many nodes

				// Find overlap between old and new segments
				x1 = MAX(_segment[j - 1].x1, seg_p->x1);
				x2 = MIN(_segment[j - 1].x2, seg_p->x2);

				// If room, add a little offset to reduce staircase effect
				dx = HERO_MAX_WIDTH >> 1;
				if (x2 - x1 < _heroWidth + dx)
					dx = 0;

				// Bear toward final hero position
				if (j == _segmentNumb - 1)
					routeNode->x = herox1;
				else if (herox1 < x1)
					routeNode->x = x1 + dx;
				else if (herox1 > x2 - _heroWidth + 1)
					routeNode->x = x2 - _heroWidth - dx;
				else
					routeNode->x = herox1;
				i = j - 2;                          // Restart segment (-1 to offset auto increment)
				break;
			}
		}

		// Terminate loop if we've reached hero
		if (routeNode->x == herox1 && routeNode->y == heroy)
			break;
	}
	return true;
}

// Process hero in route mode - called from Move_objects()
void Route::processRoute() {
	int16 herox, heroy;                             // Hero position
	Point *routeNode;                               // Ptr to current route node
	static bool turnedFl = false;                   // Used to get extra cylce for turning

	status_t &gameStatus = _vm.getGameStatus();

	debugC(1, kDebugRoute, "processRoute");

	// Current hero position
	herox = _vm._hero->x + _vm._hero->currImagePtr->x1;
	heroy = _vm._hero->y + _vm._hero->currImagePtr->y2;
	routeNode = &_route[gameStatus.routeIndex];

	// Arrived at node?
	if (abs(herox - routeNode->x) < DX + 1 && abs(heroy - routeNode->y) < DY) {
		// DX too low
		// Close enough - position hero exactly
		_vm._hero->x = _vm._hero->oldx = routeNode->x - _vm._hero->currImagePtr->x1;
		_vm._hero->y = _vm._hero->oldy = routeNode->y - _vm._hero->currImagePtr->y2;
		_vm._hero->vx = _vm._hero->vy = 0;
		_vm._hero->cycling = NOT_CYCLING;

		// Arrived at final node?
		if (--gameStatus.routeIndex < 0) {
			// See why we walked here
			switch (gameStatus.go_for) {
			case GO_EXIT:                           // Walked to an exit, proceed into it
				setWalk(_vm._hotspots[gameStatus.go_id].direction);
				break;
			case GO_LOOK:                           // Look at an object
				if (turnedFl) {
					_vm.lookObject(&_vm._objects[gameStatus.go_id]);
					turnedFl = false;
				} else {
					setDirection(_vm._objects[gameStatus.go_id].direction);
					gameStatus.routeIndex++;        // Come round again
					turnedFl = true;
				}
				break;
			case GO_GET:                            // Get (or use) an object
				if (turnedFl) {
					_vm.useObject(gameStatus.go_id);
					turnedFl = false;
				} else {
					setDirection(_vm._objects[gameStatus.go_id].direction);
					gameStatus.routeIndex++;        // Come round again
					turnedFl = true;
				}
				break;
			case GO_SPACE:
				warning("Unhandled gameStatus.go_for GO_STATUS");
				break;
			}
		}
	} else if (_vm._hero->vx == 0 && _vm._hero->vy == 0) {
		// Set direction of travel if at a node
		// Note realignment when changing to (thinner) up/down sprite,
		// otherwise hero could bump into boundaries along route.
		if (herox < routeNode->x)
			setWalk(Common::KEYCODE_RIGHT);
		else if (herox > routeNode->x)
			setWalk(Common::KEYCODE_LEFT);
		else if (heroy < routeNode->y) {
			setWalk(Common::KEYCODE_DOWN);
			_vm._hero->x = _vm._hero->oldx = routeNode->x - _vm._hero->currImagePtr->x1;
		} else if (heroy > routeNode->y) {
			setWalk(Common::KEYCODE_UP);
			_vm._hero->x = _vm._hero->oldx = routeNode->x - _vm._hero->currImagePtr->x1;
		}
	}
}

// Start a new route from hero to cx, cy
// go_for is the purpose, id indexes the exit or object to walk to
// Returns FALSE if route not found
bool Route::startRoute(go_t go_for, int16 id, int16 cx, int16 cy) {
	bool foundFl = false;                           // TRUE if route found ok

	status_t &gameStatus = _vm.getGameStatus();

	debugC(1, kDebugRoute, "startRoute(%d, %d, %d, %d)", go_for, id, cx, cy);

	// Don't attempt to walk if user does not have control
	if (_vm._hero->pathType != USER)
		return false;

	// if inventory showing, make it go away
	if (gameStatus.inventoryState != I_OFF)
		gameStatus.inventoryState = I_UP;

	gameStatus.go_for = go_for;                     // Purpose of trip
	gameStatus.go_id  = id;                         // Index of exit/object

	// Adjust destination to center hero if walking to cursor
	if (gameStatus.go_for == GO_SPACE)
		cx -= HERO_MIN_WIDTH / 2;

	if ((foundFl = findRoute(cx, cy))) {            // Found a route?
		gameStatus.routeIndex = _routeListIndex;    // Node index
		_vm._hero->vx = _vm._hero->vy = 0;      // Stop manual motion
	}

	return foundFl;
}

} // end of namespace Hugo
