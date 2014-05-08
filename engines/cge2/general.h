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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_GENERAL_H
#define CGE2_GENERAL_H

#include "common/file.h"
#include "common/rect.h"
#include "cge2/fileio.h"

namespace CGE2 {

#define SCR_WID_ 320

struct Dac {
	uint8 _r;
	uint8 _g;
	uint8 _b;
};

// From CGETYPE.H:
class V3D {
public:
	double _x, _y, _z;
	V3D(void) { }
	V3D(double x, double y, double z = 0) : _x(x), _y(y), _z(z) { }
	V3D(const V3D &p) : _x(p._x), _y(p._y), _z(p._z) { }
	V3D operator + (const V3D& p) const { return V3D(_x + p._x, _y + p._y, _z + p._z); }
	V3D operator - (const V3D& p) const { return V3D(_x - p._x, _y - p._y, _z - p._z); }
	V3D operator * (long n) const { return V3D(_x * n, _y * n, _z * n); }
	V3D operator / (long n) const { return V3D(_x / n, _y / n, _z / n); }
	bool operator == (V3D& p) const { return _x == p._x && _y == p._y && _z == p._z; }
	bool operator != (V3D& p) const { return _x != p._x || _y != p._y || _z != p._z; }
	V3D& operator += (const V3D& x) { return *this = *this + x; }
	V3D& operator -= (const V3D& x) { return *this = *this - x; }
};

class V2D : public Common::Point {
	static double trunc(double d) { return (d > 0) ? floor(d) : ceil(d); }
	static double round(double number) { return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5); }
public:
	V2D& operator = (const V3D& p3) {
		double m = Eye._z / (p3._z - Eye._z);
		x = round((Eye._x + (Eye._x - p3._x) * m));
		y = round((Eye._y + (Eye._y - p3._y) * m));
		return *this;
	}
	V2D(void) { }
	V2D(const V3D& p3) { *this = p3; }
	V2D(int x, int y) : Common::Point(x, y) { }
	static V3D Eye;
	static void setEye(const V3D &e) { Eye = e; }
	static void setEye(const V2D& e2, int z = -SCR_WID_) {
		Eye._x = e2.x; Eye._y = e2.y; Eye._z = z;
	}
	static void setEye(const char *s) {
		char *tempStr;
		strcpy(tempStr, s);
		Eye._x = atoi(EncryptedStream::token(tempStr));
		Eye._y = atoi(EncryptedStream::token(tempStr));
		Eye._z = atoi(EncryptedStream::token(tempStr));
	}
	bool operator <  (const V2D& p) const { return (x <  p.x) && (y <  p.y); }
	bool operator <= (const V2D& p) const { return (x <= p.x) && (y <= p.y); }
	bool operator >(const V2D& p) const { return (x >  p.x) && (y >  p.y); }
	bool operator >= (const V2D& p) const { return (x >= p.x) && (y >= p.y); }
	V2D operator + (const V2D& p) const { return V2D(x + p.x, y + p.y); }
	V2D operator - (const V2D& p) const { return V2D(x - p.x, y - p.y); }
	uint16 area(void) { return x * y; }
	bool limited(const V2D& p) {
		return (uint16(x) < uint16(p.x)) && (uint16(y) < uint16(p.y));
	}
	V2D scale(int z) {
		double m = Eye._z / (Eye._z - z);
		return V2D(trunc(m * x), trunc(m * y));
	}
	static V3D screenToGround(V2D pos) {
		double z = V2D::Eye._z + (V2D::Eye._y*V2D::Eye._z) / (double(pos.y) - V2D::Eye._y);
		double x = V2D::Eye._x - ((double(pos.x) - V2D::Eye._x) * (z - V2D::Eye._z)) / V2D::Eye._z;
		return V3D(round(x), 0, round(z));
	}
};

} // End of namespace CGE2

#endif // CGE2_GENERAL_H
