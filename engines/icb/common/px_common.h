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

#ifndef ICB_PX_COMMON_H
#define ICB_PX_COMMON_H

#include "common/scummsys.h"

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

#define PXNULL (0xffffffff)

// This defines the scale value for converting PC floats to PSX fixed-point representation.  Not
// sure if this is the right place for this.
#define PSX_FIXED_POINT_SCALE 4096

#define PSX_FLOAT_POINT_SHIFT 12
#define PSX_FLOAT_POINT_SCALE (1 << PSX_FLOAT_POINT_SHIFT)
#define PSX_ANGLE_POINT_SCALE 4096
#define PSX_DOUBLE_POINT_SCALE 4096

// Rather than using integer values beginning at 0 it would be more descriptive to use
// four ascii characters, so the type can be guessed from a hex dump.
// Use the FT macro to turn four characters into an enum _file_type
#define FT_MACRO(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

enum _file_type {
	FT_UNDEFINED,
	unusedFT_COMPILED_GAME_OBJECT, // A compiled game object
	FT_FONT,                       // debug console & head-up font
	FT_VOX_ANIM,                   // voxel anim file (see voxel anim file format doc)
	FT_GAME_OBJECT,                // A game engine game object
	FT_BITMAP_ANIM,                // 2D anim file (from bitmap converter) (see 2D bitmaps doc)
	FT_BITMAP_FONT,                // font file (from bitmap converter) (see 2D bitmaps doc)
	unusedFT_FN_ROUTINES_DAT,      // fn routine data for the script compiler
	unusedFT_OBJECTS_SCRIPTS_DAT,  // Compiled scripts for a single object
	unusedFT_LINKED_SCRIPTS,       // File containing scripts linked together
	unusedFT_LINKED_OBJECTS,       // File containing objects in session linked together
	FT_PROP_LIST,                  // File containing a list of prop names and there program counters
	FT_PROP_ANIM_LIST,             // File containing prop animation lists
	FT_FLOOR_MAP,                  // File containing the floors for a session [PS 06/04/98].
	FT_BARRIERS,                   // File containing the index into the barriers file for the line-of-sight stuff [PS 06/04/98].
	FT_CAMERAS,                    // File containing the camera 'cubes' for a session [PS 06/04/98].
	FT_BARRIERLIST,                // File containing the actual raw walkgrid barriers for a session [PS 01/06/98].
	FT_OBJECT_POSITIONS,           // File listing props occurring on a given floor [PS 06/04/98].
	FT_PROP_ANIMATIONS,            // File containing information about prop animations [PS 11/08/98].
	FT_VOICE_OVER_TEXT,            // Compiled voice-over text file (for Remora etc.).

	// add more here!

	/***IMPORTANT***
        DO NOT DELETE ENTRIES FROM THIS LIST OR SUBSEQUENT RESOURCE TYPES WILL BE RENUMBERED
        RENAME ENTRIES NO LONGER IN USE AND REUSE THEM LATER
        */

	// The following entries can go in any order, but should not be changed

	FT_COMPILED_SCRIPTS = FT_MACRO('C', 'S', 'C', 'R'),     // Compiled script object format (.scrobj)
	FT_LINKED_SCRIPTS = FT_MACRO('L', 'S', 'C', 'R'),       // File containing scripts linked together
	FT_LINKED_OBJECTS = FT_MACRO('L', 'O', 'B', 'J'),       // File containing objects in session linked together
	FT_COMPILED_GAME_OBJECT = FT_MACRO('C', 'O', 'B', 'J'), // A compiled game object
	FT_FN_ROUTINES_DAT = FT_MACRO('F', 'N', 'D', 'T'),      // fn routine data for the script compiler
	FT_COMBINED_OBJECT = FT_MACRO('C', 'M', 'B', 'O'),      // Combined object and script data
	FT_COMPILED_TEXT = FT_MACRO('C', 'M', 'P', 'T'),        // Compressed text
	FT_LINKED_TEXT = FT_MACRO('L', 'N', 'K', 'T'),          // Linked text

	FT_COMPILED_SFX = FT_MACRO('S', 'F', 'X', ' '), // compiled SFX file
	FT_LINKED_SFX = FT_MACRO('S', 'F', 'X', 'L'),   // linked SFX files file
	FT_REMORA_MAP = FT_MACRO('R', 'M', 'A', 'P')    // Remora map file.

};

#define STANDARD_HEADER_NAME_LENGTH 32 // Max length of the header name

class px_standard_header {
      public:
	int32 version;                          // This is incremented every time the object is updated
	_file_type type;                        // enumerated value for every type of object in the game
	int32 owner;                            // Who is responsible for producing this object
	int32 unused;                           // For future expansion
	int32 unused2;                          // For future expansion
	char name[STANDARD_HEADER_NAME_LENGTH]; // 32 bytes worth of ascii name information

	void SetData(int version, _file_type type, int owner, cstr name);
	_file_type GetType() { return (type); }
	cstr GetName() { return (name); }
	uint GetVersion() { return (version); }
};

typedef struct {
	uint8 red;
	uint8 green;
	uint8 blue;
	uint8 alpha;
} _rgb;

typedef float PXreal;
typedef float PXfloat;
typedef double PXdouble;
#define REAL_ZERO 0.0f
#define REAL_ONE 1.0f
#define REAL_TWO 2.0f
#define REAL_MIN FLT_MIN
#define REAL_MAX FLT_MAX
#define REAL_LARGE 100000.0f

#define FLOAT_ZERO 0.0f
#define FLOAT_QUARTER 0.25f
#define FLOAT_HALF 0.5f
#define FLOAT_ONE 1.0f
#define FLOAT_TWO 2.0f
#define FLOAT_MIN FLT_MIN
#define FLOAT_MAX FLT_MAX
#define FLOAT_LARGE 100000.0f

#define ZERO_TURN 0.0f
#define QUARTER_TURN 0.25f
#define HALF_TURN 0.5f
#define FULL_TURN 1.0f

#define TWO_PI (2.0f * M_PI)

// For converting pan values when the game is saved/loaded
// For PC this is equal to the PSX fixed point scaling used to represent angles
#define PAN_SCALE_FACTOR PSX_ANGLE_POINT_SCALE

// #define DEGREES_TO_RADIANS 0.01745329f
#define DEGREES_TO_RADIANS(x) ((x * TWO_PI) / 360.0f)
#define RADIANS_TO_DEGREES(x) (x * (180.0f / PI))

// How to make a PXdouble from a PXreal
#define PXreal2PXdouble(x) (double)(x)
// How to make a PXreal from a PXdouble
#define PXdouble2PXreal(x) (float)(x)

// How to make a PXfloat from a PXreal
#define PXreal2PXfloat(x) (x)
// How to make a PXreal from a PXfloat
#define PXfloat2PXreal(x) (x)

typedef struct PXsvector_PC {
	float x;
	float y;
	float z;
} PXsvector_PC;

typedef struct PXvector_PC {
	float x;
	float y;
	float z;
} PXvector_PC;

typedef struct PXsvector_PSX {
	short x;
	short y;
	short z;
	short pad;
} PXsvector_PSX;

typedef struct PXvector_PSX {
	int32 x;
	int32 y;
	int32 z;
} PXvector_PSX;

#ifdef _PSX_VECTOR
typedef PXvector_PSX PXvector;
typedef PXsvector_PSX PXsvector;
#else
typedef PXvector_PC PXvector;
typedef PXsvector_PC PXsvector;
#endif

typedef struct PXorient_PSX {
	short pan;
	short tilt;
	short cant;
	short pad;
} PXorient_PSX;

typedef struct PXorient_PC {
	float pan;
	float tilt;
	float cant;
} PXorient_PC;

#ifdef _PSX_ORIENT
typedef PXorient_PSX PXorient;
#else
typedef PXorient_PC PXorient;
#endif

// Endian safe read functions
inline uint16 READ_LE_U16(const void *p) {
	const uint8 *data = (const uint8 *)p;
	return (uint16)((data[1] << 8) | data[0]);
}

inline uint32 READ_LE_U32(const void *p) {
	const uint8 *data = (const uint8 *)p;
	return (uint32)(((uint32)data[3] << 24) | ((uint32)data[2] << 16) | ((uint32)data[1] << 8) | (uint32)data[0]);
}

#define MKTAG(a0, a1, a2, a3) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

} // End of namespace ICB

#endif // #ifndef _PX_INC_PROJECT_X_COMMON_H
