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

#ifndef ICB_RAB_API_HH
#define ICB_RAB_API_HH

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/rap_api.h"

namespace ICB {

// The output file format for the .rab file
#define RAB_API_SCHEMA 7
#define RAB_API_ID "RAB"

// All the different animation frame types used during the
// compression process
#define ALL_ANGLES_32_BYTE_SIZE (4)
#define ALL_DELTAS_24_BYTE_SIZE (3)
#define ALL_DELTAS_16_BYTE_SIZE (2)
#define ALL_DELTAS_8_BYTE_SIZE (1)

#define NONZERO_ANGLES_32_BYTE_SIZE (5)
#define NONZERO_DELTAS_24_BYTE_SIZE (4)
#define NONZERO_DELTAS_16_BYTE_SIZE (3)
#define NONZERO_DELTAS_8_BYTE_SIZE (2)

#define ZERO_ANGLE (0)

#define DELTA_24_NBITS (8)
#define DELTA_16_NBITS (5)
#define DELTA_8_NBITS (2)

#define DELTA_24_ZERO (((1 << (DELTA_24_NBITS - 1)) << (DELTA_24_NBITS + DELTA_24_NBITS)) | ((1 << (DELTA_24_NBITS - 1)) << (DELTA_24_NBITS)) | (1 << (DELTA_24_NBITS - 1)))
#define DELTA_16_ZERO (((1 << (DELTA_16_NBITS - 1)) << (DELTA_16_NBITS + DELTA_16_NBITS)) | ((1 << (DELTA_16_NBITS - 1)) << (DELTA_16_NBITS)) | (1 << (DELTA_16_NBITS - 1)))
#define DELTA_8_ZERO (((1 << (DELTA_8_NBITS - 1)) << (DELTA_8_NBITS + DELTA_8_NBITS)) | ((1 << (DELTA_8_NBITS - 1)) << (DELTA_8_NBITS)) | (1 << (DELTA_8_NBITS - 1)))

// Animation frame structures with max memory pre-allocated in them
// For use in the converter
//
// The animation frame structure is either:
//
// i) store ALL values OR store just non-zero values + bone ID
// ii) store orientations OR store deltas from the previous frame
// iii) use 8, 16, 24, 32-bits to store values
typedef struct FrameHeader {
	BoneLink poseBone;
	uint8 typeSize;
	uint8 nThings;
} FrameHeader;

// The typeSize bit masks
//
// DataTypeBit : 0 = Compressed Angle Triplet, 1 = Delta Triplet
// DataStoreZeroBit : 0 = don't store zero values, 1 = store zero values
// DataSizeBits 3 bits : size in bytes of the data value stored (1,2,3,4)

// Most significant bit
#define DataTypeMask (1 << 7)
#define DataTypeDeltas (1 << 7)
#define DataTypeAngles (0)

// Next one down
#define DataStoreZeroMask (1 << 6)
#define DataStoreZero (1 << 6)
#define DataDontStoreZero (0)

// Lower nibble is DataSizeBits
#define DataSizeBitMask (0xF)

#define ALL_ANGLES_32_TYPESIZE (DataTypeAngles | DataStoreZero | 4)
#define ALL_DELTAS_24_TYPESIZE (DataTypeDeltas | DataStoreZero | 3)
#define ALL_DELTAS_16_TYPESIZE (DataTypeDeltas | DataStoreZero | 2)
#define ALL_DELTAS_8_TYPESIZE (DataTypeDeltas | DataStoreZero | 1)

#define NONZERO_ANGLES_32_TYPESIZE (DataTypeAngles | DataDontStoreZero | 4)
#define NONZERO_DELTAS_24_TYPESIZE (DataTypeDeltas | DataDontStoreZero | 3)
#define NONZERO_DELTAS_16_TYPESIZE (DataTypeDeltas | DataDontStoreZero | 2)
#define NONZERO_DELTAS_8_TYPESIZE (DataTypeDeltas | DataDontStoreZero | 1)

// Basic angle triplet compression packs 3x12-bits into 3x10-bits
#define COMP_ANGLE_RANGE (1 << 10)
#define COMP_ANGLE_SHIFT (2)
#define COMP_VX_SHIFT (20)
#define COMP_VY_SHIFT (10)
#define COMP_VZ_SHIFT (0)
#define COMP_VX_MASK (0x3FF)
#define COMP_VY_MASK (0x3FF)
#define COMP_VZ_MASK (0x3FF)

#define COMP_EXTRA_SHIFT (0)
#define COMP_DELTA_RANGE (COMP_ANGLE_RANGE)

// Generic frame data structure
typedef struct FrameData {
	BoneLink poseBone; // the animated bone for the pose object
	uint8 typeSize;    // FrameHeader but don't want compiler alignment bollocks
	uint8 nThings;
	int8 data[1]; // data[n] : n depends on compression used store frame
} FrameData;

typedef struct MatrixHierarchy {
	MATRIX matrix;
	MatrixHierarchy *parent;
} MatrixHierarchy;

typedef struct MatrixHierarchyPC {
	MATRIXPC matrix;
	MatrixHierarchyPC *parent;
} MatrixHierarchyPC;

typedef uint32 CompTriplet;

typedef struct LinkedMatrix {
	CompTriplet crot; // compresseed angle triplet 10-bits per angle
} LinkedMatrix;

typedef struct Bone_Frame {
	BoneLink poseBone;     // the animated bone for the pose object
	LinkedMatrix bones[1]; // actually LinkedMatrix rot[nBones]
} Bone_Frame;

typedef struct rab_API {
	char id[4];
	uint32 schema;
	uint16 nFrames;
	uint8 nBones;       // nBones must match the RAP file nBones value
	uint8 currentFrame; // the current frame of animation stored

	// byte offsets from start of the file
	uint32 currentFrameOffset;

	// byte offsets from start of the file
	uint32 frameOffsets[1]; // nFrames of them

	Bone_Frame *GetFrame(const int f);

	Bone_Frame *GetCurrentFrame(void) { return (Bone_Frame *)((uint8 *)(id + currentFrameOffset)); }

	FrameData *GetFrameData(const int f) { return (FrameData *)((uint8 *)(id + frameOffsets[f])); }

} rab_API;

// Compress an SVECTOR ( uint16 vx,vy,vz, pad; ) -> uint32
// by dividing the angles (12-bits 0-4095) by four to make them 10-bits
int CompressSVECTOR(SVECTOR rotin, uint32 *rotout);

// Uncompress an SVECTOR ( uint32 -> uint16 vx,vy,vz, pad; )
// by multiplying the angles (10-bits 0-1023) by four to make them 12-bits
inline void ExpandSVECTOR(CompTriplet rotin, SVECTOR *rotout) {
	// Make the angles back up from the compressed 32-bit value
	// x = ( upper 10-bits from part1 ) * 4
	rotout->vx = (int16)(((int16)(rotin >> COMP_VX_SHIFT) & COMP_VX_MASK) << COMP_ANGLE_SHIFT);
	// y = ( middle 10-bits from part1 ) * 4
	rotout->vy = (int16)(((int16)(rotin >> COMP_VY_SHIFT) & COMP_VY_MASK) << COMP_ANGLE_SHIFT);
	// z = ( lower 10-bits from part1 ) * 4
	rotout->vz = (int16)(((int16)(rotin >> COMP_VZ_SHIFT) & COMP_VZ_MASK) << COMP_ANGLE_SHIFT);
}

} // End of namespace ICB

#endif // #ifndef RAB_API_HH
