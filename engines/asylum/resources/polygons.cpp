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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/resources/polygons.h"

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// Contains
//////////////////////////////////////////////////////////////////////////
bool Polygon::contains(const Common::Point &point) {
	// Copied from backends/vkeybd/polygon.cpp
	bool  yflag0;
	bool  yflag1;
	bool inside_flag = false;

	// if no points are defined, an intersect check will fail
	// (count - 1 would trigger an assertion in vtx0)
	if (points.size() == 0)
		return false;

	Common::Point *vtx0 = &points[count() - 1];
	Common::Point *vtx1 = &points[0];

	yflag0 = (vtx0->y > point.y);
	for (uint32 pt = 0; pt < count(); pt++, vtx1++) {
		if (point == *vtx1)
			return true;

		yflag1 = (vtx1->y > point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - point.y) * (vtx0->x - vtx1->x) > (vtx1->x - point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0   = vtx1;
	}

	return inside_flag;
}

//////////////////////////////////////////////////////////////////////////
// Polygons
//////////////////////////////////////////////////////////////////////////
Polygons::Polygons(Common::SeekableReadStream *stream) : _size(0), _numEntries(0) {
	load(stream);
}

Polygons::~Polygons() {
	_entries.clear();
}

Polygon Polygons::get(uint32 index) {
	if (index >= _entries.size())
		error("[Polygons::getEntry] Invalid polygon index (was: %d, max: %d)", index, _entries.size() - 1);

	return _entries[index];
}

void Polygons::load(Common::SeekableReadStream *stream) {
	_size       = stream->readSint32LE();
	_numEntries = stream->readSint32LE();

	for (int32 g = 0; g < _numEntries; g++) {
		Polygon poly;

		uint32 numPoints = stream->readUint32LE();

		for (uint32 i = 0; i < numPoints; i++) {
			Common::Point point;
			point.x = (int16)(stream->readSint32LE() & 0xFFFF);
			point.y = (int16)(stream->readSint32LE() & 0xFFFF);

			poly.points.push_back(point);
		}

		stream->skip((MAX_POLYGONS - numPoints) * 8);

		poly.boundingRect.left   = (int16)(stream->readSint32LE() & 0xFFFF);
		poly.boundingRect.top    = (int16)(stream->readSint32LE() & 0xFFFF);
		poly.boundingRect.right  = (int16)(stream->readSint32LE() & 0xFFFF);
		poly.boundingRect.bottom = (int16)(stream->readSint32LE() & 0xFFFF);

		_entries.push_back(poly);
	}
}

} // end of namespace Asylum
