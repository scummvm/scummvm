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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/shade.h"
#include "engines/icb/shade_pc.h"

namespace ICB {

int computeShadeMultiplierPC(ShadeTriangle *shadeTri, VECTOR *model_pos, VECTOR *light_pos, int m) {
	// No visible edges
	if (shadeTri->vEdge == 0)
		return m;

	// does line from light to the model intersect the plane ?
	// i.e.  v = light + alpha * (model - light)
	// &&    (v-shade_pos[0]) . n = 0;

	// the equation is
	// alpha = ( d - D ) / ( ( model . n ) - D )
	//
	// where :
	//   d = shade_pos[0] . n
	//   D = light . n
	int D = DOT_PRODUCT_SVECTOR(shadeTri->pn, light_pos[0]);
	int mdotn = DOT_PRODUCT_SVECTOR(shadeTri->pn, model_pos[0]);

	int mdotnSubD = mdotn - D;
	int dSubD = shadeTri->d - D;

	int retm;
	if (dSubD > 0) {
		if (dSubD > mdotnSubD)
			retm = m;
		else
			retm = 0;
	} else {
		if (dSubD < mdotnSubD)
			retm = m;
		else
			retm = 0;
	}

	// Not hit the plane
	if (retm == m)
		return m;

	// we know alpha is 0 -> 1
	if (dSubD < 0) {
		dSubD = -dSubD;
		mdotnSubD = -mdotnSubD;
	}

	// Now, dSubD & mdotnSubD are +ve
	dSubD >>= 12;
	mdotnSubD >>= 12;

	// Clip the intersection to be within the triangle
	// i.e.  v = light + alpha * (model - light)
	//
	// n01.(v-s0) < 0 - inside polygon
	// n12.(v-s1) < 0 - inside polygon
	// n20.(v-s2) < 0 - inside polygon

	// i.e.  v = light + alpha * (model - light)
	//
	// n01.(v-s0) < 0 - inside polygon
	// n01.light + alpha * ( model.n01 - light.n01 ) - n01.s0 < 0
	// alpha = dSubD / mdotnSubD
	// => n01.light * mdotnSubD + dSubD * ( model.n01 - light.n01 ) -n01.s0*mdotnSubD < 0
	// => n01dotl * mdotnSubD + dSubD * ( n01dotm - n01dotl ) - n01dots0*mdotnSubD < 0
	// => mdotnSubD * ( n01dotl - n01dots0 ) + dSubD * ( n01dotm - n01dotl ) < 0
	// Is edge one visible
	int t01, t12, t20;
	int n01dotl, n01dotm;
	int n12dotl, n12dotm;
	int n20dotl, n20dotm;

	if (shadeTri->vEdge & EDGE_ONE) {
		n01dotl = DOT_PRODUCT_SVECTOR(shadeTri->n01, light_pos[0]) / 4096;
		n01dotm = DOT_PRODUCT_SVECTOR(shadeTri->n01, model_pos[0]) / 4096;
		t01 = mdotnSubD * (n01dotl - shadeTri->n01dots0) + dSubD * (n01dotm - n01dotl);
	} else
		t01 = -666;

	if (shadeTri->vEdge & EDGE_TWO) {
		n12dotl = DOT_PRODUCT_SVECTOR(shadeTri->n12, light_pos[0]) / 4096;
		n12dotm = DOT_PRODUCT_SVECTOR(shadeTri->n12, model_pos[0]) / 4096;
		t12 = mdotnSubD * (n12dotl - shadeTri->n12dots1) + dSubD * (n12dotm - n12dotl);
	} else
		t12 = -666;

	if (shadeTri->vEdge & EDGE_THREE) {
		n20dotl = DOT_PRODUCT_SVECTOR(shadeTri->n20, light_pos[0]) / 4096;
		n20dotm = DOT_PRODUCT_SVECTOR(shadeTri->n20, model_pos[0]) / 4096;
		t20 = mdotnSubD * (n20dotl - shadeTri->n20dots2) + dSubD * (n20dotm - n20dotl);
	} else
		t20 = -666;

	// All -ve means inside the polygon
	if ((t01 <= 0) && (t12 <= 0) && (t20 <= 0))
		return 0;

	// 32cm around the shade edge for gradually brightening & darkening
#define MAX_SHADE_FALLOFF 32

	int maxd = (1 << 30);
	if ((t01 > 0) && (t01 < maxd))
		maxd = t01;
	if ((t12 > 0) && (t12 < maxd))
		maxd = t12;
	if ((t20 > 0) && (t20 < maxd))
		maxd = t20;

	maxd /= mdotnSubD;

	if (maxd >= MAX_SHADE_FALLOFF)
		return m;

	return ((m * maxd) / MAX_SHADE_FALLOFF);
}

int computeShadeMultiplierPC(ShadeQuad *shadeQuad, VECTOR *model_pos, VECTOR *light_pos, int m) {
	// No visible edges
	if (shadeQuad->vEdge == 0)
		return m;

	// does line from light to the model intersect the plane ?
	// i.e.  v = light + alpha * (model - light)
	// &&    (v-shade_pos[0]) . n = 0;

	// the equation is
	// alpha = ( d - D ) / ( ( model . n ) - D )
	//
	// where :
	//   d = shade_pos[0] . n
	//   D = light . n
	int D = DOT_PRODUCT_SVECTOR(shadeQuad->pn, light_pos[0]);
	int mdotn = DOT_PRODUCT_SVECTOR(shadeQuad->pn, model_pos[0]);

	int mdotnSubD = mdotn - D;
	int dSubD = shadeQuad->d - D;

	int retm;
	if (dSubD > 0) {
		if (dSubD > mdotnSubD)
			retm = m;
		else
			retm = 0;
	} else {
		if (dSubD < mdotnSubD)
			retm = m;
		else
			retm = 0;
	}

	// Not hit the plane
	if (retm == m)
		return m;

	// we know alpha is 0 -> 1
	if (dSubD < 0) {
		dSubD = -dSubD;
		mdotnSubD = -mdotnSubD;
	}

	// Now, dSubD & mdotnSubD are +ve
	dSubD >>= 12;
	mdotnSubD >>= 12;

	// Clip the intersection to be within the quad
	// i.e.  v = light + alpha * (model - light)
	//
	// n01.(v-s0) < 0 - inside polygon
	// n12.(v-s1) < 0 - inside polygon
	// n23.(v-s2) < 0 - inside polygon
	// n30.(v-s3) < 0 - inside polygon
	// i.e.  v = light + alpha * (model - light)
	//
	// n01.(v-s0) < 0 - inside polygon
	// n01.light + alpha * ( model.n01 - light.n01 ) - n01.s0 < 0
	// alpha = dSubD / mdotnSubD
	// => n01.light * mdotnSubD + dSubD * ( model.n01 - light.n01 ) -n01.s0*mdotnSubD < 0
	// => n01dotl * mdotnSubD + dSubD * ( n01dotm - n01dotl ) - n01dots0*mdotnSubD < 0
	// => mdotnSubD * ( n01dotl - n01dots0 ) + dSubD * ( n01dotm - n01dotl ) < 0
	int n01dotl, n12dotl, n23dotl, n30dotl;
	int n01dotm, n12dotm, n23dotm, n30dotm;
	int t01, t12, t23, t30;

	// Is edge1 visible ?
	if (shadeQuad->vEdge & EDGE_ONE) {
		n01dotl = DOT_PRODUCT_SVECTOR(shadeQuad->n01, light_pos[0]) / 4096;
		n01dotm = DOT_PRODUCT_SVECTOR(shadeQuad->n01, model_pos[0]) / 4096;
		t01 = mdotnSubD * (n01dotl - shadeQuad->n01dots0) + dSubD * (n01dotm - n01dotl);
	} else
		t01 = -66;

	// Is edge2 visible ?
	if (shadeQuad->vEdge & EDGE_TWO) {
		n12dotl = DOT_PRODUCT_SVECTOR(shadeQuad->n12, light_pos[0]) / 4096;
		n12dotm = DOT_PRODUCT_SVECTOR(shadeQuad->n12, model_pos[0]) / 4096;
		t12 = mdotnSubD * (n12dotl - shadeQuad->n12dots1) + dSubD * (n12dotm - n12dotl);
	} else
		t12 = -66;

	// Is edge3 visible ?
	if (shadeQuad->vEdge & EDGE_THREE) {
		n23dotl = DOT_PRODUCT_SVECTOR(shadeQuad->n23, light_pos[0]) / 4096;
		n23dotm = DOT_PRODUCT_SVECTOR(shadeQuad->n23, model_pos[0]) / 4096;
		t23 = mdotnSubD * (n23dotl - shadeQuad->n23dots2) + dSubD * (n23dotm - n23dotl);
	} else
		t23 = -66;

	// Is edge4 visible ?
	if (shadeQuad->vEdge & EDGE_FOUR) {
		n30dotl = DOT_PRODUCT_SVECTOR(shadeQuad->n30, light_pos[0]) / 4096;
		n30dotm = DOT_PRODUCT_SVECTOR(shadeQuad->n30, model_pos[0]) / 4096;
		t30 = mdotnSubD * (n30dotl - shadeQuad->n30dots3) + dSubD * (n30dotm - n30dotl);
	} else
		t30 = -66;

	// All -ve means inside the polygon
	if ((t01 <= 0) && (t12 <= 0) && (t23 <= 0) && (t30 <= 0))
		return 0;

	// 32cm around the shade edge for gradually brightening & darkening
#define MAX_SHADE_FALLOFF 32

	int maxd = 0;
	if (t01 > maxd)
		maxd = t01;
	if (t12 > maxd)
		maxd = t12;
	if (t23 > maxd)
		maxd = t23;
	if (t30 > maxd)
		maxd = t30;

	maxd /= mdotnSubD;

	if (maxd >= MAX_SHADE_FALLOFF)
		return m;

	return ((m * maxd) / MAX_SHADE_FALLOFF);
}

} // End of namespace ICB
