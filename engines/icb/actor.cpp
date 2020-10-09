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

#include "engines/icb/global_objects.h"
#include "engines/icb/actor.h"
#include "engines/icb/gfx/psx_scrn.h"
#include "engines/icb/gfx/psx_poly.h"
#include "engines/icb/common/px_capri_maths.h"

namespace ICB {

// return 0 if on screen
// return 1 if off screen
int QuickActorCull(psxCamera *camera, VECTOR *pos, SVECTOR *orient) {
	MATRIX lw, ls;

	// Set the focal length in the GTE
	gte_SetGeomScreen(camera->focLen);

	// Now make a "true" local-world matrix i.e. without render correction
	RotMatrix_gte(orient, &lw);
	lw.t[0] = pos->vx;
	lw.t[1] = pos->vy;
	lw.t[2] = pos->vz;

	// Make the equivalent local-screen matrix
	// Should inline this
	makeLSmatrix(&camera->view, &lw, &ls);

	// Do a high level check to see if the actor and a very large bounding box are on the screen or not
	int16 xminLocal = -100; // -1m
	int16 yminLocal = 0;    // on the ground
	int16 zminLocal = -100; // +1m

	int16 xmaxLocal = +100; // +1m
	int16 ymaxLocal = +200; // +2m high people
	int16 zmaxLocal = +100; // +1m

	SVECTOR bboxLocal[8];

	bboxLocal[0].vx = xminLocal;
	bboxLocal[0].vy = yminLocal;
	bboxLocal[0].vz = zminLocal;

	bboxLocal[1].vx = xminLocal;
	bboxLocal[1].vy = yminLocal;
	bboxLocal[1].vz = zmaxLocal;

	bboxLocal[2].vx = xmaxLocal;
	bboxLocal[2].vy = yminLocal;
	bboxLocal[2].vz = zminLocal;

	bboxLocal[3].vx = xmaxLocal;
	bboxLocal[3].vy = yminLocal;
	bboxLocal[3].vz = zmaxLocal;

	bboxLocal[4].vx = xmaxLocal;
	bboxLocal[4].vy = ymaxLocal;
	bboxLocal[4].vz = zminLocal;

	bboxLocal[5].vx = xmaxLocal;
	bboxLocal[5].vy = ymaxLocal;
	bboxLocal[5].vz = zmaxLocal;

	bboxLocal[6].vx = xminLocal;
	bboxLocal[6].vy = ymaxLocal;
	bboxLocal[6].vz = zminLocal;

	bboxLocal[7].vx = xminLocal;
	bboxLocal[7].vy = ymaxLocal;
	bboxLocal[7].vz = zmaxLocal;

	SVECTOR *local = bboxLocal;
	SVECTOR bboxScrn[8];
	SVECTOR *scrn = bboxScrn;
	int32 z0;
	int32 p, flag;
	int i;

	// Set the local-screen matrix in the GTE
	gte_SetRotMatrix(&ls);
	gte_SetTransMatrix(&ls);

	for (i = 0; i < 8; i++, local++, scrn++) {
		gte_RotTransPers(local, (int32 *)&(scrn->vx), &p, &flag, &z0);
		scrn->vz = (short)z0;
	}

	// Find the minimum and maximum screen positions (plus z)
	scrn = bboxScrn;
	SVECTOR scrnMin, scrnMax;
	copyVector(&scrnMin, scrn);
	copyVector(&scrnMax, scrn);
	scrn++;
	for (i = 1; i < 8; i++, scrn++) {
		if (scrn->vx < scrnMin.vx)
			scrnMin.vx = scrn->vx;
		if (scrn->vy < scrnMin.vy)
			scrnMin.vy = scrn->vy;
		if (scrn->vz < scrnMin.vz)
			scrnMin.vz = scrn->vz;
		if (scrn->vx > scrnMax.vx)
			scrnMax.vx = scrn->vx;
		if (scrn->vy > scrnMax.vy)
			scrnMax.vy = scrn->vy;
		if (scrn->vz > scrnMax.vz)
			scrnMax.vz = scrn->vz;
	}

	// Reject the actor if the gross bounding box isn't on the screen yet
	if ((scrnMin.vx > (SCREEN_W / 2)) ||         // min x off right of the screen
	    (scrnMin.vy > (SCREEN_H / 2)) ||         // min y off top of the screen
	    (scrnMax.vx < (-SCREEN_W / 2)) ||        // max x off left of the screen
	    (scrnMax.vy < (-SCREEN_H / 2)) ||        // max y off bottom of the screen
	    (scrnMax.vz < g_actor_hither_plane) ||   // max z infront of the hither plane
	    (scrnMin.vz > g_actor_far_clip_plane)) { // min z behind the max z plane
		return 1;
	}
	return 0;
}

} // End of namespace ICB
