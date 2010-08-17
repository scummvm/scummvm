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

#ifndef HUGO_ROUTE_H
#define HUGO_ROUTE_H

namespace Hugo {

#define kMapBound 1                                 // Mark a boundary outline
#define kMapFill  2                                 // Mark a boundary filled
#define kMaxSeg   256                               // Maximum number of segments
#define kMaxNodes 256                               // Maximum nodes in route
#define DEBUG_ROUTE FALSE

struct Point {
	int x;
	int y;
};

struct segment_t {                                  // Search segment
	int16 y;                                        // y position
	int16 x1, x2;                                   // Range of segment
};

class Route {
public:
	Route(HugoEngine &vm);

	void processRoute();
	bool startRoute(go_t go_for, short id, short cx, short cy);
	void setDirection(uint16 keyCode);
	void setWalk(uint16 direction);

private:
	HugoEngine &_vm;

	byte _boundaryMap[YPIX][XPIX];                  // Boundary byte map
	segment_t _segment[kMaxSeg];                    // List of points in fill-path
	Point _route[kMaxNodes];                        // List of nodes in route (global)
	int16 _segmentNumb;                             // Count number of segments
	int16 _routeListIndex;                          // Index into route list
	int16 _destX;
	int16 _destY;
	int16 _heroWidth;                               // Hero width
	bool  _routeFoundFl;                            // TRUE when path found
	bool  _fullStackFl;                             // TRUE if stack exhausted
	bool  _fullSegmentFl;                           // Segments exhausted

	void segment(int16 x, int16 y);
	bool findRoute(int16 cx, int16 cy);
	Point *newNode();
};

} // end of namespace Hugo
#endif //HUGO_ROUTE_H
