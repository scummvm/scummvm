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

#ifndef ICB_RAP_API_HH
#define ICB_RAP_API_HH

#include "engines/icb/gfx/psx_pcdefines.h"

#include "common/endian.h"

namespace ICB {

#define RAP_API_SCHEMA 10

#define RAP_API_ID "RAP"

// A bone-bone link
// tx, ty, tz : is translation
// parent : is the parent bone ID
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
// n0, v0 = index into the normal and vertex pool for normal/vertex 0
// n1, v1 = index into the normal and vertex pool for normal/vertex 1
// n2, v2 = index into the normal and vertex pool for normal/vertex 2
//
// The .rap file format
//
// Notice, how poor ANSI C/C++ is at its representation
//

typedef struct {
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
} RapAPI;

class RapAPIObject {
public:
	static Vertex *GetNoneLinkPtr(RapAPI *rap) { return rap->noneLinkData; }

	static VertexLink *GetSingleLinkPtr(RapAPI *rap) { return (VertexLink *)(rap->id + rap->singleLinkOffset); }

	static WeightedVertexLink *GetMultiLinkPtr(RapAPI *rap) { return (WeightedVertexLink *)(rap->id + rap->multiLinkOffset); }

	static uint32 *GetFUS3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->FUS3offset); }
	static uint32 *GetGUS3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->GUS3offset); }
	static uint32 *GetFTS3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->FTS3offset); }
	static uint32 *GetGTS3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->GTS3offset); }
	static uint32 *GetFUL3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->FUL3offset); }
	static uint32 *GetGUL3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->GUL3offset); }
	static uint32 *GetFTL3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->FTL3offset); }
	static uint32 *GetGTL3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->GTL3offset); }
	static uint32 *GetTRI3Ptr(RapAPI *rap) { return (uint32 *)(rap->id + rap->TRI3offset); }
	static uint32 *GetNormalPtr(RapAPI *rap) { return (uint32 *)(rap->id + rap->normalOffset); }
	static BoneLink *GetBonePtr(RapAPI *rap) { return (BoneLink *)(rap->id + rap->boneOffset); }
	static uint32 *GetBoneHashPtr(RapAPI *rap) {
		BoneLink *bPtr = GetBonePtr(rap);
		return (uint32 *)(bPtr + rap->nBones);
	}
	static uint32 *GetAnimPolyPtr(RapAPI *rap) {
		return (uint32 *)(rap->id + rap->animPolyOffset);
		
	}
	static uint32 *GetAnimPolyFrame(RapAPI *rap, int32 frame) {
		return (uint32 *)(rap->id + rap->animPolyOffset + rap->nAnimTypes * 2 * sizeof(uint32) + frame * rap->animPolySize);
	}
};

inline void ConvertRAP(RapAPI *rap) {
	// Do we need to do any conversion ?
	if (FROM_LE_32(rap->schema) == RAP_API_SCHEMA)
		return;

	// You can't so a schema check will fail !
	return;
}

} // End of namespace ICB

#endif // #ifndef RAP_API_HH
