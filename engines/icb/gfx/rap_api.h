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

#ifndef ICB_RAP_API_HH
#define ICB_RAP_API_HH

#include "engines/icb/gfx/psx_pcdefines.h"

namespace ICB {

#define RAP_API_SCHEMA 10

#define RAP_API_ID "RAP"

// A bone-bone link
// tx, ty, tz : is translation
// parent : is the the parent bone ID
// = nBones means no linkage
typedef struct BoneLink {
	int16 tx, ty, tz;
	uint16 parent;
} BoneLink;

// A simple vertex description
typedef struct Vertex {
	int16 vx, vy, vz;
	uint16 vertId;
} Vertex;

// A vertex-bone link
typedef struct VertexLink {
	int16 vx, vy, vz; // the position in bone co-ordinate frame
	int16 primId;     // bone id - -ve number means no bone just "local" frame
	uint32 vertId;    // vertex id
} VertexLink;

// A weighted vertex-bone link
typedef struct WeightedVertexLink {
	VertexLink link;
	uint32 weight; // % weight (fixed-point int scaled to weightScale)
} WeightedVertexLink;

// Each polygon is 6 32-bit WORDS
// Bit 31 ----> Bit 0
//
// 8-bits | 8-bits | 16-bits
// --------------------------
// u0     | v0     | cba
// u1     | v1     | tsb
// u2     | v2     | pad
// --------------------------
// n0              | v0
// n1              | v1
// n2              | v2
//
// u0, v0 = u,v of vertex 0 : 0-255
// u1, v1 = u,v of vertex 1 : 0-255
// u2, v2 = u,v of vertex 2 : 0-255
// cba = weird PSX flag, giving the VRAM x,y of the CLUT to use
// tsb = weird PSX flag, giving the VRAM texture page to use
// pad = padding !
//
// n0, v0 = index into the the normal and vertex pool for normal/vertex 0
// n1, v1 = index into the the normal and vertex pool for normal/vertex 1
// n2, v2 = index into the the normal and vertex pool for normal/vertex 2
//
// The .rap file format
//
// Notice, how poor ANSI C/C++ is at its representation
//

const int RAP_NO_PLATFORM = 0;
const int RAP_PC_PLATFORM = 1;
const int RAP_PSX_PLATFORM = 2;

typedef struct rap_API {
	char id[4];
	uint32 schema;
	uint32 platform;
	uint32 worldScaleShift;
	uint32 weightScaleShift;
	uint32 bothScaleShift;
	uint32 nNone;
	uint32 nSingle;
	uint32 nMultiple;
	uint32 nFUS3;
	uint32 nGUS3;
	uint32 nFTS3;
	uint32 nGTS3;
	uint32 nFUL3;
	uint32 nGUL3;
	uint32 nFTL3;
	uint32 nGTL3;
	uint16 nTRI3;
	uint16 nFrames;
	uint32 nAnimTypes;
	uint32 animPolySize; // in bytes
	uint16 nBones;
	int8 jawBone;            // -1 - means not there
	int8 neckBone;           // - 1 - means not there
	uint32 singleLinkOffset; // in bytes
	uint32 multiLinkOffset;  // in bytes
	uint32 FUS3offset;       // in bytes
	uint32 GUS3offset;       // in bytes
	uint32 FTS3offset;       // in bytes
	uint32 GTS3offset;       // in bytes
	uint32 FUL3offset;       // in bytes
	uint32 GUL3offset;       // in bytes
	uint32 FTL3offset;       // in bytes
	uint32 GTL3offset;       // in bytes
	uint16 TRI3offset;       // in bytes
	uint16 animPolyOffset;   // in bytes
	uint32 normalOffset;     // in bytes
	uint32 boneOffset;       // in bytes
	Vertex noneLinkData[1];  // Vertex noLinkData[nNone];

	Vertex *GetNoneLinkPtr() { return noneLinkData; }

	VertexLink *GetSingleLinkPtr() { return (VertexLink *)(id + singleLinkOffset); }

	WeightedVertexLink *GetMultiLinkPtr() { return (WeightedVertexLink *)(id + multiLinkOffset); }

	uint32 *GetFUS3Ptr() { return (uint32 *)(id + FUS3offset); }
	uint32 *GetGUS3Ptr() { return (uint32 *)(id + GUS3offset); }
	uint32 *GetFTS3Ptr() { return (uint32 *)(id + FTS3offset); }
	uint32 *GetGTS3Ptr() { return (uint32 *)(id + GTS3offset); }
	uint32 *GetFUL3Ptr() { return (uint32 *)(id + FUL3offset); }
	uint32 *GetGUL3Ptr() { return (uint32 *)(id + GUL3offset); }
	uint32 *GetFTL3Ptr() { return (uint32 *)(id + FTL3offset); }
	uint32 *GetGTL3Ptr() { return (uint32 *)(id + GTL3offset); }
	uint32 *GetTRI3Ptr() { return (uint32 *)(id + TRI3offset); }
	uint32 *GetNormalPtr() { return (uint32 *)(id + normalOffset); }
	BoneLink *GetBonePtr() { return (BoneLink *)(id + boneOffset); }
	uint32 *GetBoneHashPtr() {
		BoneLink *bPtr = GetBonePtr();
		return (uint32 *)(bPtr + nBones);
	}

	uint32 *GetAnimPolyPtr() { return (uint32 *)(id + animPolyOffset); }
	uint32 *GetAnimPolyFrame(int frame) { return (uint32 *)(id + animPolyOffset + nAnimTypes * 2 * sizeof(uint32) + frame * animPolySize); }

} rap_API;

inline void ConvertRAP(rap_API *rap) {
	// Do we need to do any conversion ?
	if (rap->schema == RAP_API_SCHEMA)
		return;

	// You can't so a schema check will fail !
	return;
}

} // End of namespace ICB

#endif // #ifndef RAP_API_HH
