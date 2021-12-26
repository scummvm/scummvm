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

#ifndef SAGA2_TCOORDS_H
#define SAGA2_TCOORDS_H

#include "common/savefile.h"
#include "common/memstream.h"

namespace Saga2 {

enum facingDirections {
	dirUp = 0,
	dirUpLeft,
	dirLeft,
	dirDownLeft,
	dirDown,
	dirDownRight,
	dirRight,
	dirUpRight,

	dirInvalid
};
typedef uint8 Direction;

struct StaticTilePoint {
	int16 u, v, z;

	void set(int nu, int nv, int nz) {
		u = nu;
		v = nv;
		z = nz;
	}

	friend StaticTilePoint operator+(StaticTilePoint a, StaticTilePoint b) {
		int16 nu = a.u + b.u;
		int16 nv = a.v + b.v;
		int16 nz = a.z + b.z;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend StaticTilePoint operator-(StaticTilePoint a, StaticTilePoint b) {
		int16 nu = a.u - b.u;
		int16 nv = a.v - b.v;
		int16 nz = a.z - b.z;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend StaticTilePoint operator*(StaticTilePoint a, int b) {
		int16 nu = a.u * b;
		int16 nv = a.v * b;
		int16 nz = a.z * b;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend StaticTilePoint operator/(StaticTilePoint a, int b) {
		int16 nu = a.u / b;
		int16 nv = a.v / b;
		int16 nz = a.z / b;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend StaticTilePoint operator<<(StaticTilePoint a, int b) {
		int16 nu = a.u << b;
		int16 nv = a.v << b;
		int16 nz = a.z << b;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend StaticTilePoint operator>>(StaticTilePoint a, int b) {
		int16 nu = a.u >> b;
		int16 nv = a.v >> b;
		int16 nz = a.z >> b;
		StaticTilePoint p = {nu, nv, nz};

		return p;
	}

	friend int operator==(StaticTilePoint a, StaticTilePoint b) {
		return a.u == b.u && a.v == b.v && a.z == b.z;
	}

	friend int operator!=(StaticTilePoint a, StaticTilePoint b) {
		return a.u != b.u || a.v != b.v || a.z != b.z;
	}

	void operator+=(StaticTilePoint a) {
		u += a.u;
		v += a.v;
		z += a.z;
	}
};

#include "common/pack-start.h"
struct TilePoint {
		//	When an object is within a World, the u & v represent
		//	the U and V coordinates on the tilemap, and the Z
		//	represents the height.
		//
		//	When an object is contained within a container, the
		//	u and v represent where the object is displayed within
		//	the container's rectangle. (Z is unused for now?)
	int16		u,v,z;

		// constructors
	TilePoint() { u = v = z = 0; }
	TilePoint(int16 nu, int16 nv, int16 nz) { u = nu; v = nv; z = nz; }
	TilePoint(Common::SeekableReadStream *stream);
	TilePoint(StaticTilePoint p) {
		u = p.u;
		v = p.v;
		z = p.z;
	}

	void load(Common::SeekableReadStream *stream) {
		u = stream->readSint16LE();
		v = stream->readSint16LE();
		z = stream->readSint16LE();
	}

	void write(Common::MemoryWriteStreamDynamic *out) const {
		out->writeSint16LE(u);
		out->writeSint16LE(v);
		out->writeSint16LE(z);
	}

		// TilePoint operators
	friend TilePoint operator+ (TilePoint a, TilePoint b)
		{ return TilePoint( (int16) (a.u + b.u), (int16) (a.v + b.v), (int16) (a.z + b.z) ); }

	friend TilePoint operator- (TilePoint a)
		{ return TilePoint( (int16) (-a.u), (int16) (-a.v), (int16) (-a.z) ); }

	friend TilePoint operator- (TilePoint a, TilePoint b)
		{ return TilePoint( (int16) (a.u - b.u), (int16) (a.v - b.v), (int16) (a.z - b.z) ); }

	friend TilePoint operator* (TilePoint a, int b)
		{ return TilePoint( (int16) (a.u * (int16)b), (int16) (a.v * (int16)b), (int16) (a.z * (int16)b) ); }

	friend TilePoint operator/ (TilePoint a, int b)
		{ return TilePoint( (int16) (a.u / (int16)b), (int16) (a.v / (int16)b), (int16) (a.z / (int16)b) ); }

	friend TilePoint operator>>(TilePoint a, int b)
		{ return TilePoint( (int16) (a.u >> (int16)b), (int16) (a.v >> (int16)b), (int16) (a.z >> (int16)b) ); }

	friend TilePoint operator<<(TilePoint a, int b)
		{ return TilePoint( (int16) (a.u << (int16)b), (int16) (a.v << (int16)b), (int16) (a.z << (int16)b) ); }

	friend int     operator==(TilePoint a, TilePoint b)
		{ return a.u == b.u && a.v == b.v && a.z == b.z; }

	friend int     operator!=(TilePoint a, TilePoint b)
		{ return a.u != b.u || a.v != b.v || a.z != b.z; }

	void operator+= (TilePoint a) { u += a.u; v += a.v; z += a.z; }
	void operator-= (TilePoint a) { u -= a.u; v -= a.v; z -= a.z; }

	int16 quickHDistance() {
		int16		au = (int16)ABS(u),
					av = (int16)ABS(v);

		if (au > av)
			return (int16)(au + (av >> 1));
		else
			return (int16)(av + (au >> 1));
	}

	int16 quickDir();

	int16 magnitude();

	void debugPrint(int level = 0, const char *msg = "TilePoint:") {
		debug(level, "%s %d, %d, %d", msg, u, v, z);
	}
} PACKED_STRUCT;
#include "common/pack-end.h"


/* ============================================================================ *
   Constants
 * ============================================================================ */

	//	A TilePoint defining a NULL location
const extern StaticTilePoint Nowhere;

/* ============================================================================ *
   TileRegion: Specifies a rectangular region of tiles using min/max
 * ============================================================================ */

struct TileRegion {
	TilePoint	min,
				max;

	void read(Common::InSaveFile *in) {
		min.load(in);
		max.load(in);
	}

	void write(Common::MemoryWriteStreamDynamic *out) {
		min.write(out);
		max.write(out);
	}
};

/* ============================================================================ *
   TilePoint32
 * ============================================================================ */

	//	int32 tilepoint for more precise calculations

class TilePoint32 {
public:
		//	When an object is within a World, the u & v represent
		//	the U and V coordinates on the tilemap, and the Z
		//	represents the height.
		//
		//	When an object is contained within a container, the
		//	u and v represent where the object is displayed within
		//	the container's rectangle. (Z is unused for now?)
	int32		u,v,z;

		// constructors
	TilePoint32(){}
	TilePoint32( int32 nu, int32 nv, int32 nz ) { u = nu; v = nv; z = nz; }
	TilePoint32( TilePoint &tp ) { u = tp.u; v = tp.v; z = tp.z; }

		// TilePoint32 operators
	friend TilePoint32 operator+ (TilePoint32 a, TilePoint32 b)
		{ return TilePoint32( a.u + b.u, a.v + b.v, a.z + b.z ); }

	friend TilePoint32 operator- (TilePoint32 a)
		{ return TilePoint32( -a.u, -a.v, -a.z ); }

	friend TilePoint32 operator- (TilePoint32 a, TilePoint32 b)
		{ return TilePoint32( a.u - b.u, a.v - b.v, a.z - b.z ); }

	friend TilePoint32 operator* (TilePoint32 a, int b)
		{ return TilePoint32( a.u * (int32)b, a.v * (int32)b, a.z * (int32)b ); }

	friend TilePoint32 operator/ (TilePoint32 a, int b)
		{ return TilePoint32( a.u / (int32)b, a.v / (int32)b, a.z / (int32)b ); }

	friend TilePoint32 operator>>(TilePoint32 a, int b)
		{ return TilePoint32( a.u >> (int32)b, a.v >> (int32)b, a.z >> (int32)b ); }

	friend TilePoint32 operator<<(TilePoint32 a, int b)
		{ return TilePoint32( a.u << (int32)b, a.v << (int32)b, a.z << (int32)b ); }

	friend int     operator==(TilePoint32 a, TilePoint32 b)
		{ return a.u == b.u && a.v == b.v && a.z == b.z; }

	friend int     operator!=(TilePoint32 a, TilePoint32 b)
		{ return a.u != b.u || a.v != b.v || a.z != b.z; }

	void operator+= (TilePoint32 a) { u += a.u; v += a.v; z += a.z; }
	void operator-= (TilePoint32 a) { u -= a.u; v -= a.v; z -= a.z; }

	int32 quickHDistance( void ) {
		int32		au = ABS( u ),
					av = ABS( v );

		if (au > av) return au + (av >> 1);
		else return av + (au >> 1);
	}
};

} // end of namespace Saga2

#endif
