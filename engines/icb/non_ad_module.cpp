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

#include "engines/icb/p4_generic_pc.h"

namespace ICB {

inline void pcApplyMatrixLV(MATRIX *m, VECTOR *invec, VECTOR *outvec) {
	outvec->vx = ((int)m->m[0][0] * invec->vx + (int)m->m[0][1] * invec->vy + (int)m->m[0][2] * invec->vz) >> 12;
	outvec->vy = ((int)m->m[1][0] * invec->vx + (int)m->m[1][1] * invec->vy + (int)m->m[1][2] * invec->vz) >> 12;
	outvec->vz = ((int)m->m[2][0] * invec->vx + (int)m->m[2][1] * invec->vy + (int)m->m[2][2] * invec->vz) >> 12;
}

void WorldToFilm(const PXvector_PC &worldpos, const PCcamera &camera, bool8 &is_onfilm, PXvector_PC &filmpos) {
	VECTOR scrn;
	VECTOR vWorldPos;
	vWorldPos.vx = (int)worldpos.x;
	vWorldPos.vy = (int)worldpos.y;
	vWorldPos.vz = (int)worldpos.z;

	pcApplyMatrixLV(const_cast<MATRIX *>(&camera.view), &vWorldPos, &scrn);
	scrn.vx += camera.view.t[0];
	scrn.vy += camera.view.t[1];
	scrn.vz += camera.view.t[2];

	if (scrn.vz) {
		scrn.vx = camera.focLen * scrn.vx / scrn.vz;
		scrn.vy = camera.focLen * scrn.vy / scrn.vz;
	}
	filmpos.x = (float)scrn.vx;
	filmpos.y = (float)-scrn.vy;        // px convention is upside down y
	filmpos.z = (float)-scrn.vz / 4.0f; // px convention is -ve z : PSX also has zscale * 4 to make it more accurate

	if ((scrn.vx < -SCREEN_WIDTH / 2) || (scrn.vx > SCREEN_WIDTH / 2) || (scrn.vy < -SCREEN_DEPTH / 2) || (scrn.vy > SCREEN_DEPTH / 2))
		is_onfilm = FALSE8;
	else
		is_onfilm = TRUE8;
}

PXreal AngleOfVector(PXreal x, PXreal y) {
	if (fabs(x) > 0.0001f) { /*smallest value safe for atan2*/
		return (float)atan2(y, x) / (float)M_PI / 2;
	} else {
		// atan2's y/x would be infinite, so treat as special case
		if (y > 0.0f)
			return (0.5f /*half turn*/ / 2);
		else
			return (-0.5f /*half turn*/ / 2);
	}
}

} // End of namespace ICB
