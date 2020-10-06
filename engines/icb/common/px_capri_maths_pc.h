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

#ifndef ICB_PX_CAPRI_MATHS_PC_H
#define ICB_PX_CAPRI_MATHS_PC_H

#include "engines/icb/common/px_rccommon.h"

#include "common/util.h"

namespace ICB {

#if (_PSX_ON_PC == 0) && !defined ICB_PX_CAPRI_MATHS_PC_H

// make our own equivalents
typedef struct MATRIXPC {
	int m[3][3]; /* 3x3 rotation matrix */
	int pad;
	int32 t[3]; /* transfer vector */
	MATRIXPC() { pad = 0; }
} MATRIXPC;

/* int32 word type 3D vector */
typedef struct VECTOR {
	int32 vx, vy;
	int32 vz, pad;
	VECTOR() { pad = 0; }
} VECTOR;

/* short word type 3D vector */
typedef struct SVECTORPC {
	int vx, vy;
	int vz, pad;
	SVECTORPC() { pad = 0; }
	bool operator==(const SVECTORPC &v) { return ((v.vx == vx) && (v.vy == vy) && (v.vz == vz)); }
} SVECTORPC;

/* short word type 3D vector */
typedef struct CVECTOR {
	uint8 r, g;
	short b, pad;
	CVECTOR() { pad = 0; }
	bool operator==(const CVECTOR &v) { return ((v.r == r) && (v.g == g) && (v.b == b)); }
} CVECTOR;

#endif // #if (_PSX_ON_PC==0)

//-=- Definitions -=-//
const int ONE_PC_SCALE = 12;
const int ONE_PC = 1 << ONE_PC_SCALE;
const float myPI_PC = 3.141592654f;
const int ZSCALE = 1;

inline int myNINT_PC(float f) {
	if (f >= 0.0f)
		return int(f + 0.5f);
	else
		return int(f - 0.5f);
}

//------------------------------------------------------------------------

#define VectorNormal_pc myVectorNormal_pc
#define ApplyMatrixLV_pc myApplyMatrixLV_pc
#define ApplyMatrixSV_pc myApplyMatrixSV_pc
#define RotMatrix_gte_pc myRotMatrix_gte_pc
#define gte_MulMatrix0_pc mygte_MulMatrix0_pc
#define gte_RotTrans_pc mygte_RotTrans_pc
#define gte_RotTransPers_pc mygte_RotTransPers_pc
#define gte_RotTransPers3_pc mygte_RotTransPers3_pc
#define gte_SetRotMatrix_pc mygte_SetRotMatrix_pc
#define gte_SetTransMatrix_pc mygte_SetTransMatrix_pc
#define gte_ApplyRotMatrix_pc mygte_ApplyRotMatrix_pc
#define gte_SetGeomScreen_pc mygte_SetGeomScreen_pc
#define gte_SetBackColor_pc mygte_SetBackColor_pc
#define gte_SetColorMatrix_pc mygte_SetColorMatrix_pc
#define gte_SetLightMatrix_pc mygte_SetLightMatrix_pc
#define gte_NormalColorCol_pc mygte_NormalColorCol_pc
#define gte_NormalColorCol3_pc mygte_NormalColorCol3_pc
#define gte_NormalClip_pc mygte_NormalClip_pc
#define gte_AverageZ3_pc mygte_AverageZ3_pc
#define gte_SetScreenScaleShift_pc mygte_SetScreenScaleShift_pc

//------------------------------------------------------------------------

extern MATRIXPC gterot_pc;
extern MATRIXPC gtetrans_pc;
extern MATRIXPC gtecolour_pc;
extern MATRIXPC gtelight_pc;
extern int gteback_pc[3];
extern int32 gtegeomscrn_pc;
extern int gtescreenscaleshift_pc;

//------------------------------------------------------------------------

inline void myApplyMatrixLV_pc(MATRIXPC *m, VECTOR *invec, VECTOR *outvec);

inline void myApplyMatrixSV_pc(MATRIXPC *m, SVECTORPC *invec, SVECTORPC *outvec);
inline void myApplyMatrixSV_pc(MATRIXPC *m, SVECTOR *invec, SVECTORPC *outvec);

inline int32 myVectorNormal_pc(VECTOR *in0, VECTOR *out0);

inline void mygte_MulMatrix0_pc(MATRIXPC *m1, MATRIXPC *m2, MATRIXPC *out);

inline void mygte_RotTrans_pc(SVECTORPC *in0, VECTOR *out0, int32 *flag);
inline void mygte_RotTrans_pc(SVECTOR *in0, VECTOR *out0, int32 *flag);

inline void mygte_RotTransPers_pc(SVECTORPC *in0, SVECTORPC *sxy0, int32 *p, int32 *flag, int32 *z);
inline void mygte_RotTransPers_pc(SVECTOR *in0, SVECTORPC *sxy0, int32 *p, int32 *flag, int32 *z);

inline void mygte_RotTransPers3_pc(SVECTORPC *in0, SVECTORPC *in1, SVECTORPC *in2, SVECTORPC *sxy0, SVECTORPC *sxy1, SVECTORPC *sxy2, int32 *p, int32 *flag, int32 *z);

inline void mygte_SetRotMatrix_pc(MATRIXPC *m);

inline void mygte_SetTransMatrix_pc(MATRIXPC *m);

inline void mygte_ApplyRotMatrix_pc(SVECTORPC *invec, VECTOR *outvec);

inline void myRotMatrix_gte_pc(SVECTOR *rot, MATRIXPC *m);

inline void mygte_SetColorMatrix_pc(MATRIXPC *m);

inline void mygte_SetLightMatrix_pc(MATRIXPC *m);

inline void mygte_SetGeomScreen_pc(int32 h);

inline void mygte_SetBackColor_pc(int32 r, int32 g, int32 b);

inline void mygte_SetScreenScaleShift_pc(int32 shift);

inline void mygte_NormalColorCol_pc(SVECTOR *v0, CVECTOR *in0, CVECTOR *out0);

inline void mygte_NormalColorCol3_pc(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *in0, CVECTOR *out0, CVECTOR *out1, CVECTOR *out2);

inline void mygte_NormalClip_pc(SVECTORPC *sxy0, SVECTORPC *sxy1, SVECTORPC *sxy2, int32 *flag);
inline void mygte_NormalClip_pc(SVECTOR *sxy0, SVECTOR *sxy1, SVECTOR *sxy2, int32 *flag);

inline void mygte_AverageZ3_pc(int32 z0, int32 z1, int32 z2, int32 *sz);

//------------------------------------------------------------------------

inline void myApplyMatrixLV_pc(MATRIXPC *m, VECTOR *invec, VECTOR *outvec) {
	outvec->vx = (m->m[0][0] * invec->vx + m->m[0][1] * invec->vy + m->m[0][2] * invec->vz) / ONE_PC;
	outvec->vy = (m->m[1][0] * invec->vx + m->m[1][1] * invec->vy + m->m[1][2] * invec->vz) / ONE_PC;
	outvec->vz = (m->m[2][0] * invec->vx + m->m[2][1] * invec->vy + m->m[2][2] * invec->vz) / ONE_PC;
}

//------------------------------------------------------------------------

inline void myApplyMatrixSV_pc(MATRIXPC *m, SVECTORPC *invec, SVECTORPC *outvec) {
	outvec->vx = (int)((m->m[0][0] * invec->vx + m->m[0][1] * invec->vy + m->m[0][2] * invec->vz) / ONE_PC);
	outvec->vy = (int)((m->m[1][0] * invec->vx + m->m[1][1] * invec->vy + m->m[1][2] * invec->vz) / ONE_PC);
	outvec->vz = (int)((m->m[2][0] * invec->vx + m->m[2][1] * invec->vy + m->m[2][2] * invec->vz) / ONE_PC);
}

//------------------------------------------------------------------------

inline void myApplyMatrixSV_pc(MATRIXPC *m, SVECTOR *invec, SVECTORPC *outvec) {
	outvec->vx = (int)((m->m[0][0] * (int)invec->vx + m->m[0][1] * (int)invec->vy + m->m[0][2] * (int)invec->vz) / ONE_PC);
	outvec->vy = (int)((m->m[1][0] * (int)invec->vx + m->m[1][1] * (int)invec->vy + m->m[1][2] * (int)invec->vz) / ONE_PC);
	outvec->vz = (int)((m->m[2][0] * (int)invec->vx + m->m[2][1] * (int)invec->vy + m->m[2][2] * (int)invec->vz) / ONE_PC);
}

//------------------------------------------------------------------------

inline void mygte_MulMatrix0_pc(MATRIXPC *m1, MATRIXPC *m2, MATRIXPC *out) {
	MATRIXPC local;
	MATRIXPC *work;
	if ((out == m1) || (out == m2))
		work = &local;
	else
		work = out;
	work->m[0][0] = (int)((m1->m[0][0] * m2->m[0][0] + m1->m[0][1] * m2->m[1][0] + m1->m[0][2] * m2->m[2][0]) / ONE_PC);
	work->m[0][1] = (int)((m1->m[0][0] * m2->m[0][1] + m1->m[0][1] * m2->m[1][1] + m1->m[0][2] * m2->m[2][1]) / ONE_PC);
	work->m[0][2] = (int)((m1->m[0][0] * m2->m[0][2] + m1->m[0][1] * m2->m[1][2] + m1->m[0][2] * m2->m[2][2]) / ONE_PC);
	work->m[1][0] = (int)((m1->m[1][0] * m2->m[0][0] + m1->m[1][1] * m2->m[1][0] + m1->m[1][2] * m2->m[2][0]) / ONE_PC);
	work->m[1][1] = (int)((m1->m[1][0] * m2->m[0][1] + m1->m[1][1] * m2->m[1][1] + m1->m[1][2] * m2->m[2][1]) / ONE_PC);
	work->m[1][2] = (int)((m1->m[1][0] * m2->m[0][2] + m1->m[1][1] * m2->m[1][2] + m1->m[1][2] * m2->m[2][2]) / ONE_PC);
	work->m[2][0] = (int)((m1->m[2][0] * m2->m[0][0] + m1->m[2][1] * m2->m[1][0] + m1->m[2][2] * m2->m[2][0]) / ONE_PC);
	work->m[2][1] = (int)((m1->m[2][0] * m2->m[0][1] + m1->m[2][1] * m2->m[1][1] + m1->m[2][2] * m2->m[2][1]) / ONE_PC);
	work->m[2][2] = (int)((m1->m[2][0] * m2->m[0][2] + m1->m[2][1] * m2->m[1][2] + m1->m[2][2] * m2->m[2][2]) / ONE_PC);

	if (work != out) {
		out->m[0][0] = work->m[0][0];
		out->m[0][1] = work->m[0][1];
		out->m[0][2] = work->m[0][2];

		out->m[1][0] = work->m[1][0];
		out->m[1][1] = work->m[1][1];
		out->m[1][2] = work->m[1][2];

		out->m[2][0] = work->m[2][0];
		out->m[2][1] = work->m[2][1];
		out->m[2][2] = work->m[2][2];
	}
}

//------------------------------------------------------------------------

inline void mygte_SetRotMatrix_pc(MATRIXPC *m) { gterot_pc = *m; }

//------------------------------------------------------------------------

inline void mygte_SetTransMatrix_pc(MATRIXPC *m) { gtetrans_pc = *m; }

//------------------------------------------------------------------------

inline void mygte_ApplyRotMatrix_pc(SVECTORPC *invec, VECTOR *outvec) {
	outvec->vx = ((gterot_pc.m[0][0] * invec->vx + gterot_pc.m[0][1] * invec->vy + gterot_pc.m[0][2] * invec->vz) / ONE_PC);
	outvec->vy = ((gterot_pc.m[1][0] * invec->vx + gterot_pc.m[1][1] * invec->vy + gterot_pc.m[1][2] * invec->vz) / ONE_PC);
	outvec->vz = ((gterot_pc.m[2][0] * invec->vx + gterot_pc.m[2][1] * invec->vy + gterot_pc.m[2][2] * invec->vz) / ONE_PC);
}

//------------------------------------------------------------------------

inline void mygte_RotTrans_pc(SVECTORPC *in0, VECTOR *out0, int32 *flag) {
	mygte_ApplyRotMatrix_pc(in0, out0);
	out0->vx += gtetrans_pc.t[0];
	out0->vy += gtetrans_pc.t[1];
	out0->vz += gtetrans_pc.t[2];

	// What GTE flags should we set ?
	*flag = 0;
}

//------------------------------------------------------------------------

inline void mygte_RotTrans_pc(SVECTOR *in0, VECTOR *out0, int32 *flag) {
	SVECTORPC sv_pc;
	sv_pc.vx = in0->vx;
	sv_pc.vy = in0->vy;
	sv_pc.vz = in0->vz;

	mygte_ApplyRotMatrix_pc(&sv_pc, out0);

	out0->vx += gtetrans_pc.t[0];
	out0->vy += gtetrans_pc.t[1];
	out0->vz += gtetrans_pc.t[2];

	// What GTE flags should we set ?
	*flag = 0;
}

//------------------------------------------------------------------------

inline void mygte_RotTransPers_pc(SVECTORPC *in0, SVECTORPC *sxy0, int32 * /* p */, int32 *flag, int32 *z) {
	VECTOR cam;
	cam.vx = ((gterot_pc.m[0][0] * in0->vx + gterot_pc.m[0][1] * in0->vy + gterot_pc.m[0][2] * in0->vz) / ONE_PC);
	cam.vy = ((gterot_pc.m[1][0] * in0->vx + gterot_pc.m[1][1] * in0->vy + gterot_pc.m[1][2] * in0->vz) / ONE_PC);
	cam.vz = ((gterot_pc.m[2][0] * in0->vx + gterot_pc.m[2][1] * in0->vy + gterot_pc.m[2][2] * in0->vz) / ONE_PC);
	cam.vx += (gtetrans_pc.t[0] << gtescreenscaleshift_pc);
	cam.vy += (gtetrans_pc.t[1] << gtescreenscaleshift_pc);
	cam.vz += (gtetrans_pc.t[2] << gtescreenscaleshift_pc);

	*flag = 0;

	if (cam.vz != 0) {
		sxy0->vx = (int)((cam.vx * gtegeomscrn_pc) / cam.vz);
		sxy0->vy = (int)((cam.vy * gtegeomscrn_pc) / cam.vz);
	} else {
		// To force an error and hence an illegal polygon
		sxy0->vx = 2048;
		sxy0->vy = 2048;
	}

	cam.vz >>= gtescreenscaleshift_pc;
	*z = cam.vz / 4;

	if (abs(sxy0->vx) > 1024)
		*flag |= 0x80000000;
	if (abs(sxy0->vy) > 1024)
		*flag |= 0x80000000;

	// set the value of flag : closer than h/2
	if (cam.vz < 0)
		*flag |= 0x80000000;
}

//------------------------------------------------------------------------

inline void mygte_RotTransPers_pc(SVECTOR *in0, SVECTORPC *sxy0, int32 * /* p */, int32 *flag, int32 *z) {
	VECTOR cam;
	cam.vx = ((gterot_pc.m[0][0] * (int)in0->vx + gterot_pc.m[0][1] * (int)in0->vy + gterot_pc.m[0][2] * (int)in0->vz) / ONE_PC);
	cam.vy = ((gterot_pc.m[1][0] * (int)in0->vx + gterot_pc.m[1][1] * (int)in0->vy + gterot_pc.m[1][2] * (int)in0->vz) / ONE_PC);
	cam.vz = ((gterot_pc.m[2][0] * (int)in0->vx + gterot_pc.m[2][1] * (int)in0->vy + gterot_pc.m[2][2] * (int)in0->vz) / ONE_PC);
	cam.vx += (gtetrans_pc.t[0] << gtescreenscaleshift_pc);
	cam.vy += (gtetrans_pc.t[1] << gtescreenscaleshift_pc);
	cam.vz += (gtetrans_pc.t[2] << gtescreenscaleshift_pc);

	*flag = 0;

	if (cam.vz != 0) {
		sxy0->vx = (int)((cam.vx * gtegeomscrn_pc) / cam.vz);
		sxy0->vy = (int)((cam.vy * gtegeomscrn_pc) / cam.vz);
	} else {
		// To force an error and hence an illegal polygon
		sxy0->vx = 2048;
		sxy0->vy = 2048;
	}

	cam.vz >>= gtescreenscaleshift_pc;
	*z = cam.vz / 4;

	if (abs(sxy0->vx) > 1024)
		*flag |= 0x80000000;
	if (abs(sxy0->vy) > 1024)
		*flag |= 0x80000000;

	// set the value of flag : closer than h/2
	if (cam.vz < 0)
		*flag |= 0x80000000;
}

//------------------------------------------------------------------------

inline void mygte_RotTransPers3_pc(SVECTORPC *in0, SVECTORPC *in1, SVECTORPC *in2, SVECTORPC *sxy0, SVECTORPC *sxy1, SVECTORPC *sxy2, int32 *p, int32 *flag, int32 *z) {
	int32 z0, z1, z2;
	int32 p0, p1, p2;
	int32 flag0, flag1, flag2;

	mygte_RotTransPers_pc(in0, sxy0, &p0, &flag0, &z0);
	mygte_RotTransPers_pc(in1, sxy1, &p1, &flag1, &z1);
	mygte_RotTransPers_pc(in2, sxy2, &p2, &flag2, &z2);

	// What GTE flags should we set ?
	*flag = flag0 | flag1 | flag2;
	*p = p2;
	*z = z2;
}

//------------------------------------------------------------------------

inline void myRotMatrix_gte_pc(SVECTOR *rot, MATRIXPC *m) {
	float ang0 = (float)rot->vx * 2.0f * myPI_PC / 4096;
	MATRIXPC m0;
	int c0 = myNINT_PC(ONE_PC * (float)cos(ang0));
	int s0 = myNINT_PC(ONE_PC * (float)sin(ang0));
	m0.m[0][0] = ONE_PC;
	m0.m[0][1] = 0;
	m0.m[0][2] = 0;

	m0.m[1][0] = 0;
	m0.m[1][1] = c0;
	m0.m[1][2] = -s0;

	m0.m[2][0] = 0;
	m0.m[2][1] = s0;
	m0.m[2][2] = c0;

	float ang1 = (float)rot->vy * 2.0f * myPI_PC / 4096;
	int c1 = myNINT_PC(ONE_PC * (float)cos(ang1));
	int s1 = myNINT_PC(ONE_PC * (float)sin(ang1));
	MATRIXPC m1;
	m1.m[0][0] = c1;
	m1.m[0][1] = 0;
	m1.m[0][2] = s1;

	m1.m[1][0] = 0;
	m1.m[1][1] = ONE_PC;
	m1.m[1][2] = 0;

	m1.m[2][0] = -s1;
	m1.m[2][1] = 0;
	m1.m[2][2] = c1;

	float ang2 = (float)rot->vz * 2.0f * myPI_PC / 4096;
	int c2 = myNINT_PC(ONE_PC * (float)cos(ang2));
	int s2 = myNINT_PC(ONE_PC * (float)sin(ang2));
	MATRIXPC m2;

	m2.m[0][0] = c2;
	m2.m[0][1] = -s2;
	m2.m[0][2] = 0;

	m2.m[1][0] = s2;
	m2.m[1][1] = c2;
	m2.m[1][2] = 0;

	m2.m[2][0] = 0;
	m2.m[2][1] = 0;
	m2.m[2][2] = ONE_PC;

	mygte_MulMatrix0_pc(&m0, &m1, m);
	mygte_MulMatrix0_pc(m, &m2, m);
}

//------------------------------------------------------------------------

inline void mygte_SetBackColor_pc(int32 r, int32 g, int32 b) {
	gteback_pc[0] = r;
	gteback_pc[1] = g;
	gteback_pc[2] = b;
}

//------------------------------------------------------------------------

inline void mygte_SetColorMatrix_pc(MATRIXPC *m) { gtecolour_pc = *m; }

//------------------------------------------------------------------------

inline void mygte_SetLightMatrix_pc(MATRIXPC *m) { gtelight_pc = *m; }

//------------------------------------------------------------------------

inline void mygte_SetGeomScreen_pc(int32 h) { gtegeomscrn_pc = h; }

//------------------------------------------------------------------------

inline void mygte_NormalColorCol_pc(SVECTOR *v0, CVECTOR *in0, CVECTOR *out0) {
	SVECTORPC lightEffect;
	// Normal line vector(local) -> light source effect
	ApplyMatrixSV_pc(&gtelight_pc, v0, &lightEffect);
	if (lightEffect.vx < 0)
		lightEffect.vx = 0;
	if (lightEffect.vy < 0)
		lightEffect.vy = 0;
	if (lightEffect.vz < 0)
		lightEffect.vz = 0;

	// Light source effect -> Colour effect(local colour matrix+back colour)
	SVECTORPC colourEffect;
	ApplyMatrixSV_pc(&gtecolour_pc, &lightEffect, &colourEffect);
	if (colourEffect.vx < 0)
		colourEffect.vx = 0;
	if (colourEffect.vy < 0)
		colourEffect.vy = 0;
	if (colourEffect.vz < 0)
		colourEffect.vz = 0;

	// colourEffect is 0-ONE_PC (2^ONE_PC_SCALE)
	// gteback is 0-255 (2^8)
	colourEffect.vx = ((colourEffect.vx >> (ONE_PC_SCALE - 8)) + gteback_pc[0]);
	colourEffect.vy = ((colourEffect.vy >> (ONE_PC_SCALE - 8)) + gteback_pc[1]);
	colourEffect.vz = ((colourEffect.vz >> (ONE_PC_SCALE - 8)) + gteback_pc[2]);

	// 256 = 1.0 in colourEffect
	// 128 = 1.0 in in0
	int red = ((in0->r * colourEffect.vx) >> 8);
	int green = ((in0->g * colourEffect.vy) >> 8);
	int blue = ((in0->b * colourEffect.vz) >> 8);

	if (red > 255)
		red = 255;
	if (green > 255)
		green = 255;
	if (blue > 255)
		blue = 255;

	out0->r = (uint8)(red);
	out0->g = (uint8)(green);
	out0->b = (uint8)(blue);
}

//------------------------------------------------------------------------

inline void mygte_NormalColorCol3_pc(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *in0, CVECTOR *out0, CVECTOR *out1, CVECTOR *out2) {
	gte_NormalColorCol_pc(v0, in0, out0);
	gte_NormalColorCol_pc(v1, in0, out1);
	gte_NormalColorCol_pc(v2, in0, out2);
}

//------------------------------------------------------------------------

inline int32 myVectorNormal_pc(VECTOR *in0, VECTOR *out0) {
	int32 r2 = (in0->vx * in0->vx + in0->vy * in0->vy + in0->vz * in0->vz);
	float r = (float)sqrt((float)r2) / (float)ONE_PC;

	if (fabs(r) < 1.0e-6)
		return 0;

	out0->vx = (int32)((float)in0->vx / r);
	out0->vy = (int32)((float)in0->vy / r);
	out0->vz = (int32)((float)in0->vz / r);
	return r2;
}

//////////////////////////////////////////////////////////////////////

inline void mygte_NormalClip_pc(SVECTORPC *sxy0, SVECTORPC *sxy1, SVECTORPC *sxy2, int32 *flag) {
	// compute the cross-product of (v1-v0) x (v2-v0)
	int l0x = sxy1->vx - sxy0->vx;
	int l0y = sxy1->vy - sxy0->vy;
	int l1x = sxy2->vx - sxy0->vx;
	int l1y = sxy2->vy - sxy0->vy;

	*flag = ((l0x * l1y) - (l0y * l1x));
}

//------------------------------------------------------------------------

inline void mygte_NormalClip_pc(SVECTOR *sxy0, SVECTOR *sxy1, SVECTOR *sxy2, int32 *flag) {
	// compute the cross-product of (v1-v0) x (v2-v0)
	int l0x = sxy1->vx - sxy0->vx;
	int l0y = sxy1->vy - sxy0->vy;
	int l1x = sxy2->vx - sxy0->vx;
	int l1y = sxy2->vy - sxy0->vy;

	*flag = ((l0x * l1y) - (l0y * l1x));
}

//------------------------------------------------------------------------

inline void mygte_AverageZ3_pc(int32 z0, int32 z1, int32 z2, int32 *sz) {
	*sz = (z0 + z1 + z2) / 3;
	*sz /= 4;
}

//------------------------------------------------------------------------

inline void mygte_SetScreenScaleShift_pc(int32 shift) { gtescreenscaleshift_pc = shift; }

//------------------------------------------------------------------------

} // End of namespace ICB

#endif // #ifndef __PC_CAPRI_MATHS_PC_H
