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

#ifndef SAGA2_TILEVECT_H
#define SAGA2_TILEVECT_H

namespace Saga2 {

// This routine returns a random vector between two bounding vectors

inline TilePoint randomVector(TilePoint minv, TilePoint maxv) {
	return TilePoint(
	           RANDOM(minv.u, maxv.u),
	           RANDOM(minv.v, maxv.v),
	           RANDOM(minv.z, maxv.z));
}

// Routine to get an orthogonal vector in the uv plane to an existing vector

inline TilePoint rightVector(TilePoint tp, bool which = 0) {
	return TilePoint(
	           tp.v * (which ? 1 : -1),
	           tp.u * (which ? -1 : 1),
	           0);
}

// Routine to force the magnitude of a vector to a value

inline void setMagnitude(TilePoint &tp, int32 newMag) {
#if DEBUG
	assert(tp.magnitude());
#else
	if (tp.magnitude() == 0)
		tp = TilePoint(1, 1, 0);
#endif
	//if ( tp.magnitude() )
	//{
	int32 nu = (tp.u * newMag) / tp.magnitude();
	int32 nv = (tp.v * newMag) / tp.magnitude();
	tp = TilePoint(nu, nv, tp.z);
	//}
}

// returns an arbitrary 'side' designation for a line and a point
//   the only determination that can be made from this value is that
//   any other points returning the same value for the same line will
//   be on the same 'side' of the line as the first
// note that this is flat in uv-space (no z)


inline bool sideOf(const TilePoint &p1,     // point 1 used to define the line
                   const TilePoint &p2,           // point 2 used to define the line
                   const TilePoint &pp) {         // the point to get the 'side' of
	int32 m;
	if (p1.u - p2.u == 0)
		return pp.u > p1.u;
	m = (p1.v - p2.v) / (p1.u - p2.u);
	return m * pp.u + p1.v > m * p1.u + pp.v;
}

// determines whether or not two points are on the same side of a
//   line


inline bool sameSide(const TilePoint &p1,   // point 1 used to define the line
                     const TilePoint &p2,         // point 2 used to define the line
                     const TilePoint &pa,         // the first point (to check)
                     const TilePoint &pb) {       // the second point (to check against)
	return sideOf(p1, p2, pa) == sideOf(p1, p2, pb);
}

// Get the lowest u,v, and z in either of 2,3 or 4 points

inline TilePoint MinTilePoint(
    const TilePoint &c1,
    const TilePoint &c2) {
	return TilePoint(MIN(c1.u, c2.u), MIN(c1.v, c2.v), MIN(c1.z, c2.z));
}

inline TilePoint MinTilePoint(const TilePoint &c1, const TilePoint &c2, const TilePoint &c3) {
	return MinTilePoint(MinTilePoint(c1, c2), c3);
}

inline TilePoint MinTilePoint(const TilePoint &c1, const TilePoint &c2,
                              const TilePoint &c3, const TilePoint &c4) {
	return MinTilePoint(MinTilePoint(c1, c2), MinTilePoint(c3, c4));
}

// Get the highest u,v, and z in either of 2,3 or 4 points

inline TilePoint MaxTilePoint(
    const TilePoint &c1,
    const TilePoint &c2) {
	return TilePoint(MAX(c1.u, c2.u), MAX(c1.v, c2.v), MAX(c1.z, c2.z));
}

inline TilePoint MaxTilePoint(const TilePoint &c1, const TilePoint &c2, const TilePoint &c3) {
	return MaxTilePoint(MaxTilePoint(c1, c2), c3);
}

inline TilePoint MaxTilePoint(const TilePoint &c1, const TilePoint &c2,
                              const TilePoint &c3, const TilePoint &c4) {
	return MaxTilePoint(MaxTilePoint(c1, c2), MaxTilePoint(c3, c4));
}

} // end of namespace Saga2

#endif
