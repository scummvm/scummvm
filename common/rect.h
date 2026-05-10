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

#ifndef COMMON_RECT_H
#define COMMON_RECT_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/str.h"

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
template<typename T, typename ConcretePoint>
struct PointBase {
	T x;	/*!< The horizontal position of the point. */
	T y;	/*!< The vertical position of the point. */

	constexpr PointBase() : x(0), y(0) {}

	/**
	 * Create a point with position defined by @p x1 and @p y1.
	 */
	constexpr PointBase(T x1, T y1) : x(x1), y(y1) {}
	/**
	 * Determine whether the position of two points is the same.
	 */
	bool  operator==(const ConcretePoint &p)    const { return x == p.x && y == p.y; }
	/**
	 * Determine whether the position of two points is not the same.
	 */
	bool  operator!=(const ConcretePoint &p)    const { return x != p.x || y != p.y; }
	/**
	 * Create a point by adding the @p delta value to a point.
	 */
	ConcretePoint operator+(const ConcretePoint &delta) const { return ConcretePoint(x + delta.x, y + delta.y); }
	/**
	 * Create a point by subtracting the @p delta value from a point.
	 */
	ConcretePoint operator-(const ConcretePoint &delta) const { return ConcretePoint(x - delta.x, y - delta.y); }
	/**
	 * Create a point by dividing a point by the (int) @p divisor value.
	 */
	ConcretePoint operator/(int divisor) const { return ConcretePoint(x / divisor, y / divisor); }
	/**
	 * Create a point by multiplying a point by the (int) @p multiplier value.
	 */
	ConcretePoint operator*(int multiplier) const { return ConcretePoint(x * multiplier, y * multiplier); }
	/**
	 * Create a point by dividing a point by the (double) @p divisor value.
	 */
	ConcretePoint operator/(double divisor) const { return ConcretePoint((T)(x / divisor), (T)(y / divisor)); }
	/**
	 * Create a point by multiplying a point by the (double) @p multiplier value.
	 */
	ConcretePoint operator*(double multiplier) const { return ConcretePoint((T)(x * multiplier), (T)(y * multiplier)); }

	/**
	 * Change a point's position by adding @p delta to its x and y coordinates.
	 */
	void operator+=(const ConcretePoint &delta) {
		x += delta.x;
		y += delta.y;
	}

	/**
	 * Change a point's position by subtracting @p delta from its x and y arguments.
	 */
	void operator-=(const ConcretePoint &delta) {
		x -= delta.x;
		y -= delta.y;
	}

	/**
	 * Return the square of the distance between this point and the point @p p.
	 *
	 * @param p		The other point.
	 * @return      The distance between this and @p p.
	 */
	uint sqrDist(const ConcretePoint &p) const {
		int diffx = ABS(p.x - x);
		if (diffx >= 0x1000)
			return 0xFFFFFF;

		int diffy = ABS(p.y - y);
		if (diffy >= 0x1000)
			return 0xFFFFFF;

		return uint(diffx * diffx + diffy * diffy);
	}

	/**
	 * Return string representation of the point.
	 */
	String toString() const {
		return String::format("%d, %d", x, y);
	}
};

/**
 * Old GCC don't support constructor inheritance
 */
#define BEGIN_POINT_TYPE(T, Point) \
	struct Point : public Common::PointBase<T, Point> {
#define END_POINT_TYPE(T, Point) \
		constexpr Point() : PointBase() {} \
		constexpr Point(T x1, T y1) : PointBase(x1, y1) {} \
	}; \
	static inline Point operator*(int multiplier, const Point &p) { return Point(p.x * multiplier, p.y * multiplier); } \
	static inline Point operator*(double multiplier, const Point &p) { return Point((T)(p.x * multiplier), (T)(p.y * multiplier)); }

BEGIN_POINT_TYPE(int16, Point)
END_POINT_TYPE(int16, Point)
BEGIN_POINT_TYPE(int32, Point32)
		constexpr Point32(const Point &o) : PointBase(o.x, o.y) {}
END_POINT_TYPE(int32, Point32)

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
template<typename T, typename ConcreteRect, typename ConcretePoint>
struct RectBase {
	T top, left;		/*!< The point at the top left of the rectangle (part of the Rect). */
	T bottom, right;	/*!< The point at the bottom right of the rectangle (not part of the Rect). */

	constexpr RectBase() : top(0), left(0), bottom(0), right(0) {}
	/**
	 * Create a rectangle with the top-left corner at position (0, 0) and the given width @p w and height @p h.
	 */
	constexpr RectBase(T w, T h) : top(0), left(0), bottom(h), right(w) {}
	/**
	 * Create a rectangle with the top-left corner at the position @p topLeft
	 * and the bottom-right corner at the position @p bottomRight.
	 *
	 * The @p topLeft x value must be greater or equal @p bottomRight x and
	 * @p topLeft y must be greater or equal @p bottomRight y.
	 */
	RectBase(const ConcretePoint &topLeft, const ConcretePoint &bottomRight) : top(topLeft.y), left(topLeft.x), bottom(bottomRight.y), right(bottomRight.x) {
		assert(isValidRect());
	}
	/**
	 * Create a rectangle with the top-left corner at the position @p topLeft
	 * and the given width @p w and height @p h.
	 */
	constexpr RectBase(const ConcretePoint &topLeft, T w, T h) : top(topLeft.y), left(topLeft.x), bottom(topLeft.y + h), right(topLeft.x + w) {
	}
	/**
	 * Create a rectangle with the top-left corner at the given position (x1, y1)
	 * and the bottom-right corner at the position (x2, y2).
	 *
	 * The @p x2 value must be greater or equal @p x1 and @p y2 must be greater or equal @p y1.
	 */
	RectBase(T x1, T y1, T x2, T y2) : top(y1), left(x1), bottom(y2), right(x2) {
		assert(isValidRect());
	}
	/**
	 * Check if two rectangles are identical.
	 *
	 * @return True if the rectangles are identical, false otherwise.
	 */
	bool operator==(const ConcreteRect &rhs) const { return equals(rhs); }
	/**
	 * Check if two rectangles are different.
	 *
	 * @return True if the rectangles are different, false otherwise.
	 */
	bool operator!=(const ConcreteRect &rhs) const { return !equals(rhs); }

	ConcretePoint origin() const { return ConcretePoint(left, top); } /*!< Return the origin of a rectangle. */
	T width() const { return right - left; }                          /*!< Return the width of a rectangle. */
	T height() const { return bottom - top; }                         /*!< Return the height of a rectangle. */

	void setWidth(T aWidth) {   /*!< Set the width to @p aWidth value. */
		right = left + aWidth;
	}

	void setHeight(T aHeight) { /*!< Set the height to @p aHeight value. */
		bottom = top + aHeight;
	}

	/**
	 * Set the rectangle to a new position and size.
	 */
	void setRect(T newLeft, T newTop, T newRight, T newBottom) {
		left   = newLeft;
		top    = newTop;
		right  = newRight;
		bottom = newBottom;
	}

	/**
	 * Check if the given position is inside this rectangle.
	 *
	 * @param x The horizontal position to check.
	 * @param y The vertical position to check.
	 *
	 * @return True if the given position is inside this rectangle, false otherwise.
	 */
	bool contains(T x, T y) const {
		return (left <= x) && (x < right) && (top <= y) && (y < bottom);
	}

	/**
	 * Check if the given point is inside this rectangle.
	 *
	 * @param p The point to check.
	 *
	 * @return True if the given point is inside this rectangle, false otherwise.
	 */
	bool contains(const ConcretePoint &p) const {
		return contains(p.x, p.y);
	}

	/**
	 * Check if the given Rect is contained inside this rectangle.
	 *
	 * @param r The rectangle to check.
	 *
	 * @return True if the given Rect is inside, false otherwise.
	 */
	bool contains(const ConcreteRect &r) const {
		return (left <= r.left) && (r.right <= right) && (top <= r.top) && (r.bottom <= bottom);
	}

	/**
	 * Check if the given Rect is equal to this one.
	 *
	 * @param r The rectangle to check.
	 *
	 * @return true If the given Rect is equal, false otherwise.
	 */
	bool equals(const ConcreteRect &r) const {
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
	bool intersects(const ConcreteRect &r) const {
		return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
	}

	/**
	 * Find the intersecting rectangle between this rectangle and the given rectangle.
	 *
	 * @param r The intersecting rectangle.
	 *
	 * @return The intersection of the rectangles or an empty rectangle if not intersecting.
	 */
	ConcreteRect findIntersectingRect(const ConcreteRect &r) const {
		if (!intersects(r))
			return ConcreteRect();

		return ConcreteRect(MAX(r.left, left), MAX(r.top, top), MIN(r.right, right), MIN(r.bottom, bottom));
	}

	/**
	 * Extend this rectangle so that it contains @p r.
	 *
	 * @param r The rectangle to extend by.
	 */
	void extend(const ConcreteRect &r) {
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
	void grow(T offset) {
		top -= offset;
		left -= offset;
		bottom += offset;
		right += offset;
	}

	/**
	 * Clip this rectangle to within the bounds of another rectangle @p r.
	 */
	void clip(const ConcreteRect &r) {
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
	 * Reduce the dimensions of this rectangle by setting max width and max height.
	 */
	void clip(T maxw, T maxh) {
		clip(ConcreteRect(0, 0, maxw, maxh));
	}

	/**
	 * Reset the rectangle to an empty state.
	 */
	void setEmpty() {
		left = right = top = bottom = 0;
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
	void moveTo(T x, T y) {
		bottom += y - top;
		right += x - left;
		top = y;
		left = x;
	}

	/**
	 * Move the rectangle by the given delta x and y values.
	 */
	void translate(T dx, T dy) {
		left += dx; right += dx;
		top += dy; bottom += dy;
	}

	/**
	 * Move this rectangle to the position of the point @p p.
	 */
	void moveTo(const ConcretePoint &p) {
		moveTo(p.x, p.y);
	}

	/**
	 * Ensures the rectangle fits in an another one
	 *
	 * @return True if the rectangle could be constrained
	 */
	bool constrain(const ConcreteRect &o) {
		return o.constrain(left, top, width(), height());
	}

	/**
	 * Ensures the provided coordinates fit inside our rectangle
	 *
	 * @return True if the rectangle could be constrained
	 */
	bool constrain(T &x, T &y, T w, T h) const {
		if (w > width() || h > height()) {
			return false;
		}

		if (x < left) {
			x = left;
		} else if (x > right - w) { // x + w > right
			x = right - w;
		}
		if (y < top) {
			y = top;
		} else if (y > bottom - h) { // y + h > bottom
			y = bottom - h;
		}

		return true;
	}

	/**
	 * Print debug messages related to this class.
	 */
	void debugPrint(int debuglevel = 0, const char *caption = "Rect:") const {
		debug(debuglevel, "%s %d, %d, %d, %d", caption, left, top, right, bottom);
	}

	 /**
	 * Print debug messages related to this class.
	 */
	void debugPrintC(int debuglevel, uint32 debugChannel, const char *caption = "Rect:") const {
		debugC(debuglevel, debugChannel, "%s %d, %d, %d, %d", caption, left, top, right, bottom);
	}

	/**
	 * Return string representation of the rectangle.
	 */
	String toString() const {
		return String::format("%d, %d, %d, %d", left, top, right, bottom);
	}

	/**
	 * Create a rectangle around the given center.
	 * @note The center point is rounded up and left when given an odd width and height.
	 */
	static ConcreteRect center(T cx, T cy, T w, T h) {
		T x = cx - w / 2, y = cy - h / 2;
		return ConcreteRect(x, y, x + w, y + h);
	}

	/**
	* Return a Point indicating the centroid of the rectangle
	* @note The center point is rounded up and left when width and/or height are odd
	*/
	ConcretePoint center() const {
		return ConcretePoint((left + right) / 2, (bottom + top) / 2);
	}

	/**
	 * Given target surface with size clip, this function ensures that
	 * blit arguments @p dst and @p rect are within the @p clip rectangle,
	 * shrinking them as necessary.
	 * @param dst  Blit destination coordinates.
	 * @param rect Blit source rectangle.
	 * @param clip Clip rectangle (size of destination surface).
	 */
	static bool getBlitRect(ConcretePoint &dst, ConcreteRect &rect, const ConcreteRect &clip) {
		if (dst.x < clip.left) {
			rect.left += clip.left - dst.x;
			dst.x = clip.left;
		}

		if (dst.y < clip.top) {
			rect.top += clip.top - dst.y;
			dst.y = clip.top;
		}

		int right = dst.x + rect.width();
		if (right > clip.right)
			rect.right -= right - clip.right;

		int bottom = dst.y + rect.height();
		if (bottom > clip.bottom)
			rect.bottom -= bottom - clip.bottom;
		return !rect.isEmpty();
	}
};

/**
 * Old GCC don't support constructor inheritance
 */
#define BEGIN_RECT_TYPE(T, Rect, Point) \
	struct Rect : public Common::RectBase<T, Rect, Point> {

#define END_RECT_TYPE(T, Rect, Point) \
		constexpr Rect() : RectBase() {} \
		constexpr Rect(T w, T h) : RectBase(w, h) {} \
		Rect(const Point &topLeft, const Point &bottomRight) : RectBase(topLeft, bottomRight) {} \
		constexpr Rect(const Point &topLeft, T w, T h) : RectBase(topLeft, w, h) {} \
		Rect(T x1, T y1, T x2, T y2) : RectBase(x1, y1, x2, y2) {} \
	};

BEGIN_RECT_TYPE(int16, Rect, Point)
END_RECT_TYPE(int16, Rect, Point)
BEGIN_RECT_TYPE(int32, Rect32, Point32)
END_RECT_TYPE(int32, Rect32, Point32)
/** @} */

} // End of namespace Common

#endif
