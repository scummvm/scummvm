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

#include "engines/icb/gfx/psx_camera.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/common/px_capri_maths.h"

namespace ICB {

// Make the local to screen matrix from the
// world-screen & local-world matrices

// ls rot matrix = world2screen * local2world
// ls trans vector = world2screen * local2world_trans + world2screen_trans
void makeLSmatrix(MATRIX *ws, MATRIX *lw, MATRIX *ls) {
	VECTOR tmp;
	// Make the rotation matrix
	gte_MulMatrix0(ws, lw, ls);
	// make the ls trans vector
	//  = world2screen * local2world_trans + world2screen_trans
	ApplyMatrixLV(ws, (VECTOR *)&lw->t[0], &tmp);
	ls->t[0] = tmp.vx + ws->t[0];
	ls->t[1] = tmp.vy + ws->t[1];
	ls->t[2] = tmp.vz + ws->t[2];
}

// Project x has silly definitions for these so have to silly stuff
// with y & z
void psxWorldToFilm(const PXvector_PSX &worldpos, const psxCamera &camera, bool8 &is_onfilm, PXvector_PSX &filmpos) {
	VECTOR scrn;
	ApplyMatrixLV(const_cast<MATRIX *>(&camera.view), (VECTOR *)const_cast<PXvector_PSX *>(&worldpos), &scrn);
	scrn.vx += camera.view.t[0];
	scrn.vy += camera.view.t[1];
	scrn.vz += camera.view.t[2];

	if (scrn.vz != 0) {
		scrn.vx = camera.focLen * scrn.vx / scrn.vz;
		scrn.vy = camera.focLen * scrn.vy / scrn.vz;
	}
	filmpos.x = scrn.vx;
	filmpos.y = -scrn.vy;     // px convention is upside down y
	filmpos.z = -scrn.vz / 4; // px convention is -ve z : PSX also has zscale * 4 to make it more accurate

	if ((scrn.vx < -SCREEN_W / 2) || (scrn.vx > SCREEN_W / 2) || (scrn.vy < -SCREEN_H / 2) || (scrn.vy > SCREEN_H / 2))
		is_onfilm = FALSE8;
	else
		is_onfilm = TRUE8;
}

} // End of namespace ICB
