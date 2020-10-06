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

#include "engines/icb/px_common.h"
#include "engines/icb/px_rcutypes.h"
#include "engines/icb/psx_pcdefines.h"
#include "engines/icb/shade.h"

namespace ICB {

// Handy maths function
void makePlaneEquation(FVECTOR *v0, FVECTOR *v1, FVECTOR *v2, int32 *d, FVECTOR *pn) {
	FVECTOR l01, l02;
	// Line from vertex 0 -> vertex 1
	VEC_SUB_VECTOR(*v0, *v1, l01);
	// Line from vertex 0 -> vertex 2
	VEC_SUB_VECTOR(*v0, *v2, l02);

	// the cross-product to get the normal : l01 x l02
	CROSS_PRODUCT_VECTOR(l01, l02, *pn);
	NORMALISE_VECTOR(*pn);

	// the planar distance
	*d = (int32)DOT_PRODUCT_VECTOR(*pn, *v0);
}

/*
// A triangle
typedef struct ShadeTriangle
{
   SVECTOR l01;  // line vertex 0 -> vertex 1
   SVECTOR l12;  // line vertex 1 -> vertex 2
   SVECTOR l20;  // line vertex 2 -> vertex 0
   SVECTOR n01;  // normal to plane & line 01
   SVECTOR n12;  // normal to plane & line 12
   SVECTOR n20;  // normal to plane & line 20
   SVECTOR pn;   // plane normal
   uint32 d;     // plane distance
} ShadeTriangle;
*/

void preprocessShadeData(FVECTOR v[3], ShadeTriangle *s) {
	// find the normal for the shade triangle / polygon !
	double l01x = v[1].vx - v[0].vx;
	double l01y = v[1].vy - v[0].vy;
	double l01z = v[1].vz - v[0].vz;

	double l20x = v[0].vx - v[2].vx;
	double l20y = v[0].vy - v[2].vy;
	double l20z = v[0].vz - v[2].vz;

	double l12x = v[2].vx - v[1].vx;
	double l12y = v[2].vy - v[1].vy;
	double l12z = v[2].vz - v[1].vz;

	// the cross-product to get the normal : l01 x l02
	double nx, ny, nz;
	CROSS_PRODUCT(l01x, l01y, l01z, -l20x, -l20y, -l20z, nx, ny, nz);
	NORMALISE(nx, ny, nz);

	// the planar distance
	double d = DOT_PRODUCT(nx, ny, nz, v[0].vx, v[0].vy, v[0].vz);

	// n01 is normal in the plane of l01 : n01 = l01 x n
	double n01x, n01y, n01z;
	CROSS_PRODUCT(l01x, l01y, l01z, nx, ny, nz, n01x, n01y, n01z);
	NORMALISE(n01x, n01y, n01z);
	// n12 is normal in the plane of l12 : n12 = l12 x n
	double n12x, n12y, n12z;
	CROSS_PRODUCT(l12x, l12y, l12z, nx, ny, nz, n12x, n12y, n12z);
	NORMALISE(n12x, n12y, n12z);
	// n20 is normal in the plane of l20 : n20 = l20 x n
	double n20x, n20y, n20z;
	CROSS_PRODUCT(l20x, l20y, l20z, nx, ny, nz, n20x, n20y, n20z);
	NORMALISE(n20x, n20y, n20z);

	double n01dots0, n12dots1, n20dots2;
	n01dots0 = DOT_PRODUCT(n01x, n01y, n01z, v[0].vx, v[0].vy, v[0].vz);
	n12dots1 = DOT_PRODUCT(n12x, n12y, n12z, v[1].vx, v[1].vy, v[1].vz);
	n20dots2 = DOT_PRODUCT(n20x, n20y, n20z, v[2].vx, v[2].vy, v[2].vz);

	// Store the pre-processed data
	s->l01.vx = (short)l01x;
	s->l01.vy = (short)l01y;
	s->l01.vz = (short)l01z;

	s->l12.vx = (short)l12x;
	s->l12.vy = (short)l12y;
	s->l12.vz = (short)l12z;

	s->l20.vx = (short)l20x;
	s->l20.vy = (short)l20y;
	s->l20.vz = (short)l20z;

	s->n01.vx = (short)(4096.0f * n01x);
	s->n01.vy = (short)(4096.0f * n01y);
	s->n01.vz = (short)(4096.0f * n01z);

	s->n12.vx = (short)(4096.0f * n12x);
	s->n12.vy = (short)(4096.0f * n12y);
	s->n12.vz = (short)(4096.0f * n12z);

	s->n20.vx = (short)(4096.0f * n20x);
	s->n20.vy = (short)(4096.0f * n20y);
	s->n20.vz = (short)(4096.0f * n20z);

	s->pn.vx = (short)(4096.0f * nx);
	s->pn.vy = (short)(4096.0f * ny);
	s->pn.vz = (short)(4096.0f * nz);

	s->d = (int)(4096.0f * d);
	s->n01dots0 = (int)n01dots0;
	s->n12dots1 = (int)n12dots1;
	s->n20dots2 = (int)n20dots2;
}

/*
// A quad
typedef struct ShadeQuad
{
   SVECTOR l01;       // line vertex 0 -> vertex 1
   SVECTOR l12;       // line vertex 1 -> vertex 2
   SVECTOR l23;       // line vertex 2 -> vertex 3
   SVECTOR l30;       // line vertex 3 -> vertex 0
   SVECTOR n01;       // normal to plane & line 01
   SVECTOR n12;       // normal to plane & line 12
   SVECTOR n23;       // normal to plane & line 23
   SVECTOR n30;       // normal to plane & line 30
   SVECTOR pn;        // plane normal
   uint32 d;          // plane distance
   uint32 n01dots0;   // n01 . vertex 0
   uint32 n12dots1;   // n12 . vertex 1
   uint32 n20dots2;   // n23 . vertex 2
   uint32 n20dots3;   // n30 . vertex 3
} ShadeQuad;
*/

void preprocessShadeData(FVECTOR v[4], ShadeQuad *s) {

	// find the normal for the shade quad !
	double l01x = v[1].vx - v[0].vx;
	double l01y = v[1].vy - v[0].vy;
	double l01z = v[1].vz - v[0].vz;

	double l12x = v[2].vx - v[1].vx;
	double l12y = v[2].vy - v[1].vy;
	double l12z = v[2].vz - v[1].vz;

	double l23x = v[3].vx - v[2].vx;
	double l23y = v[3].vy - v[2].vy;
	double l23z = v[3].vz - v[2].vz;

	double l30x = v[0].vx - v[3].vx;
	double l30y = v[0].vy - v[3].vy;
	double l30z = v[0].vz - v[3].vz;

	// the cross-product to get the normal : l01 x l03
	double nx, ny, nz;
	CROSS_PRODUCT(l01x, l01y, l01z, -l30x, -l30y, -l30z, nx, ny, nz);
	NORMALISE(nx, ny, nz);

	// the planar distance
	double d = DOT_PRODUCT(nx, ny, nz, v[0].vx, v[0].vy, v[0].vz);

	// n01 is normal in the plane of l01 : n01 = l01 x n
	double n01x, n01y, n01z;
	CROSS_PRODUCT(l01x, l01y, l01z, nx, ny, nz, n01x, n01y, n01z);
	NORMALISE(n01x, n01y, n01z);
	// n12 is normal in the plane of l12 : n12 = l12 x n
	double n12x, n12y, n12z;
	CROSS_PRODUCT(l12x, l12y, l12z, nx, ny, nz, n12x, n12y, n12z);
	NORMALISE(n12x, n12y, n12z);
	// n23 is normal in the plane of l23 : n23 = l23 x n
	double n23x, n23y, n23z;
	CROSS_PRODUCT(l23x, l23y, l23z, nx, ny, nz, n23x, n23y, n23z);
	NORMALISE(n23x, n23y, n23z);
	// n30 is normal in the plane of l30 : n30 = l30 x n
	double n30x, n30y, n30z;
	CROSS_PRODUCT(l30x, l30y, l30z, nx, ny, nz, n30x, n30y, n30z);
	NORMALISE(n30x, n30y, n30z);

	double n01dots0, n12dots1, n23dots2, n30dots3;
	n01dots0 = DOT_PRODUCT(n01x, n01y, n01z, v[0].vx, v[0].vy, v[0].vz);
	n12dots1 = DOT_PRODUCT(n12x, n12y, n12z, v[1].vx, v[1].vy, v[1].vz);
	n23dots2 = DOT_PRODUCT(n23x, n23y, n23z, v[2].vx, v[2].vy, v[2].vz);
	n30dots3 = DOT_PRODUCT(n30x, n30y, n30z, v[3].vx, v[3].vy, v[3].vz);

	// Store the pre-processed data
	s->l01.vx = (short)l01x;
	s->l01.vy = (short)l01y;
	s->l01.vz = (short)l01z;

	s->l12.vx = (short)l12x;
	s->l12.vy = (short)l12y;
	s->l12.vz = (short)l12z;

	s->l23.vx = (short)l23x;
	s->l23.vy = (short)l23y;
	s->l23.vz = (short)l23z;

	s->l30.vx = (short)l30x;
	s->l30.vy = (short)l30y;
	s->l30.vz = (short)l30z;

	s->n01.vx = (short)(4096.0f * n01x);
	s->n01.vy = (short)(4096.0f * n01y);
	s->n01.vz = (short)(4096.0f * n01z);

	s->n12.vx = (short)(4096.0f * n12x);
	s->n12.vy = (short)(4096.0f * n12y);
	s->n12.vz = (short)(4096.0f * n12z);

	s->n23.vx = (short)(4096.0f * n23x);
	s->n23.vy = (short)(4096.0f * n23y);
	s->n23.vz = (short)(4096.0f * n23z);

	s->n30.vx = (short)(4096.0f * n30x);
	s->n30.vy = (short)(4096.0f * n30y);
	s->n30.vz = (short)(4096.0f * n30z);

	s->pn.vx = (short)(4096.0f * nx);
	s->pn.vy = (short)(4096.0f * ny);
	s->pn.vz = (short)(4096.0f * nz);

	s->d = (int)(4096.0f * d);
	s->n01dots0 = (int)n01dots0;
	s->n12dots1 = (int)n12dots1;
	s->n23dots2 = (int)n23dots2;
	s->n30dots3 = (int)n30dots3;
}

} // End of namespace ICB
