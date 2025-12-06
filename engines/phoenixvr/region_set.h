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

#ifndef PHOENIXVR_REGIONS_H
#define PHOENIXVR_REGIONS_H

#include "common/array.h"
#include "phoenixvr/rectf.h"

namespace Common {
class String;
};

namespace PhoenixVR {
struct Region {
	float a, b, c, d;
	void setEmpty() {
		a = b = c = d = 0;
	}
	RectF toRect() const;
	Common::String toString() const;
	bool contains(float angleX, float angleY) const;
	bool contains(const PointF &p) const {
		return contains(p.x, p.y);
	}
};

class RegionSet {
	Common::Array<Region> _regions;

public:
	RegionSet(const Common::String &fname);
	uint size() const { return _regions.size(); }
	const Region &getRegion(uint idx) const {
		return _regions[idx];
	}
};
} // namespace PhoenixVR

#endif
