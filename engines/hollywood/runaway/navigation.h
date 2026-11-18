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

#ifndef HOLLYWOOD_RUNAWAY_NAVIGATION_H
#define HOLLYWOOD_RUNAWAY_NAVIGATION_H

#include "common/array.h"
#include "common/rect.h"

namespace Hollywood {

class RunawayWalkMap {
public:
	RunawayWalkMap(int width, int height, const Common::Array<byte> &mask, const Common::Array<byte> &maps) :
		_width(width), _height(height), _mask(mask), _maps(maps) {}
	void findPath(Common::Point start, Common::Point target, Common::Array<Common::Point> &path) const;

private:
	bool walkable(int x, int y) const;
	bool clearPath(Common::Point start, Common::Point end) const;
	int _width, _height;
	const Common::Array<byte> &_mask, &_maps;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_NAVIGATION_H
