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

#ifndef COMMON_RECT_H
#define COMMON_RECT_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/debug.h"

#define PRINT_RECT(x) (x).left,(x).top,(x).right,(x).bottom

namespace Common {

/**
 * @defgroup common_rect Rectangular zones
 * @ingroup common
 *
 * @brief API for operations on rectangular zones.
 *
 * @{
 */

/**
 * Simple class for handling both 2D position and size.
 */
struct Point {
	int16 x;	/*!< The horizontal position of the point. */
	int16 y;	/*!< The vertical position of the point. */

	Point() : x(0), y(0) {}
	
	/**
     * Create a point with position defined by @p x1 and @p y1.
     */
	Point(int16 x1, int16 y1) : x(x1), y(y1) {}
	/**
     * Determine whether the position of two points is the same.
     */
	bool  operator==(const Point &p)    const { return x == p.x && y == p.y; }
	/**
     * Determine whether the position of two points is not the same.
     */
	bool  operator!=(const Point &p)    const { return x != p.x || y != p.y; }
	/**
     * Create a point by adding the @p delta value to a point.
     */
	Point operator+(const Point &delta) const { return Point(x + delta.x, y + delta.y); }
	/**
     * Create a point by subtracting the @p delta value from a point.
     */
	Point operator-(const Point &delta) const { return Point(x - delta.x, y - delta.y); }
	/**
     * Create a point by dividing a point by the (int) @p divisor value.
     */
	Point operator/(int divisor) const { return Point(x / divisor, y / divisor); }
	/**
     * Create a point by multiplying a point by the (int) @p multiplier value.
     */
	Point operator*(int multiplier) const { return Point(x * multiplier, y * multiplier); }
	/**
     * Create a point by dividing a point by the (double) @p divisor value.
     */
	Point operator/(double divisor) const { return Point(x / divisor, y / divisor); }
	/**
     * Create a point by multiplying a point by the (double) @p multiplier value.
     */
	Point operator*(double multiplier) const { return Point(x * multiplier, y * multiplier); }

    /**
     * Change a point's position by adding @p delta to its x and y coordinates.
     */
	void operator+=(const Point &delta) {
		x += delta.x;
		y += delta.y;
	}

    /**
     * Change a point's position by subtracting @p delta from its x and y arguments.
     */
	void operator-=(const Point &delta) {
		x -= delta.x;
		y -= delta.y;
	}

	/**
	 * Return the square of the distance between this point and the point @p p.
	 *
	 * @param p		The other point.
	 * @return      The distance between this and @p p.
	 */
	uint sqrDist(const Point &p) const {
		int diffx = ABS(p.x - x);
		if (diffx >= 0x1000)
			return 0xFFFFFF;

		int diffy = ABS(p.y - y);
		if (diffy >= 0x1000)
			return 0xFFFFFF;

		return uint(diffx * diffx + diffy * diffy);
	}
};

static inline Point operator*(int multiplier, const Point &p) { return Point(p.x * multiplier, p.y * multiplier); }
static inline Point operator*(double multiplier, const Point &p) { return Point(p.x * multiplier, p.y * multiplier); }

/**
 * Simple class for handling a rectangular zone.
 *
 * Note: This implementation is built around the assumption that (top,left) is
 * part of the rectangle, but (bottom,right) is not. This is reflected in
 * various methods, including contains(), intersects(), and others.
 *
 * Another very widespread approach to rectangle classes treats (bottom,right)
 * also as a part of the rectangle.
 *
 * Conceptually, both are sound, but the approach we use saves many intermediate
 * computations (like computing the height in our case is done by doing this:
 *   height = bottom - top;
 * while in the alternate system, it would be
 *   height = bottom - top + 1;
 *
 * When writing code using our Rect class, always keep this principle in mind!
*/
struct Rect {
	int16 top, left;		/*!< The point at the top left of the rectangle (part of the Rect). */
	int16 bottom, right;	/*!< The point at the bottom right of the rectangle (not part of the Rect). */

	Rect() : top(0), left(0), bottom(0), right(0) {}
	/**
     * Create a rectangle with the top-left corner at position (0, 0) and the given width @p w and height @p h.
     */
	Rect(int16 w, int16 h) : top(0), left(0), bottom(h), right(w) {}
	/**
     * Create a rectangle with the top-left corner at the given position (x1, y1)
	 * and the bottom-right corner at the position (x2, y2).
	 * 
	 * The @p x2 value must be greater or equal @p x1 and @p y2 must be greater or equal @p y1.
     */
	Rect(int16 x1, int16 y1, int16 x2, int16 y2) : top(y1), left(x1), bottom(y2), right(x2) {
		assert(isValidRect());
	}
	/**
     * Check if two rectangles are identical.
	 *
	 * @return True if the rectangles are identical, false otherwise.
     */
	bool operator==(const Rect &rhs) const { return equals(rhs); }
    /**
     * Check if two rectangles are different.
	 *
	 * @return True if the rectangles are different, false otherwise.
     */
	bool operator!=(const Rect &rhs) const { return !equals(rhs); }

	int16 width() const { return right - left; }  /*!< Return the width of a rectangle. */
	int16 height() const { return bottom - top; } /*!< Return the height of a rectangle. */

	void setWidth(int16 aWidth) {   /*!< Set the width to @p aWidth value. */
		right = left + aWidth;
	}

	void setHeight(int16 aHeight) { /*!< Set the height to @p aHeight value. */
		bottom = top + aHeight;
	}

	/**
	 * Check if the given position is inside this rectangle.
	 *
	 * @param x The horizontal position to check.
	 * @param y The vertical position to check.
	 *
	 * @return True if the given position is inside this rectangle, false otherwise.
	 */
	bool contains(int16 x, int16 y) const {
		return (left <= x) && (x < right) && (top <= y) && (y < bottom);
	}

	/**
	 * Check if the given point is inside this rectangle.
	 *
	 * @param p The point to check.
	 *
	 * @return True if the given point is inside this rectangle, false otherwise.
	 */
	bool contains(const Point &p) const {
		return contains(p.x, p.y);
	}

	/**
	 * Check if the given Rect is contained inside this rectangle.
	 *
	 * @param r The rectangle to check.
	 *
	 * @return True if the given Rect is inside, false otherwise.
	 */
	bool contains(const Rect &r) const {
		return (left <= r.left) && (r.right <= right) && (top <= r.top) && (r.bottom <= bottom);
	}

	/**
	 * Check if the given Rect is equal to this one.
	 *
	 * @param r The rectangle to check.
	 *
	 * @return true If the given Rect is equal, false otherwise.
	 */
	bool equals(const Rect &r) const {
		return (left == r.left) && (right == r.right) && (top == r.top) && (bottom == r.bottom);
	}

	/**
	 * Check if the given rectangle intersects with this rectangle.
	 *
	 * @param r The rectangle to check.
	 *
	 * @return True if the given rectangle has a non-empty intersection with
	 *         this rectangle, false otherwise.
	 */
	bool intersects(const Rect &r) const {
		return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
	}

	/**
	 * Find the intersecting rectangle between this rectangle and the given rectangle.
	 *
	 * @param r The intersecting rectangle.
	 *
	 * @return The intersection of the rectangles or an empty rectangle if not intersecting.
	 */
	Rect findIntersectingRect(const Rect &r) const {
		if (!intersects(r))
			return Rect();

		return Rect(MAX(r.left, left), MAX(r.top, top), MIN(r.right, right), MIN(r.bottom, bottom));
	}

	/**
	 * Extend this rectangle so that it contains @p r.
	 *
	 * @param r The rectangle to extend by.
	 */
	void extend(const Rect &r) {
		left = MIN(left, r.left);
		right = MAX(right, r.right);
		top = MIN(top, r.top);
		bottom = MAX(bottom, r.bottom);
	}

	/**
	 * Extend this rectangle in all four directions by the given number of pixels.
	 *
	 * @param offset The size to grow by.
	 */
	void grow(int16 offset) {
		top -= offset;
		left -= offset;
		bottom += offset;
		right += offset;
	}

	/**
	 * Clip this rectangle with another rectangle @p r.
	 */
	void clip(const Rect &r) {
		assert(isValidRect());
		assert(r.isValidRect());

		if (top < r.top) top = r.top;
		else if (top > r.bottom) top = r.bottom;

		if (left < r.left) left = r.left;
		else if (left > r.right) left = r.right;

		if (bottom > r.bottom) bottom = r.bottom;
		else if (bottom < r.top) bottom = r.top;

		if (right > r.right) right = r.right;
		else if (right < r.left) right = r.left;
	}

   	/**
	 * Reduce the dimensions of this rectangle by setting max width and max heigth.
	 */
	void clip(int16 maxw, int16 maxh) {
		clip(Rect(0, 0, maxw, maxh));
	}

   	/**
	 * Check if the rectangle is empty (its width or length is 0) or invalid (its width or length are negative).
	 *
	 * @retval true  The rectangle is empty or invalid.
	 * @retval false The rectangle is valid and not empty.
	 */
	bool isEmpty() const {
		return (left >= right || top >= bottom);
	}

    /**
	 * Check if this is a valid rectangle.
	 */
	bool isValidRect() const {
		return (left <= right && top <= bottom);
	}

    /**
	 * Move this rectangle to the position defined by @p x, @p y. 
	 */
	void moveTo(int16 x, int16 y) {
		bottom += y - top;
		right += x - left;
		top = y;
		left = x;
	}

	/**
	 * Move the rectangle by the given delta x and y values.
	 */
	void translate(int16 dx, int16 dy) {
		left += dx; right += dx;
		top += dy; bottom += dy;
	}

    /**
	 * Move this rectangle to the position of the point @p p. 
	 */
	void moveTo(const Point &p) {
		moveTo(p.x, p.y);
	}

     /**
	 * Print debug messages related to this class. 
	 */
	void debugPrint(int debuglevel = 0, const char *caption = "Rect:") const {
		debug(debuglevel, "%s %d, %d, %d, %d", caption, left, top, right, bottom);
	}

	/**
	 * Create a rectangle around the given center.
	 * @note The center point is rounded up and left when given an odd width and height.
	 */
	static Rect center(int16 cx, int16 cy, int16 w, int16 h) {
		int x = cx - w / 2, y = cy - h / 2;
		return Rect(x, y, x + w, y + h);
	}

	/**
	 * Given target surface with size clip, this function ensures that
	 * blit arguments @p dst and @p rect are within the @p clip rectangle.
	 * @param dst  Blit destination coordinates.
	 * @param rect Blit source rectangle.
	 * @param clip Clip rectangle (size of destination surface).
	 */
	static bool getBlitRect(Point &dst, Rect &rect, const Rect &clip) {
		if (dst.x < clip.left) {
			rect.left += clip.left - dst.x;
			dst.x = clip.left;
		}

		if (dst.y < clip.top) {
			rect.top += clip.top - dst.y;
			dst.y = clip.top;
		}

		int right = dst.x + rect.right;
		if (right > clip.right)
			rect.right -= right - clip.right;

		int bottom = dst.y + rect.bottom;
		if (bottom > clip.bottom)
			rect.bottom -= bottom - clip.bottom;
		return !rect.isEmpty();
	}
};

/** @} */

} // End of namespace Common

#endif
