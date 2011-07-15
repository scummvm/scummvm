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

class Polygon {
public:
	Common::Array<Common::Point> points;
	Common::Rect  boundingRect;

	Polygon() {};
	Polygon(Common::Point point1, Common::Point point2, Common::Point point3, Common::Point point4) {
		points.push_back(point1);
		points.push_back(point2);
		points.push_back(point3);
		points.push_back(point4);
	}

	bool contains(const Common::Point &point);
	uint32 count() { return points.size(); }
};

class Polygons {
public:
	Polygons(Common::SeekableReadStream *stream);
	virtual ~Polygons();

	Polygon get(uint32 index);
	uint32 size() { return _entries.size(); }

private:
	int32 _size;
	int32 _numEntries;

	Common::Array<Polygon> _entries;

	void load(Common::SeekableReadStream *stream);
}; // end of class Polygons

} // end of namespace Asylum

#endif
