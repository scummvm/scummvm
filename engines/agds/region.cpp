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
 */

#include "agds/region.h"
#include "agds/resourceManager.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"

namespace AGDS {

Region::Region(const Common::String &resourceName, Common::SeekableReadStream &stream) {
	auto size = stream.size();
	name = readString(stream);
	center.x = stream.readSint16LE();
	center.y = stream.readSint16LE();
	flags = stream.readUint16LE();
	debug("region %s at (%d,%d) %04x", name.c_str(), center.x, center.y, flags);
	while (stream.pos() + 2 <= size) {
		uint16 ext = stream.readUint16LE();
		if (ext)
			debug("extended entries %u", ext);

		PointsType points;
		while (ext--) {
			int16 a = stream.readSint16LE();
			int16 b = stream.readSint16LE();
			int16 c = stream.readUint16LE();
			if (c != -12851) // 0xcdcd
				debug("extended entry: %d %d %d", a, b, c);
			else
				debug("extended entry: %d %d", a, b);
			points.push_back(Common::Point(a, b));
		}
		regions.push_back(points);
	}
}

Region::Region(const Common::Rect rect) : flags(0) {
	PointsType points;
	points.push_back(Common::Point(rect.left, rect.top));
	points.push_back(Common::Point(rect.right, rect.top));
	points.push_back(Common::Point(rect.right, rect.bottom));
	points.push_back(Common::Point(rect.left, rect.bottom));
	regions.push_back(points);

	center.x = (rect.left + rect.right) / 2;
	center.y = (rect.top + rect.bottom) / 2;
}

Common::String Region::toString() const {
	Common::String str = Common::String::format("region(%d, %d, [", center.x, center.y);
	for (size_t i = 0; i < regions.size(); ++i) {
		if (i != 0)
			str += ", ";
		str += "Region {";
		const PointsType &points = regions[i];
		for (size_t j = 0; j < points.size(); ++j) {
			if (j != 0)
				str += ", ";
			str += Common::String::format("(%d, %d)", points[j].x, points[j].y);
		}
		str += "}";
	}
	str += "]";
	return str;
}

void Region::move(Common::Point rel) {
	if (rel.x == 0 && rel.y == 0)
		return;

	center += rel;
	for (uint i = 0; i < regions.size(); ++i) {
		PointsType &points = regions[i];
		for (uint j = 0; j < points.size(); ++j)
			points[j] += rel;
	}
}

Common::Point Region::topLeft() const {
	if (regions.empty())
		return Common::Point();

	Common::Point p = regions[0][0];
	for (uint i = 0; i < regions.size(); ++i) {
		const PointsType &points = regions[i];

		for (uint j = 0; j < points.size(); ++j) {
			Common::Point point = points[j];
			if (point.x < p.x)
				p.x = point.x;
			if (point.y < p.y)
				p.y = point.y;
		}
	}
	return p;
}

// FIXME: copied from wintermute/base_region.cpp

struct dPoint {
	double x, y;
};

bool Region::pointIn(Common::Point point) const {
	for (uint r = 0; r < regions.size(); ++r) {
		const PointsType &points = regions[r];
		uint32 size = points.size();
		if (size < 3) {
			continue;
		}

		int counter = 0;
		double xinters;
		dPoint p, p1, p2;

		p.x = (double)point.x;
		p.y = (double)point.y;

		p1.x = (double)points[0].x;
		p1.y = (double)points[0].y;

		for (uint32 i = 1; i <= size; i++) {
			p2.x = (double)points[i % size].x;
			p2.y = (double)points[i % size].y;

			if (p.y > MIN(p1.y, p2.y)) {
				if (p.y <= MAX(p1.y, p2.y)) {
					if (p.x <= MAX(p1.x, p2.x)) {
						if (p1.y != p2.y) {
							xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
							if (p1.x == p2.x || p.x <= xinters) {
								counter++;
							}
						}
					}
				}
			}
			p1 = p2;
		}

		if (counter % 2 == 0) {
			continue;
		} else {
			return true;
		}
	}
	return false;
}

} // namespace AGDS
