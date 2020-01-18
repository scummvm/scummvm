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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_SHARED_CORE_RECT_H
#define ULTIMA_SHARED_CORE_RECT_H

#include "common/rect.h"

namespace Ultima {
namespace Shared {

typedef Common::Rect Rect;

struct Point : public Common::Point {
	Point() : Common::Point() {}
	Point(int16 x1, int16 y1) : Common::Point(x1, y1) {}
	Point(const Common::Point &pt) : Common::Point(pt.x, pt.y) {}
};

class TextPoint : public Common::Point {
public:
	TextPoint() : Point() {}
	TextPoint(int16 x1, int16 y1) : Point(x1, y1) {}

	operator Point() const {
		return Point(x * 8, y * 8);
	}
};

} // End of namespace Shared

using Shared::Rect;
using Shared::Point;
using Shared::TextPoint;

} // End of namespace Ultima

#endif
