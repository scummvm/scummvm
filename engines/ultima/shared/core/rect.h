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

class TextPoint;

typedef Common::Rect Rect;

typedef Common::Point Point;

/**
 * Simple derived point class that converts text coordinates to graphic screen coordinates
 */
class TextPoint : public Common::Point {
public:
	TextPoint() : Common::Point() {}
	TextPoint(int16 x1, int16 y1) : Common::Point(x1 * 8, y1 * 8) {}
};

/**
 * Simple derived rect class that converts text coordinates to graphic screen coordinates
 */
class TextRect : public Common::Rect {
public:
	TextRect() : Common::Rect() {}
	TextRect(int16 x1, int16 y1, int16 x2, int16 y2) : Common::Rect(x1 * 8, y1 * 8, (x2 + 1) * 8, (y2 + 1) * 8) {}
};

} // End of namespace Shared

using Shared::Rect;
using Shared::Point;
using Shared::TextPoint;
using Shared::TextRect;

} // End of namespace Ultima

#endif
