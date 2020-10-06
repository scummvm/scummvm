/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
enum _barrier_type { BRICK = 0, GLASS, BULLET_PROOF_GLASS, THIN_STEEL, WIRE_FENCE, UNIT_HEIGHT, VIEW_FIELD, LEFT_NUDGE, RIGHT_NUDGE };

#define BARRIER_TYPE_CARDINALITY 9 // Must match number of enums in previous type (because C++
// doesn't provide a way to get this).

// This is an enumerated type for the things that might try to pass through a barrier.  Note: the TEST_RAY
// is blocked by all types of barrier.
enum _barrier_ray_type { TEST_RAY, LIGHT, BULLET };

#define RAY_TYPE_CARDINALITY 3

// Defines a multi-state logic value for use with the barriers.
enum _barrier_logic_value { NO_IMPACT = 0, BLOCKS, ALLOWS, MAYBE, SPECIAL };

// This is the truth table that states what kind of ray passes through what
// type of barrier.
static enum _barrier_logic_value _barrier_logic_table[BARRIER_TYPE_CARDINALITY][RAY_TYPE_CARDINALITY] = {
    {BLOCKS, BLOCKS, BLOCKS}, {BLOCKS, ALLOWS, SPECIAL}, {BLOCKS, ALLOWS, BLOCKS}, {BLOCKS, BLOCKS, ALLOWS}, {BLOCKS, ALLOWS, MAYBE},
    {BLOCKS, ALLOWS, ALLOWS}, {BLOCKS, ALLOWS, ALLOWS},  {BLOCKS, ALLOWS, ALLOWS}, {BLOCKS, ALLOWS, ALLOWS}};

// simple, this is just for the converters
// Some extra figures to speed up barrier collision detection.
typedef struct {
	PXfloat linedist, alinedist, blinedist;
	PXfloat lpx, lpz;   // Main barrier
	PXfloat alpx, alpz; // End A.
	PXfloat blpx, blpz; // End B.
} _simple_barrier_collision_maths;

// simple, this is just for the converters
// This holds one single barrier.
typedef struct {
	PXreal x1, z1;                       // Looking down on the model, the position of the first vertical edge of the barrier.
	PXreal x2, z2;                       // Looking down on the model, the position of the second vertical edge.
	PXreal bottom;                       // The bottom of the barrier.
	PXreal top;                          // The top of the barrier.
	_barrier_type material;              // The material the barrier is made of.
	PXfloat pan;                         // The barrier's pan value.
	_simple_barrier_collision_maths bcm; // Some extra figures to speed up barrier collision detection.
} _simple_route_barrier;

class _barrier_collision_maths {
private:
	// these are in both versions
	PXfloat m_linedist, m_alinedist, m_blinedist;

	PXfloat m_lpx, m_lpz;   // Main barrier
	PXfloat m_alpx, m_alpz; // End A.
	PXfloat m_blpx, m_blpz; // End B.

public:
	inline PXfloat linedist() { return m_linedist; }

	inline PXfloat alinedist() { return m_alinedist; }
	inline PXfloat blinedist() { return m_blinedist; }

	// on pc these are pxfloats

	inline PXfloat lpx() { return m_lpx; }
	inline PXfloat lpz() { return m_lpz; }

	inline PXfloat alpx() {
		// return m_alpx;
		return -lpz();
	}

	inline PXfloat alpz() {
		// return m_alpz;
		return lpx();
	}

	inline PXfloat blpx() {
		// return m_blpx;
		return lpz();
	}

	inline PXfloat blpz() {
		// return m_blpz;
		return -lpx();
	}

	void Generate(PXreal x1, PXreal z1, PXreal x2, PXreal z2) {
		PXreal dx = x1 - x2;
		PXreal dz = z1 - z2;

		int32 nLength = (int32)PXsqrt((PXdouble)(dx * dx + dz * dz));

		PXfloat xunit = PXreal2PXfloat(dx) / nLength;
		PXfloat zunit = PXreal2PXfloat(dz) / nLength;

		m_lpx = -zunit;
		m_lpz = xunit;

		m_linedist = (x1 * lpx()) + (z1 * lpz());

		m_alinedist = (x1 * alpx()) + (z1 * alpz());

		m_blinedist = (x2 * blpx()) + (z2 * blpz());
	}

	_barrier_collision_maths() {
		(void)m_alpx; // shutup warning
		(void)m_alpz; // shutup warning
		(void)m_blpx; // shutup warning
		(void)m_blpz; // shutup warning
	}
};

class _route_barrier {

	PXreal m_x1, m_z1;        // Looking down on the model, the position of the first vertical edge of the barrier.
	PXreal m_x2, m_z2;        // Looking down on the model, the position of the second vertical edge.
	PXreal m_bottom;          // The bottom of the barrier.
	PXreal m_top;             // The top of the barrier.
	_barrier_type m_material; // The material the barrier is made of.
	PXfloat m_pan;            // The barrier's pan value.
	_barrier_collision_maths m_bcm; // Some extra figures to speed up barrier collision detection.

public:

	void Create_pan() { m_pan = PXAngleOfVector(m_z1 - m_z2, m_x1 - m_x2); }

	void x1(PXreal x) { m_x1 = x; }
	void z1(PXreal z) { m_z1 = z; }

	inline PXreal x1() const { return m_x1; }
	inline PXreal z1() const { return m_z1; }

	void x2(PXreal x) { m_x2 = x; }
	void z2(PXreal z) { m_z2 = z; }
	inline PXreal x2() const { return m_x2; }
	inline PXreal z2() const { return m_z2; }
	inline PXreal bottom() const { return m_bottom; }
	inline PXreal top() const { return m_top; }
	inline _barrier_type material() const { return m_material; }

	inline PXfloat pan() const { return m_pan; }

	inline _barrier_collision_maths &bcm() { return m_bcm; }

	_route_barrier() {}

	_route_barrier(PXreal inX1, PXreal inZ1, PXreal inX2, PXreal inZ2, PXreal inBottom, PXreal inTop, _barrier_type inMaterial) {
		m_x1 = inX1;
		m_z1 = inZ1;
		m_x2 = inX2;
		m_z2 = inZ2;
		m_bottom = inBottom;
		m_top = inTop;
		m_material = inMaterial;
	}
};

// This holds several barriers.  These barriers all at least partly occupy a given cube in space.  If one barrier passes
// through more than one cube, it will have a duplicate entry in each cube.
typedef struct {
	int32 num_barriers; // The number of barriers referenced in this cube.
	uint32 barriers;    // Offset to an array of barrier indices.
} _barrier_cube;

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
} _barrier_slice;

// This is used in the following definition of _parent_box, and holds one group of barriers.
typedef struct {
	PXreal back, left;   // Back/left of the bounding box holding this group of barriers (looking down into the model).
	PXreal front, right; // Ditto front/right.
	uint32 num_barriers; // Number of barriers in this group.
	uint32 barriers[1];  // Array of barrier indices.

} _child_group;

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

} _parent_box;

// This is also a slice through the model, but the data is grouped in a different way which is more suited to routing.
typedef struct {
	PXreal bottom;           // The bottom of the slice.
	PXreal top;              // The top of the slice.
	uint32 num_parent_boxes; // The number of parent boxes in this slice (same as the number of floor rectangles at this height).
	uint32 parent_boxes[1];  // An array of offsets to parent boxes.

} _routing_slice;

__inline _barrier_logic_value IsBarrierTo(_barrier_type eMaterial, _barrier_ray_type eRay) { return _barrier_logic_table[eMaterial][eRay]; }

} // End of namespace ICB

#endif // #ifndef _PX_ROUTE_BARRIERS_H_INCLUDED
