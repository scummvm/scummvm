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

#ifndef REGION_H
#define REGION_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/stream.h"

namespace AGDS {

struct Region {
	using PointsType = Common::Array<Common::Point>;

	Common::String name;
	Common::Point center;
	uint16 flags;
	Common::Array<PointsType> regions;

	Region(const Common::String &resourceName, Common::SeekableReadStream &stream);
	Region(const Common::Rect rect);

	bool pointIn(Common::Point point) const;
	Common::String toString() const;
	void move(Common::Point rel);
	Common::Point topLeft() const;

	bool empty() const {
		return regions.empty();
	}
};

} // End of namespace AGDS

#endif /* AGDS_REGION_H */
