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
 */

#ifndef ASYLUM_POLYGONS_H
#define ASYLUM_POLYGONS_H

#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"

#define MAX_POLYGONS 200

namespace Asylum {

typedef struct PolyDefinitions {
	Common::Array<Common::Point> points;
	Common::Rect  boundingRect;

	PolyDefinitions() {};
	PolyDefinitions(Common::Point point1, Common::Point point2, Common::Point point3, Common::Point point4) {
		points.push_back(point1);
		points.push_back(point2);
		points.push_back(point3);
		points.push_back(point4);
	}

	/**
	 * Check if the x/y coordinates exist within
	 * the current polygon definition
	 *
	 * (was pointInPoly())
	 */
	bool contains(int16 x, int16 y);

	bool contains(Common::Point point) {
		return contains(point.x, point.y);
	}

	uint32 count() {
		return points.size();
	}

} PolyDefinitions;

class Polygons {
public:
	Polygons(Common::SeekableReadStream *stream);
	virtual ~Polygons();

	int32 size;
	int32 numEntries; // TODO remove and use entries.size()

	Common::Array<PolyDefinitions> entries;

	static bool contains(Common::Point *points, uint32 count, Common::Point point, Common::Rect *boundingRect);
	static bool contains(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4);

private:
	void load(Common::SeekableReadStream *stream);

	static int32 compareDistance(int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3);
	static bool containsRect(Common::Point *points, uint32 count, Common::Point point, Common::Rect *boundingRect);

}; // end of class Polygons

} // end of namespace Asylum

#endif
