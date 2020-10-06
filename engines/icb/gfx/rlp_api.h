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

#ifndef ICB_RLP_API_H
#define ICB_RLP_API_H

namespace ICB {

#define RLP_PROP_NAME_LENGTH 32
#define RLP_SHADE_NAME_LENGTH 8
#define RLP_LAMP_NAME_LENGTH 16

#define MAX_NUMBER_SHADE_STATES 128

#define RLP_API_SCHEMA 10
#define RLP_API_ID "RLP"

#define EDGE_ONE 0x1
#define EDGE_TWO 0x2
#define EDGE_THREE 0x4
#define EDGE_FOUR 0x8
#define EDGE_ALL_VISIBLE (EDGE_ONE | EDGE_TWO | EDGE_THREE | EDGE_FOUR)

// schema 2
// ========
// if (anu==0) then ans = ane = 0
// if (afu==0) then afs=afe=ATTEN_MAX_DISTANCE

// schema 3
// ========
// ambient scaled down from 0-4096 to 0-255

// schema 4
// ========
// m: multiplier scaled down from 0-4096 to 0-128
// illegal beam_angles OR illegal beam_softness
//    disables beam_enable + WARNING should be an error,
//    but then need artist to fix the model(s)
//    An error will be enabled after big demo to Sony
// beam_angle & beam_softness converted to cos's of half angle
// ans, ane, afs, afe : pre-multiplied to be square distances

// schema 5
// ========
// added ID "RPL" to the strucure

// schema 6
// ========
// reduced ATTEN_MAX_DISTANCE to prevent integer overflow from (128*1024)->(128*128)

// schema 7
// ========
// supported the bs & ba parameters for DIRECT lights

// Lights are ignored beyond this distance
// Note, the square gets stored, so make it (1<<31)/2
#define ATTEN_MAX_DISTANCE (1 << 15)

// 180 degrees is the maximum size for any kind of beam angles
#define MAX_BEAM_ANGLE (4096 / 2)

// schema 9
// ========
// added the decay parameter
typedef struct PSXrgb { uint16 r, g, b, v; } PSXrgb;

#define OMNI_LIGHT (0)
#define SPOT_LIGHT (1)
#define DIRECT_LIGHT (2)

#define DECAY_NONE (0)
#define DECAY_INV (1)
#define DECAY_INV_SQR (2)

// Trial and error has shown the 3DS MAX decay law constant is this
#define RLP_DECAY_CONSTANT (50)
// The decay law is:
// DECAY_NONE: m = 1.0;
// DECAY_INV: m = ( 1.0/ (distance/factor) )
// DECAY_INV_SQR: m = ( 1.0/ (distance/factor)^2 )

typedef struct PSXLampState {
	PSXrgb c;    // colour not pre-multiplied
	VECTOR pos;  // world position of the lamp
	int16 vx;    // x-direction
	int16 vy;    // y-direction
	int16 vz;    // z-direction
	uint16 m;    // fixed-point int 128 = 1.0 (0=off)
	uint32 ans2; // atten_near_start^2
	uint32 ane2; // atten_near_end^2
	uint32 afs2; // atten_far_start^2
	uint32 afe2; // atten_far_end^2
} PSXLampState;

typedef struct PSXLamp {
	uint8 nStates;
	uint8 type; // light_type (0=OMNI,1=SPOT,2=DIRECT)
	uint8 anu;  // atten_near_use
	uint8 afu;  // atten_far_use

	int32 ba; // cos(beam_angle/2) fixed-point int 4096 = 1.0
	int32 bs; // cos(beam_softness/2) fixed-point int 4096 = 1.0

	uint32 decay;                         // decay parameter (0=none,1=inverse,2=inverse-squared)
	uint16 w;                             // width in cm
	uint16 b;                             // bounce factor : fixed-point int 128 = 1.0 (0=off)
	char lamp_name[RLP_LAMP_NAME_LENGTH]; // for debugging purposes will go in final version
	char prop_name[RLP_PROP_NAME_LENGTH];
	PSXLampState states[1]; // actually PSXLampState states[nStates];
} PSXLamp;

// A triangle
typedef struct ShadeTriangle {
	uint32 vEdge; // bit-flag saying if edge 0,1,2 are visible
	SVECTOR l01;  // line vertex 0 -> vertex 1
	SVECTOR l12;  // line vertex 1 -> vertex 2
	SVECTOR l20;  // line vertex 2 -> vertex 0
	SVECTOR n01;  // normal to plane & line 01
	SVECTOR n12;  // normal to plane & line 12
	SVECTOR n20;  // normal to plane & line 20
	SVECTOR pn;   // plane normal
	int d;        // plane distance
	int n01dots0; // n01 . vertex 0
	int n12dots1; // n12 . vertex 1
	int n20dots2; // n20 . vertex 2
} ShadeTriangle;

// A quad
typedef struct ShadeQuad {
	uint32 vEdge; // bit-flag saying if edge 0,1,2,3 are visible
	SVECTOR l01;  // line vertex 0 -> vertex 1
	SVECTOR l12;  // line vertex 1 -> vertex 2
	SVECTOR l23;  // line vertex 2 -> vertex 3
	SVECTOR l30;  // line vertex 3 -> vertex 0
	SVECTOR n01;  // normal to plane & line 01
	SVECTOR n12;  // normal to plane & line 12
	SVECTOR n23;  // normal to plane & line 23
	SVECTOR n30;  // normal to plane & line 30
	SVECTOR pn;   // plane normal
	int d;        // plane distance
	int n01dots0; // n01 . vertex 0
	int n12dots1; // n12 . vertex 1
	int n23dots2; // n23 . vertex 2
	int n30dots3; // n30 . vertex 3
} ShadeQuad;

typedef struct PSXShade {
	char shade_name[RLP_SHADE_NAME_LENGTH];
	char prop_name[RLP_PROP_NAME_LENGTH];
	uint32 nStates;
	ShadeQuad states[1]; // ShadeQuad states[nStates]
} PSXShade;

// For use with pre-processing converter
typedef struct ShadeInfo {
	char shade_name[RLP_SHADE_NAME_LENGTH];
	char prop_name[RLP_PROP_NAME_LENGTH];
	uint32 nStates;
	ShadeQuad states[MAX_NUMBER_SHADE_STATES];
} ShadeInfo;

typedef struct rlp_API {
	char id[4];
	uint32 schema;
	uint8 nLamps;
	uint8 nShades;
	uint16 shadeOffsetsOffset;
	PSXrgb ambient;
	// Byte offsets
	uint32 lampOffsets[1]; // actually lampOffsets[nLamps]
	inline PSXLamp *GetLamp(const int l);
	inline uint32 *GetShadeOffsets(void);
	inline PSXShade *GetShade(const int s);
} rlp_API;

inline PSXLamp *rlp_API::GetLamp(const int l) { return (PSXLamp *)(id + lampOffsets[l]); }

inline uint32 *rlp_API::GetShadeOffsets(void) { return (uint32 *)(id + shadeOffsetsOffset); }

inline PSXShade *rlp_API::GetShade(const int s) {
	uint32 *shadeOffsets = GetShadeOffsets();
	return (PSXShade *)(id + shadeOffsets[s]);
}

typedef struct rlp_API_9 {
	char id[4];
	uint32 schema;
	uint32 nLamps;
	PSXrgb ambient;
	// Byte offsets
	uint32 lampOffsets[1]; // actually lampOffsets[nLamps]
	                       // PSXLamp lamps[nLamps]
} rlp_API_9;

// Convert from API schema 9 to API schema 10
inline void ConvertRLP(rlp_API *rlp) {
	if (rlp->schema == RLP_API_SCHEMA)
		return;
}

} // End of namespace ICB

#endif // #ifndef RLP_API_H
