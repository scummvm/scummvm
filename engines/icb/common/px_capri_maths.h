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

#ifndef ICB_PC_CAPRI_MATHS_H
#define ICB_PC_CAPRI_MATHS_H

#include "engines/icb/common/px_capri_maths_pc.h"
#include "engines/icb/common/px_rccommon.h"

namespace ICB {

#if (_PSX_ON_PC == 0) && !defined ICB_PSX_PCDEFINES_H

// make our own equivalents
typedef struct MATRIX {
	short m[3][3]; /* 3x3 rotation matrix */
	short pad;
	int32 t[3]; /* transfer vector */
	MATRIX() { pad = 0; }
} MATRIX;

/* int32 word type 3D vector */
typedef struct VECTOR {
	int32 vx, vy;
	int32 vz, pad;
	VECTOR() { pad = 0; }
} VECTOR;

/* short word type 3D vector */
typedef struct SVECTOR {
	short vx, vy;
	short vz, pad;
	SVECTOR() { pad = 0; }
	bool operator==(const SVECTOR &v) { return ((v.vx == vx) && (v.vy == vy) && (v.vz == vz)); }
} SVECTOR;

/* short word type 3D vector */
typedef struct CVECTOR {
	uint8 r, g;
	short b, pad;
	CVECTOR() { pad = 0; }
	bool operator==(const CVECTOR &v) { return ((v.r == r) && (v.g == g) && (v.b == b)); }
} CVECTOR;

#endif // #if (_PSX_ON_PC==0)

#define ONE 4096
#define myPI (3.141592654f)

inline int myNINT(float f) {
	if (f >= 0.0f)
		return int(f + 0.5f);
	else
		return int(f - 0.5f);
}

#define VectorNormal myVectorNormal
#define ApplyMatrixLV myApplyMatrixLV
#define ApplyMatrixSV myApplyMatrixSV
#define RotMatrix_gte myRotMatrix_gte
#define gte_MulMatrix0 mygte_MulMatrix0
#define gte_RotTrans mygte_RotTrans
#define gte_RotTransPers mygte_RotTransPers
#define gte_RotTransPers3 mygte_RotTransPers3
#define gte_SetRotMatrix mygte_SetRotMatrix
#define gte_SetTransMatrix mygte_SetTransMatrix
#define gte_ApplyRotMatrix mygte_ApplyRotMatrix
#define gte_SetGeomScreen mygte_SetGeomScreen
#define gte_SetBackColor mygte_SetBackColor
#define gte_SetColorMatrix mygte_SetColorMatrix
#define gte_SetLightMatrix mygte_SetLightMatrix
#define gte_NormalColorCol mygte_NormalColorCol
#define gte_NormalColorCol3 mygte_NormalColorCol3
#define gte_NormalClip mygte_NormalClip
#define gte_AverageZ3 mygte_AverageZ3

extern MATRIX gterot;
extern MATRIX gtetrans;
extern MATRIX gtecolour;
extern MATRIX gtelight;
extern short gteback[3];
extern int32 gtegeomscrn;
extern uint8 dcache[1024];

#define getScratchAddr(x) ((uint32 *)(dcache + x))

inline void myApplyMatrixLV(MATRIX *m, VECTOR *invec, VECTOR *outvec);

inline void myApplyMatrixSV(MATRIX *m, SVECTOR *invec, SVECTOR *outvec);

inline int32 myVectorNormal(VECTOR *in0, VECTOR *out0);

inline void mygte_MulMatrix0(MATRIX *m1, MATRIX *m2, MATRIX *out);

inline void mygte_RotTrans(SVECTOR *in0, VECTOR *out0, int32 *flag);

inline void mygte_RotTransPers(SVECTOR *in0, int32 *sxy0, int32 *p, int32 *flag, int32 *z);

inline void mygte_RotTransPers3(SVECTOR *in0, SVECTOR *in1, SVECTOR *in2, int32 *sxy0, int32 *sxy1, int32 *sxy2, int32 *p, int32 *flag, int32 *z);

inline void mygte_SetRotMatrix(MATRIX *m);

inline void mygte_SetTransMatrix(MATRIX *m);

inline void mygte_ApplyRotMatrix(SVECTOR *invec, VECTOR *outvec);

inline void myRotMatrix_gte(SVECTOR *rot, MATRIX *m);

inline void mygte_SetColorMatrix(MATRIX *m);

inline void mygte_SetLightMatrix(MATRIX *m);

inline void mygte_SetGeomScreen(int32 h);

inline void mygte_SetBackColor(int32 r, int32 g, int32 b);

inline void mygte_NormalColorCol(SVECTOR *v0, CVECTOR *in0, CVECTOR *out0);

inline void mygte_NormalColorCol3(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *in0, CVECTOR *out0, CVECTOR *out1, CVECTOR *out2);

inline void mygte_NormalClip(int32 sxy0, int32 sxy1, int32 sxy2, int32 *flag);

inline void mygte_AverageZ3(int32 z0, int32 z1, int32 z2, int32 *sz);

inline void myApplyMatrixLV(MATRIX *m, VECTOR *invec, VECTOR *outvec) {
	outvec->vx = ((int)m->m[0][0] * invec->vx + (int)m->m[0][1] * invec->vy + (int)m->m[0][2] * invec->vz) / 4096;
	outvec->vy = ((int)m->m[1][0] * invec->vx + (int)m->m[1][1] * invec->vy + (int)m->m[1][2] * invec->vz) / 4096;
	outvec->vz = ((int)m->m[2][0] * invec->vx + (int)m->m[2][1] * invec->vy + (int)m->m[2][2] * invec->vz) / 4096;
}

inline void myApplyMatrixSV(MATRIX *m, SVECTOR *invec, SVECTOR *outvec) {
	outvec->vx = (short)(((int)m->m[0][0] * invec->vx + (int)m->m[0][1] * invec->vy + (int)m->m[0][2] * invec->vz) / 4096);
	outvec->vy = (short)(((int)m->m[1][0] * invec->vx + (int)m->m[1][1] * invec->vy + (int)m->m[1][2] * invec->vz) / 4096);
	outvec->vz = (short)(((int)m->m[2][0] * invec->vx + (int)m->m[2][1] * invec->vy + (int)m->m[2][2] * invec->vz) / 4096);
}

inline void mygte_MulMatrix0(MATRIX *m1, MATRIX *m2, MATRIX *out) {
	MATRIX local;
	MATRIX *work;
	if ((out == m1) || (out == m2))
		work = &local;
	else
		work = out;
	work->m[0][0] = (short)(((int)m1->m[0][0] * (int)m2->m[0][0] + (int)m1->m[0][1] * (int)m2->m[1][0] + (int)m1->m[0][2] * (int)m2->m[2][0]) / 4096);
	work->m[0][1] = (short)(((int)m1->m[0][0] * (int)m2->m[0][1] + (int)m1->m[0][1] * (int)m2->m[1][1] + (int)m1->m[0][2] * (int)m2->m[2][1]) / 4096);
	work->m[0][2] = (short)(((int)m1->m[0][0] * (int)m2->m[0][2] + (int)m1->m[0][1] * (int)m2->m[1][2] + (int)m1->m[0][2] * (int)m2->m[2][2]) / 4096);
	work->m[1][0] = (short)(((int)m1->m[1][0] * (int)m2->m[0][0] + (int)m1->m[1][1] * (int)m2->m[1][0] + (int)m1->m[1][2] * (int)m2->m[2][0]) / 4096);
	work->m[1][1] = (short)(((int)m1->m[1][0] * (int)m2->m[0][1] + (int)m1->m[1][1] * (int)m2->m[1][1] + (int)m1->m[1][2] * (int)m2->m[2][1]) / 4096);
	work->m[1][2] = (short)(((int)m1->m[1][0] * (int)m2->m[0][2] + (int)m1->m[1][1] * (int)m2->m[1][2] + (int)m1->m[1][2] * (int)m2->m[2][2]) / 4096);
	work->m[2][0] = (short)(((int)m1->m[2][0] * (int)m2->m[0][0] + (int)m1->m[2][1] * (int)m2->m[1][0] + (int)m1->m[2][2] * (int)m2->m[2][0]) / 4096);
	work->m[2][1] = (short)(((int)m1->m[2][0] * (int)m2->m[0][1] + (int)m1->m[2][1] * (int)m2->m[1][1] + (int)m1->m[2][2] * (int)m2->m[2][1]) / 4096);
	work->m[2][2] = (short)(((int)m1->m[2][0] * (int)m2->m[0][2] + (int)m1->m[2][1] * (int)m2->m[1][2] + (int)m1->m[2][2] * (int)m2->m[2][2]) / 4096);

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

inline void mygte_SetRotMatrix(MATRIX *m) { gterot = *m; }

inline void mygte_SetTransMatrix(MATRIX *m) { gtetrans = *m; }

inline void mygte_ApplyRotMatrix(SVECTOR *invec, VECTOR *outvec) {
	outvec->vx = (((int)gterot.m[0][0] * (int)invec->vx + (int)gterot.m[0][1] * (int)invec->vy + (int)gterot.m[0][2] * (int)invec->vz) / 4096);
	outvec->vy = (((int)gterot.m[1][0] * (int)invec->vx + (int)gterot.m[1][1] * (int)invec->vy + (int)gterot.m[1][2] * (int)invec->vz) / 4096);
	outvec->vz = (((int)gterot.m[2][0] * (int)invec->vx + (int)gterot.m[2][1] * (int)invec->vy + (int)gterot.m[2][2] * (int)invec->vz) / 4096);
}

inline void mygte_RotTrans(SVECTOR *in0, VECTOR *out0, int32 *flag) {
	mygte_ApplyRotMatrix(in0, out0);
	out0->vx += gtetrans.t[0];
	out0->vy += gtetrans.t[1];
	out0->vz += gtetrans.t[2];

	// What GTE flags should we set ?
	*flag = 0;
}

inline void mygte_RotTransPers(SVECTOR *in0, int32 *sxy0, int32 * /* p */, int32 *flag, int32 *z) {
	VECTOR cam;
	SVECTOR *scrn = (SVECTOR *)sxy0;

	gte_RotTrans(in0, &cam, flag);

	*flag = 0;

	if (cam.vz != 0) {
		scrn->vx = (short)((cam.vx * gtegeomscrn) / cam.vz);
		scrn->vy = (short)((cam.vy * gtegeomscrn) / cam.vz);
	} else {
		// To force an error and hence an illegal polygon
		scrn->vx = 2048;
		scrn->vy = 2048;
	}

	*z = cam.vz / 4;

	if (abs(scrn->vx) > 1024)
		*flag |= 0x80000000;
	if (abs(scrn->vy) > 1024)
		*flag |= 0x80000000;

	// set the value of flag : closer than h/2
	if (cam.vz < 0)
		*flag |= 0x80000000;
}

inline void mygte_RotTransPers3(SVECTOR *in0, SVECTOR *in1, SVECTOR *in2, int32 *sxy0, int32 *sxy1, int32 *sxy2, int32 *p, int32 *flag, int32 *z) {
	int32 z0, z1, z2;
	int32 p0, p1, p2;
	int32 flag0, flag1, flag2;

	mygte_RotTransPers(in0, sxy0, &p0, &flag0, &z0);
	mygte_RotTransPers(in1, sxy1, &p1, &flag1, &z1);
	mygte_RotTransPers(in2, sxy2, &p2, &flag2, &z2);

	// What GTE flags should we set ?
	*flag = flag0 | flag1 | flag2;
	*p = p2;
	*z = z2;
}

inline void myRotMatrix_gte(SVECTOR *rot, MATRIX *m) {
	const int one = (1 << 12);
	float ang0 = (float)rot->vx * 2.0f * myPI / one;
	MATRIX m0;
	int c0 = myNINT(one * (float)cos(ang0));
	int s0 = myNINT(one * (float)sin(ang0));
	m0.m[0][0] = one;
	m0.m[0][1] = 0;
	m0.m[0][2] = 0;

	m0.m[1][0] = 0;
	m0.m[1][1] = (short)c0;
	m0.m[1][2] = (short)-s0;

	m0.m[2][0] = 0;
	m0.m[2][1] = (short)s0;
	m0.m[2][2] = (short)c0;

	float ang1 = (float)rot->vy * 2.0f * myPI / one;
	int c1 = myNINT(one * (float)cos(ang1));
	int s1 = myNINT(one * (float)sin(ang1));
	MATRIX m1;
	m1.m[0][0] = (short)c1;
	m1.m[0][1] = 0;
	m1.m[0][2] = (short)s1;

	m1.m[1][0] = 0;
	m1.m[1][1] = one;
	m1.m[1][2] = 0;

	m1.m[2][0] = (short)-s1;
	m1.m[2][1] = 0;
	m1.m[2][2] = (short)c1;

	float ang2 = (float)rot->vz * 2.0f * myPI / one;
	int c2 = myNINT(one * (float)cos(ang2));
	int s2 = myNINT(one * (float)sin(ang2));
	MATRIX m2;

	m2.m[0][0] = (short)c2;
	m2.m[0][1] = (short)-s2;
	m2.m[0][2] = 0;

	m2.m[1][0] = (short)s2;
	m2.m[1][1] = (short)c2;
	m2.m[1][2] = 0;

	m2.m[2][0] = 0;
	m2.m[2][1] = 0;
	m2.m[2][2] = one;

	mygte_MulMatrix0(&m0, &m1, m);
	mygte_MulMatrix0(m, &m2, m);
}

inline void mygte_SetBackColor(int32 r, int32 g, int32 b) {
	gteback[0] = (short)r;
	gteback[1] = (short)g;
	gteback[2] = (short)b;
}

inline void mygte_SetColorMatrix(MATRIX *m) { gtecolour = *m; }

inline void mygte_SetLightMatrix(MATRIX *m) { gtelight = *m; }

inline void mygte_SetGeomScreen(int32 h) { gtegeomscrn = h; }

inline void mygte_NormalColorCol(SVECTOR *v0, CVECTOR *in0, CVECTOR *out0) {
	SVECTOR lightEffect;
	// Normal line vector(local) -> light source effect
	ApplyMatrixSV(&gtelight, v0, &lightEffect);
	if (lightEffect.vx < 0)
		lightEffect.vx = 0;
	if (lightEffect.vy < 0)
		lightEffect.vy = 0;
	if (lightEffect.vz < 0)
		lightEffect.vz = 0;

	// Light source effect -> Colour effect(local colour matrix+back colour)
	SVECTOR colourEffect;
	ApplyMatrixSV(&gtecolour, &lightEffect, &colourEffect);
	if (colourEffect.vx < 0)
		colourEffect.vx = 0;
	if (colourEffect.vy < 0)
		colourEffect.vy = 0;
	if (colourEffect.vz < 0)
		colourEffect.vz = 0;

	// colourEffect is 0-4095 (2^12)
	// gteback is 0-255 (2^8)
	colourEffect.vx = (short)((colourEffect.vx >> 4) + gteback[0]);
	colourEffect.vy = (short)((colourEffect.vy >> 4) + gteback[1]);
	colourEffect.vz = (short)((colourEffect.vz >> 4) + gteback[2]);

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

inline void mygte_NormalColorCol3(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *in0, CVECTOR *out0, CVECTOR *out1, CVECTOR *out2) {
	gte_NormalColorCol(v0, in0, out0);
	gte_NormalColorCol(v1, in0, out1);
	gte_NormalColorCol(v2, in0, out2);
}

inline int32 myVectorNormal(VECTOR *in0, VECTOR *out0) {
	int32 r2 = (in0->vx * in0->vx + in0->vy * in0->vy + in0->vz * in0->vz);
	float r = (float)sqrt((float)r2) / 4096.0f;

	if (fabs(r) < 1.0e-6)
		return 0;

	out0->vx = (int32)((float)in0->vx / r);
	out0->vy = (int32)((float)in0->vy / r);
	out0->vz = (int32)((float)in0->vz / r);
	return r2;
}

inline void mygte_NormalClip(int32 sxy0, int32 sxy1, int32 sxy2, int32 *flag) {
	SVECTOR *v0 = (SVECTOR *)&sxy0;
	SVECTOR *v1 = (SVECTOR *)&sxy1;
	SVECTOR *v2 = (SVECTOR *)&sxy2;

	// compute the cross-product of (v1-v0) x (v2-v0)
	int l0x = v1->vx - v0->vx;
	int l0y = v1->vy - v0->vy;
	int l1x = v2->vx - v0->vx;
	int l1y = v2->vy - v0->vy;

	*flag = ((l0x * l1y) - (l0y * l1x));
}

inline void mygte_AverageZ3(int32 z0, int32 z1, int32 z2, int32 *sz) {
	*sz = (z0 + z1 + z2) / 3;
	*sz /= 4;
}

} // End of namespace ICB

#endif // #ifndef __PC_CAPRI_MATHS_H
