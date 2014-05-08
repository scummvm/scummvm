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
	double X, Y, Z;
	V3D(void) { }
	V3D(double x, double y, double z = 0) : X(x), Y(y), Z(z) { }
	V3D(const V3D &p) : X(p.X), Y(p.Y), Z(p.Z) { }
	V3D operator + (const V3D& p) const { return V3D(X + p.X, Y + p.Y, Z + p.Z); }
	V3D operator - (const V3D& p) const { return V3D(X - p.X, Y - p.Y, Z - p.Z); }
	V3D operator * (long n) const { return V3D(X * n, Y * n, Z * n); }
	V3D operator / (long n) const { return V3D(X / n, Y / n, Z / n); }
	bool operator == (V3D& p) const { return X == p.X && Y == p.Y && Z == p.Z; }
	bool operator != (V3D& p) const { return X != p.X || Y != p.Y || Z != p.Z; }
	V3D& operator += (const V3D& x) { return *this = *this + x; }
	V3D& operator -= (const V3D& x) { return *this = *this - x; }
};

class V2D : public Common::Point {
	double trunc(double d) { return (d > 0) ? floor(d) : ceil(d); }
	double round(double number) { return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5); }
public:
	V2D& operator = (const V3D& p3) {
		double m = Eye.Z / (p3.Z - Eye.Z);
		x = round((Eye.X + (Eye.X - p3.X) * m));
		y = round((Eye.Y + (Eye.Y - p3.Y) * m));
		return *this;
	}
	V2D(void) { }
	V2D(const V3D& p3) { *this = p3; }
	V2D(int x, int y) : Common::Point(x, y) { }
	static V3D Eye;
	static void SetEye(const V3D &e) { Eye = e; }
	static void SetEye(const V2D& e2, int z = -SCR_WID_) {
		Eye.X = e2.x; Eye.Y = e2.y; Eye.Z = z;
	}
	static void SetEye(const char *s) {
		char *tempStr;
		strcpy(tempStr, s);
		Eye.X = atoi(EncryptedStream::token(tempStr));
		Eye.Y = atoi(EncryptedStream::token(tempStr));
		Eye.Z = atoi(EncryptedStream::token(tempStr));
	}
	bool operator <  (const V2D& p) const { return (x <  p.x) && (y <  p.y); }
	bool operator <= (const V2D& p) const { return (x <= p.x) && (y <= p.y); }
	bool operator >(const V2D& p) const { return (x >  p.x) && (y >  p.y); }
	bool operator >= (const V2D& p) const { return (x >= p.x) && (y >= p.y); }
	V2D operator + (const V2D& p) const { return V2D(x + p.x, y + p.y); }
	V2D operator - (const V2D& p) const { return V2D(x - p.x, y - p.y); }
	uint16 Area(void) { return x * y; }
	bool Limited(const V2D& p) {
		return (uint16(x) < uint16(p.x)) && (uint16(y) < uint16(p.y));
	}
	V2D Scale(int z) {
		double m = Eye.Z / (Eye.Z - z);
		return V2D(trunc(m * x), trunc(m * y));
	}
};

} // End of namespace CGE2

#endif // CGE2_GENERAL_H
