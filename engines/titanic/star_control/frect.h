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

#ifndef TITANIC_FRECT_H
#define TITANIC_FRECT_H

namespace Titanic {

/**
 * Floating point rect class
 */
class FRect {
public:
	double left, top;
	double right, bottom;
public:
	FRect() : left(0), top(0), right(0), bottom(0) {}
	FRect(double x1, double y1, double x2, double y2) :
		left(x1), top(y1), right(x2), bottom(y2) {}

	/**
	 * Returns true if the rects equal
	 */
	bool operator==(const FRect &p) const;

	/**
	 * Returns true if the rects are not equal
	 */
	bool operator!=(const FRect &p) const;

	/**
	 * Returns true if the rect is empty
	 */
	bool empty() const;
};

} // End of namespace Titanic

#endif /* TITANIC_FRECT_H */
