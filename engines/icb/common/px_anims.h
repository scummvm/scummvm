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

#ifndef ICB_PX_ANIMS_H_
#define ICB_PX_ANIMS_H_

#include "engines/icb/common/px_common.h"

#include "common/endian.h"

namespace ICB {

#define PXANIM_SCHEMA 5

#define PXANIM_TAG "Peas"
#define ORG_POS 0
#define ORG_STRING "ORG"
#define INT_POS 1
#define INT_STRING "INT"
#define OBJ_STRING "OBJ"

#define ORG_TYPE 0
#define INT_TYPE 1
#define INT0_TYPE 2 // Forced export of INT marker on frame 0 of anim

#define TRI_TYPE 3
#define OBJ_TYPE 4

// PXmarker_PC : the PC version
typedef struct {
	uint32 m_type;
	float m_x, m_y, m_z;
	float m_pan;
} PXmarker_PC;

// PXmarker_PC : the PC version
class PXmarker_PC_Object {
public:
	static void SetType(PXmarker_PC *marker, uint32 type) { marker->m_type = type; }
	static void SetPan(PXmarker_PC *marker, float pan) { marker->m_pan = pan; }
	static void SetXYZ(PXmarker_PC *marker, float x, float y, float z);

	static uint32 GetType(PXmarker_PC *marker) { return marker->m_type; }
	static void GetPan(PXmarker_PC *marker, float *pan) { *pan = marker->m_pan; }
	static void GetXYZ(PXmarker_PC *marker, float *x, float *y, float *z);
};

inline void PXmarker_PC_Object::SetXYZ(PXmarker_PC *marker, float x, float y, float z) {
	marker->m_x = x;
	marker->m_y = y;
	marker->m_z = z;
}

inline void PXmarker_PC_Object::GetXYZ(PXmarker_PC *marker, float *x, float *y, float *z) {
	*x = marker->m_x;
	*y = marker->m_y;
	*z = marker->m_z;
}

// PXframe_PC : the PC version //
typedef struct {
	int16 left_foot_distance;
	int16 right_foot_distance;
	uint8 marker_qty;
	uint8 leftFootStep;
	uint8 rightFootStep;
	uint8 pad3;
	PXmarker_PC markers[1];
} PXframe_PC;

// PXmarker_PSX : the PSX version
typedef struct {
	uint8 m_type;
	uint8 x8;
	uint16 x7y9;
	uint32 y6z15pan11;
} PXmarker_PSX;

// PXmarker_PSX : the PSX version
class PXmarker_PSX_Object {
public:
	static void SetType(PXmarker_PSX *marker, uint8 type) { marker->m_type = type; }
	static void SetPackedXYZPan(PXmarker_PSX *marker, uint8 _x8, uint16 _x7y9, uint32 _y6z15pan11);

	static uint8 GetType(PXmarker_PSX *marker) { return marker->m_type; }

	static void GetPan(PXmarker_PSX *marker, float *pan);
	static void GetXYZ(PXmarker_PSX *marker, float *x, float *y, float *z);
};

inline void PXmarker_PSX_Object::SetPackedXYZPan(PXmarker_PSX *marker, uint8 _x8, uint16 _x7y9, uint32 _y6z15pan11) {
	marker->x8 = _x8;
	marker->x7y9 = _x7y9;
	marker->y6z15pan11 = _y6z15pan11;
}

inline void PXmarker_PSX_Object::GetPan(PXmarker_PSX *marker, float *pan) {
	*pan = (float)(((marker->y6z15pan11 & 0x7FF) << 1)) / 4096.0f;
}

inline void PXmarker_PSX_Object::GetXYZ(PXmarker_PSX *marker, float *x, float *y, float *z) {
	int32 ix, iy, iz;

	ix = ((marker->x8 << 7) | (marker->x7y9 >> 9));
	if (ix >= 16384)
		ix = ix - 32768;

	iy = (((marker->x7y9 & 0x1FF) << 6) | (marker->y6z15pan11 >> 26));
	if (iy >= 16384)
		iy = iy - 32768;

	iz = ((marker->y6z15pan11 >> 11) & 0x7FFF);
	if (iz >= 16384)
		iz = iz - 32768;

	*x = (float)ix;
	*y = (float)iy;
	*z = (float)iz;
}

// PXframe_PSX : the PSX version //
typedef struct {
	int16 left_foot_distance;
	int16 right_foot_distance;
	uint8 marker_qty;
	uint8 leftFootStep;
	uint8 rightFootStep;
	uint8 pad3;
	PXmarker_PSX markers[1];
} PXframe_PSX;

// PXanim //
typedef struct {
	char tag[4];
	int32 schema;
	uint8 frame_qty;
	uint8 speed;
	uint16 offsets[1];
} PXanim_PSX;

typedef struct {
	char tag[4];
	int32 schema;
	uint8 frame_qty;
	uint8 speed;
	uint16 offsets[1];
} PXanim_PC;

inline void ConvertPXanim(PXanim_PSX *anim) {
	// Support old schema type files
	if (FROM_LE_32(anim->schema) == PXANIM_SCHEMA - 1) {
		int32 nFrames = anim->frame_qty;
		anim->frame_qty = (uint8)nFrames;
		anim->speed = 1;
		anim->schema = TO_LE_32(PXANIM_SCHEMA);
	}
}

inline void ConvertPXanim(PXanim_PC *anim) {
	// Support old schema type files
	if (FROM_LE_32(anim->schema) == PXANIM_SCHEMA - 1) {
		int32 nFrames = anim->frame_qty;
		anim->frame_qty = (uint8)nFrames;
		anim->speed = 1;
		anim->schema = TO_LE_32(PXANIM_SCHEMA);
	}
}

// The animation, frame, marker
typedef PXframe_PSX PXframe;
typedef PXmarker_PSX PXmarker;

} // End of namespace ICB

#endif //  _library__PX_ANIMS_H_
