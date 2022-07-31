/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_ROUTE_BARRIERS_H_INCLUDED
#define ICB_PX_ROUTE_BARRIERS_H_INCLUDED

// Include headers needed by this file.

#include "engines/icb/common/px_common.h"

namespace ICB {

// These define the filenames for files containing barrier maps and routing maps.
#define PX_FILENAME_LINEOFSIGHT "pxwglineofsight"
#define PX_FILENAME_ROUTING "pxwgrouting"
#define PX_FILENAME_BARRIERLIST "pxwgbarrierlist"

#ifndef PC_EXT_LINKED
#define PC_EXT_LINKED "linked"
#endif

#ifndef PSX_EXT_LINKED
#define PSX_EXT_LINKED "PSXlinked"
#endif

#ifdef PX_EXT_LINKED
#undef PX_EXT_LINKED
#endif

#define PX_EXT_LINKED PC_EXT_LINKED

// This is the version for these files.  The engine checks this runtime to know that it is running with
// the correct version of file.
#define VERSION_PXWGLINEOFSIGHT 200
#define VERSION_PXWGROUTING 200
#define VERSION_PXWGBARRIERLIST 200

// This is the size of the sides of the cubes that each floor is divided into in centimetres.
#define FLOOR_CUBE_SIZE 1000   // 10-metre sides.
#define ABOVE_ALL_MODELS 10000 // Set this to be higher than any model point ever.

// This is an enumerated type for the barrier (the types listed are just illustrations - they may well be changed).
//  BRICK               -   can't walk through it, see through it or shoot through it.
//  GLASS               -   can't walk through it; can see through it; not sure about shooting (glass would need to break).
//  BULLET_PROOF_GLASS  -   can't walk through it or shoot through it, but can see through it.
//  THIN_STEEL          -   can't see through it or walk through it, but can shoot through it.
//  WIRE_FENCE          -   can't walk through it, but can see through it; can shoot through it with random success.
//  UNIT_HEIGHT         -   special one for stopping characters walking off the edge of ledges etc.
//  VIEW_FIELD          -   stops characters walking out of camera field-of-view.
//  LEFT_NUDGE          -   use to assist player control going through doors.
//  RIGHT_NUDGE         -   ditto last one.
enum eBarrierType { BRICK = 0, GLASS, BULLET_PROOF_GLASS, THIN_STEEL, WIRE_FENCE, UNIT_HEIGHT, VIEW_FIELD, LEFT_NUDGE, RIGHT_NUDGE };

#define BARRIER_TYPE_CARDINALITY 9 // Must match number of enums in previous type (because C++
// doesn't provide a way to get this).

// This is an enumerated type for the things that might try to pass through a barrier.  Note: the TEST_RAY
// is blocked by all types of barrier.
enum eBarrierRayType { TEST_RAY, LIGHT, BULLET };

#define RAY_TYPE_CARDINALITY 3

// Defines a multi-state logic value for use with the barriers.
enum eBarrierLogicValue { NO_IMPACT = 0, BLOCKS, ALLOWS, MAYBE, SPECIAL };

// This is the truth table that states what kind of ray passes through what
// type of barrier.
static enum eBarrierLogicValue barrierLogicTable[BARRIER_TYPE_CARDINALITY][RAY_TYPE_CARDINALITY] = {
	{BLOCKS, BLOCKS, BLOCKS}, {BLOCKS, ALLOWS, SPECIAL}, {BLOCKS, ALLOWS, BLOCKS}, {BLOCKS, BLOCKS, ALLOWS}, {BLOCKS, ALLOWS, MAYBE},
	{BLOCKS, ALLOWS, ALLOWS}, {BLOCKS, ALLOWS, ALLOWS},  {BLOCKS, ALLOWS, ALLOWS}, {BLOCKS, ALLOWS, ALLOWS}};

typedef struct {
	// these are in both versions
	PXfloat m_linedist, m_alinedist, m_blinedist;

	PXfloat m_lpx, m_lpz;   // Main barrier
	PXfloat m_alpx, m_alpz; // End A.
	PXfloat m_blpx, m_blpz; // End B.
} BarrierCollisionMaths;

class BarrierCollisionMathsObject {
public:
	static inline PXfloat alpx(BarrierCollisionMaths *bmath) {
		// return m_alpx;
		return -bmath->m_lpz;
	}

	static inline PXfloat alpz(BarrierCollisionMaths *bmath) {
		// return m_alpz;
		return bmath->m_lpx;
	}

	static inline PXfloat blpx(BarrierCollisionMaths *bmath) {
		// return m_blpx;
		return bmath->m_lpz;
	}

	static inline PXfloat blpz(BarrierCollisionMaths *bmath) {
		// return m_blpz;
		return -bmath->m_lpx;
	}

	static void Generate(BarrierCollisionMaths *bmath, PXreal x1, PXreal z1, PXreal x2, PXreal z2) {
		PXreal dx = x1 - x2;
		PXreal dz = z1 - z2;

		int32 nLength = (int32)PXsqrt((PXdouble)(dx * dx + dz * dz));

		PXfloat xunit = PXreal2PXfloat(dx) / nLength;
		PXfloat zunit = PXreal2PXfloat(dz) / nLength;

		bmath->m_lpx = -zunit;
		bmath->m_lpz = xunit;

		bmath->m_linedist = (x1 * bmath->m_lpx) + (z1 * bmath->m_lpz);

		bmath->m_alinedist = (x1 * alpx(bmath)) + (z1 * alpz(bmath));

		bmath->m_blinedist = (x2 * blpx(bmath)) + (z2 * blpz(bmath));
	}
};

typedef struct {
	PXreal m_x1, m_z1;        // Looking down on the model, the position of the first vertical edge of the barrier.
	PXreal m_x2, m_z2;        // Looking down on the model, the position of the second vertical edge.
	PXreal m_bottom;          // The bottom of the barrier.
	PXreal m_top;             // The top of the barrier.
	eBarrierType m_material;  // The material the barrier is made of.
	PXfloat m_pan;            // The barrier's pan value.
	BarrierCollisionMaths m_bcm; // Some extra figures to speed up barrier collision detection.
} RouteBarrier;

inline void routeBarrierCreatePan(RouteBarrier *barrier) { barrier->m_pan = PXAngleOfVector(barrier->m_z1 - barrier->m_z2, barrier->m_x1 - barrier->m_x2); }

// This holds several barriers.  These barriers all at least partly occupy a given cube in space.  If one barrier passes
// through more than one cube, it will have a duplicate entry in each cube.
typedef struct {
	int32 num_barriers; // The number of barriers referenced in this cube.
	uint32 barriers;    // Offset to an array of barrier indices.
} BarrierCube;

// This is a horizontal slice through the Max model, containing all the route barriers that pass through this level.  The
// extremeties of the whole cuboid are given first so that a quick initial check can be done to see if there might be
// route barriers in the way.
typedef struct {
	PXreal bottom;          // The bottom of the slice.
	PXreal top;             // The top of the slice.
	PXreal left_edge;       // Leftmost edge of the cube of space occupied by this floor slice.
	PXreal right_edge;      // Ditto right edge.
	PXreal back_edge;       // Back edge.
	PXreal front_edge;      // Ditto front edge.
	uint32 num_cubes;       // Number of _route_cubes in this floor (could be calculated by dividing overall cube size by FLOOR_CUBE_SIZE).
	uint32 row_length;      // Size of the rows in the array (eg. 6 cubes could be 1X6, 2X3, 3X2 or 6X1).
	uint32 offset_cubes[1]; // An array of offsets to cubes (2D array of size row_length * (num_cubes / row_length) ).
} BarrierSlice;

// This is used in the following definition of _parent_box, and holds one group of barriers.
typedef struct {
	PXreal back, left;   // Back/left of the bounding box holding this group of barriers (looking down into the model).
	PXreal front, right; // Ditto front/right.
	uint32 num_barriers; // Number of barriers in this group.
	uint32 barriers[1];  // Array of barrier indices.

} ChildGroup;

// This holds one parent box entry.
typedef struct {
	PXreal back, left;      // Top/left of the parent box (looking down into the model).
	PXreal front, right;    // Ditto bottom/right.
	uint32 num_barriers;    // Number of barriers in the parent (not its children).
	uint32 barriers;        // Offset to an array of barrier indices.
	uint32 num_specials;    // Number of special barriers (eg. field-of-view).
	uint32 specials;        // Offset of the array of special barrier indices.
	uint32 num_childgroups; // Number of child groups owned by this parent box.
	uint32 childgroups[1];  // Array of offsets to the child groups.

} ParentBox;

// This is also a slice through the model, but the data is grouped in a different way which is more suited to routing.
typedef struct {
	PXreal bottom;           // The bottom of the slice.
	PXreal top;              // The top of the slice.
	uint32 num_parent_boxes; // The number of parent boxes in this slice (same as the number of floor rectangles at this height).
	uint32 parent_boxes[1];  // An array of offsets to parent boxes.

} RoutingSlice;

inline eBarrierLogicValue IsBarrierTo(eBarrierType eMaterial, eBarrierRayType eRay) { return barrierLogicTable[eMaterial][eRay]; }

} // End of namespace ICB

#endif // #ifndef _PX_ROUTE_BARRIERS_H_INCLUDED
