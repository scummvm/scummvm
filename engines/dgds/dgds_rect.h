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

#ifndef DGDS_DGDS_RECT_H
#define DGDS_DGDS_RECT_H

#include "common/str.h"
#include "common/rect.h"

namespace Dgds {

// TODO: This should be replaced by Common::Rect, but during dev it's easier
// to have one that exactly matches semantics of the origingal game's struct.
class DgdsRect {
public:
	DgdsRect(): x(0), y(0), width(0), height(0) {}
	DgdsRect(int x_, int y_, int width_, int height_): x(x_), y(y_), width(width_), height(height_) {}
	int x;
	int y;
	int width;
	int height;

	Common::Rect toCommonRect() const { return Common::Rect(x, y, x + width, y + height); }
	bool contains(const Common::Point &pt) const { return pt.x >= x && pt.x < (x + width) && pt.y >= y && pt.y < (y + height); }
	Common::String dump(const Common::String &indent) const;
};

} // end namespace Dgds

#endif // DGDS_DGDS_RECT_H
