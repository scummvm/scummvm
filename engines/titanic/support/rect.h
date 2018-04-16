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

#ifndef TITANIC_RECT_H
#define TITANIC_RECT_H

#include "common/rect.h"

namespace Titanic {

enum Quadrant {
	Q_CENTER = 0, Q_LEFT, Q_RIGHT, Q_TOP, Q_BOTTOM
};

typedef Common::Point Point;

class Rect : public Common::Rect {
public:
	Rect() : Common::Rect() {}
	Rect(int16 w, int16 h) : Common::Rect(w, h) {}
	Rect(int16 x1, int16 y1, int16 x2, int16 y2) : Common::Rect(x1, y1, x2, y2) {}

	/**
	 * Returns the top/left corner of the rect as a point
	 */
	operator Point() { return Point(left, top); }

	/**
	 * Clear the rect
	 */
	void clear() { left = top = right = bottom = 0; }

	/**
	 * Combine another rect into this one
	 */
	void combine(const Rect &r);

	/**
	 * Constrains/clips to the intersection area of the given rect
	 */
	void constrain(const Rect &r);

	/**
	 * Returns a center point for a given edge or center of the rect
	 */
	Point getPoint(Quadrant quadrant);
};

} // End of namespace Titanic

#endif /* TITANIC_RECT_H */
