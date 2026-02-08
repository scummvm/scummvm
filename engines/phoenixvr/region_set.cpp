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

#include "phoenixvr/region_set.h"
#include "common/debug.h"
#include "common/file.h"
#include "phoenixvr/math.h"

namespace PhoenixVR {
RegionSet::RegionSet(const Common::Path &fname) {
	Common::File file;
	if (!file.open(fname)) {
		debug("can't find region %s", fname.toString().c_str());
		return;
	}
	auto n = file.readUint32LE();
	while (n--) {
		auto a = file.readFloatLE();
		auto b = file.readFloatLE();
		auto c = file.readFloatLE();
		auto d = file.readFloatLE();
		_regions.push_back(Region{MIN(a, b), MAX(a, b), MIN(c, d), MAX(c, d)});
		debug("region %s", _regions.back().toString().c_str());
	}
}

Common::String Region::toString() const {
	return Common::String::format("{ x: %g,%g, y:%g,%g }", a, b, c, d);
}

RectF Region::toRect() const {
	RectF rect;
	rect.left = MIN(a, b);
	rect.right = MAX(a, b);
	rect.top = MIN(c, d);
	rect.bottom = MAX(c, d);
	return rect;
}

bool Region::contains3D(float angleX, float angleY) const {
	float x0 = a, x1 = b;
	float y0 = c, y1 = d;
	if (x1 - x0 > kPi) {
		float t = x0 + kTau;
		x0 = x1;
		x1 = t;
	}
	if (y1 - y0 > kPi) {
		float t = y0 + kTau;
		y0 = y1;
		y1 = t;
	}
	float ax_pi2 = angleX + kTau;
	if ((angleX >= x0 && angleX <= x1) || (ax_pi2 >= x0 && ax_pi2 <= x1)) {
		if (angleY >= y0 && angleY <= y1)
			return true;

		float ay_pi2 = angleY + kTau;
		if (ay_pi2 < y0)
			return false;
		if (ay_pi2 <= y1)
			return true;
	}
	return false;
}

} // namespace PhoenixVR
