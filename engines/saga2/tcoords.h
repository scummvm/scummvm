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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TCOORDS_H
#define SAGA2_TCOORDS_H

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

/* ============================================================================ *
   A class which defines a position on a tile map (including height)
 * ============================================================================ */

class TilePoint {
public:
		//	When an object is within a World, the u & v represent
		//	the U and V coordinates on the tilemap, and the Z
		//	represents the height.
		//
		//	When an object is contained within a container, the
		//	u and v represent where the object is displayed within
		//	the container's rectangle. (Z is unused for now?)
	int16		u,v,z;

		// constructors
	TilePoint(){}
	TilePoint( int16 nu, int16 nv, int16 nz ) { u = nu; v = nv; z = nz; }

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

	int16 quickHDistance( void )
	{
		int16		au = (int16) abs( u ),
					av = (int16) abs( v );

		if (au > av) return (int16)(au + (av >> 1));
		else return (int16)(av + (au >> 1));
	}

	int16 quickDir( void );

	int16 magnitude( void );
};

/* ============================================================================ *
   Constants
 * ============================================================================ */

	//	Size of a tile in ( U, V ) coords

const int			tileUVSize = 16,
					tileUVShift = 4,
					tileZSize = 8,
					tileZShift = 3,
					tileUVMask = (tileUVSize - 1);

	//	Size of a map sector (4 metatiles x 4 metatiles)

const int			sectorSize = tileUVSize * 8 * 4,
					sectorShift = tileUVShift + 3 + 2,
					sectorMask = sectorSize - 1;

	//	A TilePoint defining a NULL location
const extern TilePoint	Nowhere;

/* ============================================================================ *
   TileRegion: Specifies a rectangular region of tiles using min/max
 * ============================================================================ */

struct TileRegion {
	TilePoint	min,
				max;
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
		int32		au = abs( u ),
					av = abs( v );

		if (au > av) return au + (av >> 1);
		else return av + (au >> 1);
	}
};

} // end of namespace Saga2

#endif
