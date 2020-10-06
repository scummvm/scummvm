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

#ifndef ICB_PX_ANIMS_H_
#define ICB_PX_ANIMS_H_

#include "engines/icb/common/px_common.h"

namespace ICB {

#define PSX_PXANIM_SCHEMA 5
#define PC_PXANIM_SCHEMA 5

#define PXANIM_SCHEMA PC_PXANIM_SCHEMA

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

#if PXANIM_SCHEMA == 1

#define TRI_POS 2
#define TRI_STRING "TRI"

#undef INT_TYPE
#define INT_TYPE *(uint32 *)INT_STRING

#undef TRI_TYPE
#define TRI_TYPE *(uint32 *)TRI_STRING

#endif // #if PXANIM_SCHEMA == 1

// PXmarker_PC : the PC version
class PXmarker_PC {
private:
	uint32 m_type;
	float m_x, m_y, m_z;
	float m_pan;

#if PXANIM_SCHEMA == 1
	float dummy1, dummy2;
#endif

public:
	void SetType(uint32 type) { m_type = type; }
	void SetPan(float pan) { m_pan = pan; }
	void SetXYZ(float x, float y, float z);

	uint32 GetType(void) { return m_type; }
	void GetPan(float *pan) { *pan = m_pan; }
	void GetXYZ(float *x, float *y, float *z);
};

inline void PXmarker_PC::SetXYZ(float x, float y, float z) {
	m_x = x;
	m_y = y;
	m_z = z;
}

inline void PXmarker_PC::GetXYZ(float *x, float *y, float *z) {
	*x = m_x;
	*y = m_y;
	*z = m_z;
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
class PXmarker_PSX {
private:
	uint8 m_type;
	uint8 x8;
	uint16 x7y9;
	uint32 y6z15pan11;

public:
	void SetType(uint8 type) { m_type = type; }
	void SetPackedXYZPan(uint8 _x8, uint16 _x7y9, uint32 _y6z15pan11);

	uint8 GetType(void) const { return m_type; }

	void GetPan(float *pan) const;
	void GetXYZ(float *x, float *y, float *z) const;
};

inline void PXmarker_PSX::SetPackedXYZPan(uint8 _x8, uint16 _x7y9, uint32 _y6z15pan11) {
	x8 = _x8;
	x7y9 = _x7y9;
	y6z15pan11 = _y6z15pan11;
}

inline void PXmarker_PSX::GetPan(float *pan) const { *pan = (float)(((y6z15pan11 & 0x7FF) << 1)) / 4096.0f; }

inline void PXmarker_PSX::GetXYZ(float *x, float *y, float *z) const {
	int ix, iy, iz;

	ix = ((x8 << 7) | (x7y9 >> 9));
	if (ix >= 16384)
		ix = ix - 32768;

	iy = (((x7y9 & 0x1FF) << 6) | (y6z15pan11 >> 26));
	if (iy >= 16384)
		iy = iy - 32768;

	iz = ((y6z15pan11 >> 11) & 0x7FFF);
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
	int schema;
	uint8 frame_qty;
	uint8 speed;
	uint16 offsets[1];
} PXanim_PSX;

typedef struct {
	char tag[4];
	int schema;
	uint8 frame_qty;
	uint8 speed;
	uint16 offsets[1];
} PXanim_PC;

inline void ConvertPXanim(PXanim_PSX *anim) {
	// Support old schema type files
	if (anim->schema == PSX_PXANIM_SCHEMA - 1) {
		int nFrames = anim->frame_qty;
		anim->frame_qty = (uint8)nFrames;
		anim->speed = 1;
		anim->schema = PSX_PXANIM_SCHEMA;
	}
}

inline void ConvertPXanim(PXanim_PC *anim) {
	// Support old schema type files
	if (anim->schema == PC_PXANIM_SCHEMA - 1) {
		int nFrames = anim->frame_qty;
		anim->frame_qty = (uint8)nFrames;
		anim->speed = 1;
		anim->schema = PC_PXANIM_SCHEMA;
	}
}

// The animation, frame, marker
typedef PXframe_PSX PXframe;
typedef PXmarker_PSX PXmarker;

} // End of namespace ICB

#endif //  _library__PX_ANIMS_H_
