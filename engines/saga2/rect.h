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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_RECT_H
#define SAGA2_RECT_H

#include "common/memstream.h"

namespace Saga2 {

struct StaticPoint16 {
	int16 x, y;

	void set(int16 nx, int16 ny) {
		x = nx;
		y = ny;
	}
};

class Point16 {
public:
	int16               x, y;

	// constructors
	Point16() { x = y = 0; }
	Point16(int16 nx, int16 ny) {
		x = nx;
		y = ny;
	}

	Point16(StaticPoint16 p) {
		x = p.x;
		y = p.y;
	}

	void load(Common::SeekableReadStream *stream);
	void write(Common::MemoryWriteStreamDynamic *out);

	// Point16 operators
	friend Point16 operator+ (Point16 a, Point16 b) {
		return Point16(a.x + b.x, a.y + b.y);
	}

	friend Point16 operator- (Point16 a) {
		return Point16(-a.x, -a.y);
	}

	friend Point16 operator- (Point16 a, Point16 b) {
		return Point16(a.x - b.x, a.y - b.y);
	}

	friend Point16 operator* (Point16 a, int b) {
		return Point16(a.x * (int16)b, a.y * (int16)b);
	}

	friend Point16 operator/ (Point16 a, int b) {
		return Point16(a.x / (int16)b, a.y / (int16)b);
	}

	friend Point16 operator>>(Point16 a, int b) {
		return Point16(a.x >> (int16)b, a.y >> (int16)b);
	}

	friend Point16 operator<<(Point16 a, int b) {
		return Point16(a.x << (int16)b, a.y << (int16)b);
	}

	friend int     operator==(Point16 a, Point16 b) {
		return a.x == b.x && a.y == b.y;
	}

	friend int     operator!=(Point16 a, Point16 b) {
		return a.x != b.x || a.y != b.y;
	}

	void operator+= (Point16 a) {
		x += a.x;
		y += a.y;
	}
	void operator-= (Point16 a) {
		x -= a.x;
		y -= a.y;
	}
	void operator*= (int a)     {
		x *= (int16)a;
		y *= (int16)a;
	}
	void operator/= (int a)     {
		x /= (int16)a;
		y /= (int16)a;
	}
	void operator>>=(int a)     {
		x >>= (int16)a;
		y >>= (int16)a;
	}
	void operator<<=(int a)     {
		x <<= (int16)a;
		y <<= (int16)a;
	}

	void debugPrint(int level = 0, const char *msg = "Point32:") {
		debug(level, "%s %d,%d", msg, x, y);
	}
	// Point16 functions

};

typedef Point16         Vector16;           // vectors are treated as points

//  An Extent is like a rect, but with only size, not position.

typedef Point16         Extent16;               // contains width and height

/* ===================================================================== *
   Point32: 32-bit 2-D point
 * ===================================================================== */

struct StaticPoint32 {
	int32 x, y;

	void set(int16 nx, int16 ny) {
		x = nx;
		y = ny;
	}

	friend StaticPoint32 operator+ (StaticPoint32 a, StaticPoint32 b) {
		StaticPoint32 p;
		p.x = a.x + b.x;
		p.y = a.y + b.y;
		return p;
	}

	friend StaticPoint32 operator- (StaticPoint32 a, StaticPoint32 b) {
		StaticPoint32 p;
		p.x = a.x - b.x;
		p.y = a.y - b.y;
		return p;
	}

	friend StaticPoint32 operator*(StaticPoint32 a, int b) {
		StaticPoint32 p;
		p.x = a.x * b;
		p.y = a.y * b;

		return p;
	}

	friend StaticPoint32 operator/(StaticPoint32 a, int b) {
		StaticPoint32 p;
		p.x = a.x / b;
		p.y = a.y / b;

		return p;
	}

	void operator+= (StaticPoint32 a) {
		x += a.x;
		y += a.y;
	}
};

class Point32 {
public:
	int32               x, y;

	// constructors
	Point32() { x = y = 0; }
	Point32(int32 nx, int32 ny) {
		x = nx;
		y = ny;
	}

	//  Conversion operators
	Point32(Point16 p) {
		x = p.x;
		y = p.y;
	}
	operator Point16() {
		return Point16((int16)x, (int16)y);
	}

	Point32(StaticPoint32 p) {
		x = p.x;
		y = p.y;
	}

	// Point32 operators
	friend Point32 operator+ (Point32 a, Point32 b) {
		return Point32(a.x + b.x, a.y + b.y);
	}

	friend Point32 operator- (Point32 a) {
		return Point32(-a.x, -a.y);
	}

	friend Point32 operator- (Point32 a, Point32 b) {
		return Point32(a.x - b.x, a.y - b.y);
	}

	friend Point32 operator* (Point32 a, int b) {
		return Point32(a.x * (int32)b, a.y * (int32)b);
	}

	friend Point32 operator/ (Point32 a, int b) {
		return Point32(a.x / (int32)b, a.y / (int32)b);
	}

	friend Point32 operator>>(Point32 a, int b) {
		return Point32(a.x >> (int32)b, a.y >> (int32)b);
	}

	friend Point32 operator<<(Point32 a, int b) {
		return Point32(a.x << (int32)b, a.y << (int32)b);
	}

	friend int     operator==(Point32 a, Point32 b) {
		return a.x == b.x && a.y == b.y;
	}

	friend int     operator!=(Point32 a, Point32 b) {
		return a.x != b.x || a.y != b.y;
	}

	void operator+= (Point32 a) {
		x += a.x;
		y += a.y;
	}
	void operator-= (Point32 a) {
		x -= a.x;
		y -= a.y;
	}
	void operator*= (int a)     {
		x *= (int32)a;
		y *= (int32)a;
	}
	void operator/= (int a)     {
		x /= (int32)a;
		y /= (int32)a;
	}
	void operator>>=(int a)     {
		x >>= (int32)a;
		y >>= (int32)a;
	}
	void operator<<=(int a)     {
		x <<= (int32)a;
		y <<= (int32)a;
	}

	void debugPrint(int level = 0, const char *msg = "Point32:") {
		debug(level, "%s %d,%d", msg, x, y);
	}
};

typedef Point32         Vector32;           // vectors are treated as points

//  An Extent is like a rect, but with only size, not position.

typedef Point32         Extent32;               // contains width and height

struct StaticRect {
	int16 x, y, width, height;
};

/* ===================================================================== *
   Rect16: 16-bit 2-D rectangle
 * ===================================================================== */

class Rect16 {
public:
	int16               x, y,
	                    width, height;

	// constructors
	Rect16() { x = y = width = height = 0; }
	Rect16(int16 nx, int16 ny, int16 nw, int16 nh) {
		x = nx;
		y = ny;
		width = nw;
		height = nh;
	}
	Rect16(Point16 a, int16 nw, int16 nh) {
		x = a.x;
		y = a.y;
		width = nw;
		height = nh;
	}
	Rect16(Point16 a, Point16 b) {
		x = a.x;
		y = a.y;
		width = (int16)(b.x - a.x);
		height = (int16)(b.y - a.y);
	}

	Rect16(StaticRect r) {
		x = r.x;
		y = r.y;
		width = r.width;
		height = r.height;
	}

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);

	//  Rect16 operators

	friend int     operator==(Rect16 a, Rect16 b) {
		return  a.x == b.x
		        && a.y == b.y
		        && a.width == b.width
		        && a.height == b.height;
	}

	friend int     operator!=(Rect16 a, Rect16 b) {
		return  a.x != b.x
		        || a.y != b.y
		        || a.width != b.width
		        || a.height != b.height;
	}

	friend Rect16 operator+ (Rect16 a, Point16 b) {
		return Rect16(a.x + b.x, a.y + b.y, a.width, a.height);
	}

	friend Rect16 operator+ (Point16 b, Rect16 a) {
		return Rect16(a.x + b.x, a.y + b.y, a.width, a.height);
	}

	friend Rect16 operator- (Rect16 a, Point16 b) {
		return Rect16(a.x - b.x, a.y - b.y, a.width, a.height);
	}

	void operator+= (Point16 a) {
		x += a.x;
		y += a.y;
	}
	void operator-= (Point16 a) {
		x -= a.x;
		y -= a.y;
	}

	// functions
	void normalize();                        // make rect right-side out
	void expand(int16 dx, int16 dy);             // grow or shrink the rect
	void expand(int16 left, int16 top, int16 right, int16 bottom);

	int empty() const {
		return width <= 0 || height <= 0;
	}

	bool ptInside(int16 px, int16 py) const {         // true if point inside
		return px >= x && px < (x + width) && py >= y && py < (y + height);
	}
	bool ptInside(const Point16 p) const {
		return ptInside(p.x, p.y);
	}
	bool overlap(const Rect16 &r) const;
};

//  Intersect two rectangles

Rect16 intersect(const Rect16, const Rect16);
Rect16 bound(const Rect16, const Rect16);

/* ===================================================================== *
   Rect32: 16-bit 2-D rectangle
 * ===================================================================== */

class Rect32 {
public:
	int32               x, y,
	                    width, height;

	// constructors
	Rect32() { x = y = width = height = 0; }
	Rect32(int32 nx, int32 ny, int32 nw, int32 nh) {
		x = nx;
		y = ny;
		width = nw;
		height = nh;
	}
	Rect32(Point16 a, int32 nw, int32 nh) {
		x = a.x;
		y = a.y;
		width = nw;
		height = nh;
	}
	Rect32(Point16 a, Point16 b) {
		x = a.x;
		y = a.y;
		width = (int32)(b.x - a.x);
		height = (int32)(b.y - a.y);
	}
	Rect32(Point32 a, int32 nw, int32 nh) {
		x = a.x;
		y = a.y;
		width = nw;
		height = nh;
	}
	Rect32(Point32 a, Point32 b) {
		x = a.x;
		y = a.y;
		width = (int32)(b.x - a.x);
		height = (int32)(b.y - a.y);
	}
	//  Conversion operators
	Rect32(Rect16 r) {
		x = r.x;
		y = r.y;
		width = r.width;
		height = r.height;
	}
	operator Rect16() {
		return Rect16((int16)x, (int16)y, (int16)width, (int16)height);
	}

	//  Rect32 operators

	friend int     operator==(Rect32 a, Rect32 b) {
		return  a.x == b.x
		        && a.y == b.y
		        && a.width == b.width
		        && a.height == b.height;
	}

	friend int     operator!=(Rect32 a, Rect32 b) {
		return  a.x != b.x
		        || a.y != b.y
		        || a.width != b.width
		        || a.height != b.height;
	}

	friend Rect32 operator+ (Rect32 a, Point16 b) {
		return Rect32(a.x + b.x, a.y + b.y, a.width, a.height);
	}

	friend Rect32 operator+ (Point16 b, Rect32 a) {
		return Rect32(a.x + b.x, a.y + b.y, a.width, a.height);
	}

	friend Rect32 operator- (Rect32 a, Point16 b) {
		return Rect32(a.x - b.x, a.y - b.y, a.width, a.height);
	}

	friend Rect32 operator+ (Point32 b, Rect32 a) {
		return Rect32(a.x + b.x, a.y + b.y, a.width, a.height);
	}

	friend Rect32 operator- (Rect32 a, Point32 b) {
		return Rect32(a.x - b.x, a.y - b.y, a.width, a.height);
	}

	void operator+= (Point16 a) {
		x += a.x;
		y += a.y;
	}
	void operator-= (Point16 a) {
		x -= a.x;
		y -= a.y;
	}

	void operator+= (Point32 a) {
		x += a.x;
		y += a.y;
	}
	void operator-= (Point32 a) {
		x -= a.x;
		y -= a.y;
	}
	// functions
	void normalize();                        // make rect right-side out

	void expand(int32 dx, int32 dy);             // grow or shrink the rect
	void expand(int32 left, int32 top, int32 right, int32 bottom);

	int empty() const {
		return width <= 0 || height <= 0;
	}

	bool ptInside(int32 px, int32 py) const {         // true if point inside
		return px >= x && px < (x + width) && py >= y && py < (y + height);
	}
	bool ptInside(const Point16 p) const {
		return ptInside(p.x, p.y);
	}

	bool overlap(const Rect32 &r) const;
};

//  Intersect two rectangles

Rect32 intersect(const Rect32, const Rect32);
Rect32 bound(const Rect32, const Rect32);

} // end of namespace Saga2

#endif
