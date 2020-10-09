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

#ifndef ICB_SHADE_H
#define ICB_SHADE_H

#include "engines/icb/gfx/psx_pcdefines.h"
#include "engines/icb/gfx/rlp_api.h"

namespace ICB {

typedef struct FVECTOR {
	float vx;
	float vy;
	float vz;
} FVECTOR;

// Handy maths function
void makePlaneEquation(FVECTOR *v0, FVECTOR *v1, FVECTOR *v2, int32 *d, FVECTOR *pn);

void preprocessShadeData(FVECTOR v[3], ShadeTriangle *s);

void preprocessShadeData(FVECTOR v[4], ShadeQuad *s);

#define DOT_PRODUCT(v0x, v0y, v0z, v1x, v1y, v1z) (((v0x) * (v1x)) + ((v0y) * (v1y)) + ((v0z) * (v1z)))

#define CROSS_PRODUCT(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z)                                                                                                                 \
	{                                                                                                                                                                          \
		v2x = ((v0y) * (v1z)) - ((v0z) * (v1y));                                                                                                                           \
		v2y = ((v0z) * (v1x)) - ((v0x) * (v1z));                                                                                                                           \
		v2z = ((v0x) * (v1y)) - ((v0y) * (v1x));                                                                                                                           \
	}

#define VEC_SUB(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z)                                                                                                                       \
	{                                                                                                                                                                          \
		v2x = v0x - v1x;                                                                                                                                                   \
		v2y = v0y - v1y;                                                                                                                                                   \
		v2z = v0z - v1z;                                                                                                                                                   \
	}

#define VEC_ADD(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z)                                                                                                                       \
	{                                                                                                                                                                          \
		v2x = v0x + v1x;                                                                                                                                                   \
		v2y = v0y + v1y;                                                                                                                                                   \
		v2z = v0z + v1z;                                                                                                                                                   \
	}

#define DOT_PRODUCT(v0x, v0y, v0z, v1x, v1y, v1z) (((v0x) * (v1x)) + ((v0y) * (v1y)) + ((v0z) * (v1z)))

#define CROSS_PRODUCT(v0x, v0y, v0z, v1x, v1y, v1z, v2x, v2y, v2z)                                                                                                                 \
	{                                                                                                                                                                          \
		v2x = ((v0y) * (v1z)) - ((v0z) * (v1y));                                                                                                                           \
		v2y = ((v0z) * (v1x)) - ((v0x) * (v1z));                                                                                                                           \
		v2z = ((v0x) * (v1y)) - ((v0y) * (v1x));                                                                                                                           \
	}

#define NORMALISE(v0x, v0y, v0z)                                                                                                                                                   \
	{                                                                                                                                                                          \
		double r = sqrt(DOT_PRODUCT(v0x, v0y, v0z, v0x, v0y, v0z));                                                                                                        \
		(v0x) = (float)((v0x) / r);                                                                                                                                        \
		(v0y) = (float)((v0y) / r);                                                                                                                                        \
		(v0z) = (float)((v0z) / r);                                                                                                                                        \
	}

#define NORMALISE_VECTOR(v) NORMALISE((v).vx, (v).vy, (v).vz)

#define DOT_PRODUCT_VECTOR(v0, v1) DOT_PRODUCT(((v0).vx), ((v0).vy), ((v0).vz), ((v1).vx), ((v1).vy), ((v1).vz))

#define CROSS_PRODUCT_VECTOR(v0, v1, v2) CROSS_PRODUCT(((v0).vx), ((v0).vy), ((v0).vz), ((v1).vx), ((v1).vy), ((v1).vz), ((v2).vx), ((v2).vy), ((v2).vz), )

#define VEC_ADD_VECTOR(v0, v1, v2) VEC_ADD(((v0).vx), ((v0).vy), ((v0).vz), ((v1).vx), ((v1).vy), ((v1).vz), ((v2).vx), ((v2).vy), ((v2).vz), )

#define VEC_SUB_VECTOR(v0, v1, v2) VEC_SUB(((v0).vx), ((v0).vy), ((v0).vz), ((v1).vx), ((v1).vy), ((v1).vz), ((v2).vx), ((v2).vy), ((v2).vz), )

#define DOT_PRODUCT_SVECTOR(v0, v1) (DOT_PRODUCT((v0.vx), (v0.vy), (v0.vz), (v1.vx), (v1.vy), (v1.vz)))

} // End of namespace ICB

#endif // #ifndef SHADE_H
