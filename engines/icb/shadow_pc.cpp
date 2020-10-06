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
#include "engines/icb/debug.h"
#include "engines/icb/shadow_pc.h"
#include "engines/icb/softskin_pc.h"
#include "engines/icb/drawpoly_pc.h"
#include "engines/icb/global_objects_psx.h"
#include "engines/icb/actor_pc.h"
#include "engines/icb/common/px_capri_maths.h"

#include "common/system.h"

namespace ICB {

// 128 so then it fits into scratch pad !
#define MAX_VECTORS 512
#define MAX_SCRATCHPAD_VECTORS 128

int sverttpc;
int st1pc;
int st2pc;
int st3pc;

void MakeShadowPC(rap_API *srap, SVECTORPC *local, int nVertices, SVECTORPC *p_n, int p_d, SVECTORPC *ldir, CVECTOR *lcolour, MATRIXPC *world2screen, MATRIXPC *local2world,
                  int debug, SVECTOR *bbox, SVECTOR *minbbox, SVECTOR *maxbbox, int16 xminLocal, int16 xmaxLocal, int16 yminLocal, int16 ymaxLocal, int16 zminLocal,
                  int16 zmaxLocal);

void DrawShadow1PC(rap_API *srap, int poseBone, MATRIXPC *lw, MATRIXPC *world2screen, MATRIXPC *local2world, int nShadows, SVECTORPC *ldirs, CVECTOR *lcolours, SVECTORPC *p_n,
                   int *p_d, int debug, SVECTOR **shadowBox, SVECTOR *shadowBoxMin, SVECTOR *shadowBoxMax) {
	if (nShadows == 0)
		return;

	SVECTORPC local[MAX_VECTORS];
	// compute the current animation positions of the
	// shadow mesh (rap) vertices

	int16 xminLocal = +32767;
	int16 xmaxLocal = -32767;
	int16 yminLocal = +32767;
	int16 ymaxLocal = -32767;
	int16 zminLocal = +32767;
	int16 zmaxLocal = -32767;

	sverttpc = g_system->getMillis();
	int screenScale = 0;
	int nVertices = softskinPC(srap, poseBone, lw, local, &xminLocal, &xmaxLocal, &yminLocal, &ymaxLocal, &zminLocal, &zmaxLocal, screenScale);

	gte_SetScreenScaleShift_pc(screenScale);

	// So all the local positions have been made
	sverttpc = g_system->getMillis() - sverttpc;

	int s;
	SVECTORPC *pp_n = p_n;
	int *pp_d = p_d;
	SVECTORPC *pldirs = ldirs;
	CVECTOR *plcolours = lcolours;
	for (s = 0; s < nShadows; s++) {
		MakeShadowPC(srap, local, nVertices, pp_n, *pp_d, pldirs, plcolours, world2screen, local2world, debug, shadowBox[s], shadowBoxMin + s, shadowBoxMax + s, xminLocal,
		             xmaxLocal, yminLocal, ymaxLocal, zminLocal, zmaxLocal);
		pldirs++;
		plcolours++;
	}
}

void MakeShadowPC(rap_API *srap, SVECTORPC *local, int nVertices, SVECTORPC *p_n, int p_d, SVECTORPC *ldir, CVECTOR *lcolour, MATRIXPC *world2screen, MATRIXPC *local2world,
                  int debug, SVECTOR *bbox, SVECTOR *minbbox, SVECTOR *maxbbox, int16 xminLocal, int16 xmaxLocal, int16 yminLocal, int16 ymaxLocal, int16 zminLocal,
                  int16 zmaxLocal) {

	SVECTORPC workVerts[MAX_VECTORS];
	SVECTORPC *vertices = NULL;

	vertices = workVerts;

	/*
	   Make the shadow projection matrix

	   projection equation is:
	    P = V + ( D - V.N ) L
	    -   -         - -   -

	   Where:
	     P = is the projection point of vertex V
	    V = vertex to be projected
	     N = normal vector of the plane to project onto
	         (doesn't have to be normalised)
	     D = 'd' in the plane equation of the plane to project onto
	         i.e. D = N.(point_plane)
	     L = normalised light direction
	       = L_direction / (L_direction . N )


	   This expands into the following "rotation" matrix:

	   /                               \
	   |  (1 - Nx*Lx)  -Ny*Lx   -Nz*Lx |
	   |                               |
	   |  -Nx*Ly  (1 - Ny*Ly)   -Nz*Ly |
	   |                               |
	   |  -Nx*Lz  -Ny*Lz   (1 - Nz*Lz) |
	   \                               /

	   and "transformation" matrix

	   /         \
	   |  +Lx*D  |
	   |         |
	   |  +Ly*D  |
	   |         |
	   |  +Lz*D  |
	   \         /
	*/

	st1pc = g_system->getMillis();
	// First-up let us make normalised_light_direction
	int ld = p_n->vx * ldir->vx + p_n->vy * ldir->vy + p_n->vz * ldir->vz;

	// Can't do a shadow if ld == 0 : light perpendicular to the plane
	if (ld == 0)
		return;

	// To match names in the comments
	int D;
	SVECTORPC *N;
	SVECTORPC L;
	int32 work;

	D = p_d;
	N = p_n;
	// * 4096 to get some fixed point accuracy in there!
	work = (ldir->vx << 12) / ld;
	if (work > 32767)
		work = 32767;
	if (work < -32767)
		work = -32767;
	L.vx = (int)work;
	work = (ldir->vy << 12) / ld;
	if (work > 32767)
		work = 32767;
	if (work < -32767)
		work = -32767;
	L.vy = (int)work;
	work = (ldir->vz << 12) / ld;
	if (work > 32767)
		work = 32767;
	if (work < -32767)
		work = -32767;
	L.vz = (int)work;

	MATRIXPC sproj;

	// |  (1 - Nx*Lx)  -Ny*Lx   -Nz*Lx |
	sproj.m[0][0] = (int)(4096 - N->vx * L.vx);
	sproj.m[0][1] = (int)(-N->vy * L.vx);
	sproj.m[0][2] = (int)(-N->vz * L.vx);

	// |  -Nx*Ly  (1 - Ny*Ly)   -Nz*Ly |
	sproj.m[1][0] = (int)(-N->vx * L.vy);
	sproj.m[1][1] = (int)(4096 - N->vy * L.vy);
	sproj.m[1][2] = (int)(-N->vz * L.vy);

	// |  -Nx*Lz  -Ny*Lz   (1 - Nz*Lz) |
	sproj.m[2][0] = (int)(-N->vx * L.vz);
	sproj.m[2][1] = (int)(-N->vy * L.vz);
	sproj.m[2][2] = (int)(4096 - N->vz * L.vz);

	// |  +Lx*D  |
	sproj.t[0] = (L.vx * D) >> 12; // go back to integer maths not fixed point

	// |  +Ly*D  |
	sproj.t[1] = (L.vy * D) >> 12; // go back to integer maths not fixed point

	// |  +Lz*D  |
	sproj.t[2] = (L.vz * D) >> 12; // go back to integer maths not fixed point

	st1pc = g_system->getMillis() - st1pc;
	st2pc = g_system->getMillis();

	SVECTORPC *world = vertices;
	SVECTORPC *pvert = vertices;

	int32 flag;
	SVECTORPC *pworld;
	SVECTORPC *plocal;
	SVECTOR *pbbox;

	plocal = local;
	pworld = world;
	VECTOR lpvert;
	int i;

	// Transform the local vertices into world vertices
	gte_SetRotMatrix_pc(local2world);
	gte_SetTransMatrix_pc(local2world);
	for (i = 0; i < nVertices; i++) {
		gte_RotTrans_pc(plocal, &lpvert, &flag);
		pworld->vx = (int)lpvert.vx;
		pworld->vy = (int)lpvert.vy;
		pworld->vz = (int)lpvert.vz;
		plocal++;
		pworld++;
	}

	// Convert the bounding box from local co-ordinates into world co-ordinates
	bbox[0].vx = xminLocal;
	bbox[0].vy = yminLocal;
	bbox[0].vz = zminLocal;

	bbox[1].vx = xminLocal;
	bbox[1].vy = yminLocal;
	bbox[1].vz = zmaxLocal;

	bbox[2].vx = xmaxLocal;
	bbox[2].vy = yminLocal;
	bbox[2].vz = zminLocal;

	bbox[3].vx = xmaxLocal;
	bbox[3].vy = yminLocal;
	bbox[3].vz = zmaxLocal;

	bbox[4].vx = xmaxLocal;
	bbox[4].vy = ymaxLocal;
	bbox[4].vz = zminLocal;

	bbox[5].vx = xmaxLocal;
	bbox[5].vy = ymaxLocal;
	bbox[5].vz = zmaxLocal;

	bbox[6].vx = xminLocal;
	bbox[6].vy = ymaxLocal;
	bbox[6].vz = zminLocal;

	bbox[7].vx = xminLocal;
	bbox[7].vy = ymaxLocal;
	bbox[7].vz = zmaxLocal;

	pbbox = bbox;
	for (i = 0; i < 8; i++) {
		gte_RotTrans_pc(pbbox, &lpvert, &flag);
		pbbox->vx = (short)lpvert.vx;
		pbbox->vy = (short)lpvert.vy;
		pbbox->vz = (short)lpvert.vz;
		pbbox++;
	}

	// So basically do a load of RotTrans to project the world vertices
	// into a new set of world vertices
	gte_SetRotMatrix_pc(&sproj);
	gte_SetTransMatrix_pc(&sproj);
	SVECTORPC *ppvert;
	plocal = world;
	ppvert = pvert;

	for (i = 0; i < nVertices; i++) {
		gte_RotTrans_pc(plocal, &lpvert, &flag);
		ppvert->vx = lpvert.vx;
		ppvert->vy = lpvert.vy;
		ppvert->vz = lpvert.vz;

		plocal++;
		ppvert++;
	}

	// Do the same for the bounding box
	pbbox = bbox;
	for (i = 0; i < 8; i++) {
		gte_RotTrans_pc(pbbox, &lpvert, &flag);
		pbbox->vx = (short)lpvert.vx;
		pbbox->vy = (short)lpvert.vy;
		pbbox->vz = (short)lpvert.vz;
		pbbox++;
	}

	// Put the correct rot and trans matrix in place
	// transform vertices from world space to screen space
	gte_SetRotMatrix_pc(world2screen);
	gte_SetTransMatrix_pc(world2screen);

	// Loop over the vector pool converting them all to screen co-ordinates
	int32 p;
	if (debug == 0)
		ConvertToScreenCoords(pvert, pvert, nVertices);

	// Do the same for the bounding box
	pbbox = bbox;
	SVECTORPC sxy0;
	for (i = 0; i < 8; i++) {
		gte_RotTransPers_pc(pbbox, &sxy0, &p, &flag, (int32 *)&(pbbox->vz));
		pbbox->vx = (short)sxy0.vx;
		pbbox->vy = (short)sxy0.vy;
		pbbox++;
	}

	// Find the minimum and maximum screen positions (plus z)
	pbbox = bbox;
	copyVector(minbbox, pbbox);
	copyVector(maxbbox, pbbox);
	pbbox++;
	for (i = 1; i < 8; i++, pbbox++) {
		if (pbbox->vx < minbbox->vx)
			minbbox->vx = pbbox->vx;
		if (pbbox->vy < minbbox->vy)
			minbbox->vy = pbbox->vy;
		if (pbbox->vz < minbbox->vz)
			minbbox->vz = pbbox->vz;
		if (pbbox->vx > maxbbox->vx)
			maxbbox->vx = pbbox->vx;
		if (pbbox->vy > maxbbox->vy)
			maxbbox->vy = pbbox->vy;
		if (pbbox->vz > maxbbox->vz)
			maxbbox->vz = pbbox->vz;
	}

	st2pc = g_system->getMillis() - st2pc;
	st3pc = g_system->getMillis();

	// Now draw the little blighters
	unlitPoly.r = lcolour->r;
	unlitPoly.g = lcolour->g;
	unlitPoly.b = lcolour->b;
	unlitPoly.cd = 0x80; // Switch to 0x80 for subtractive once the dutch are fixed.

	// Now go and find the actual polygon data for this primitive
	uint32 *polyStart;
	uint32 nPolys;

	nPolys = srap->nTRI3;
	if (nPolys != 0) {
		polyStart = srap->GetTRI3Ptr();
		// Do the drawing using internal C based debugging drawing code
#if CD_MODE == 0
		if (debug)
			drawTRI3PC(polyStart, nPolys, pvert);
		else
#endif // #if CD_MODE == 0
			fastDrawTRI3PC(polyStart, nPolys, pvert);
	}
	st3pc = g_system->getMillis() - st3pc;
}

} // End of namespace ICB
