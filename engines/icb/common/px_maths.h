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

#ifndef ICB_PX_MATHS_H
#define ICB_PX_MATHS_H

namespace ICB {

static inline int fastABSc(int a) {
	int r0 = a >> 31;
	int r1 = a + r0;
	int r2 = r1 ^ r0;
	return r2;
}

static inline int fastMINc(int a, int b) {
	int r0 = a - b;
	int r1 = r0 >> 31;
	int r2 = a & r1;
	int r3 = ~r1;
	int r4 = b & r3;
	int r5 = r2 | r4;

	return r5;
}

static inline int fastMAXc(int a, int b) {
	int r0 = a - b;
	int r1 = r0 >> 31;
	int r2 = b & r1;
	int r3 = ~r1;
	int r4 = a & r3;
	int r5 = r2 | r4;

	return r5;
}

#include "engines/icb/common/px_maths_pc.h"

#define PXmin(a, b) PCfastMIN(a, b)
#define PXmax(a, b) PCfastMAX(a, b)
#define PXabs(a) PCfastABS(a)

} // End of namespace ICB

#endif // #ifndef PX_MATHS_H
